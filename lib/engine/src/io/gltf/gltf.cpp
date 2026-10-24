#include "io/gltf/gltf.hpp"

#include <algorithm>

#include "cgltf.h"
#include "render/data/vertex.hpp"
#include <fstream>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <unordered_map>

namespace {
    void DecomposeNodeMatrix(const float m[16], GLTFNode& out) {       // column-major, as in glTF
        out.pos = {m[12], m[13], m[14]};
        float c[3][3];                                                   // basis columns
        for (int i = 0; i < 3; ++i) {
            c[i][0] = m[i*4 + 0]; c[i][1] = m[i*4 + 1]; c[i][2] = m[i*4 + 2];
        }
        float s[3];
        for (int i = 0; i < 3; ++i) {
            s[i] = std::sqrt(c[i][0]*c[i][0] + c[i][1]*c[i][1] + c[i][2]*c[i][2]);
            if (s[i] > 1e-8f) for (int k = 0; k < 3; ++k) c[i][k] /= s[i];
        }
        // negative determinant = mirrored: push the flip into scale.x
        float det = c[0][0]*(c[1][1]*c[2][2]-c[1][2]*c[2][1])
                  - c[0][1]*(c[1][0]*c[2][2]-c[1][2]*c[2][0])
                  + c[0][2]*(c[1][0]*c[2][1]-c[1][1]*c[2][0]);
        if (det < 0.0f) { s[0] = -s[0]; for (int k = 0; k < 3; ++k) c[0][k] = -c[0][k]; }
        out.scale = {s[0], s[1], s[2]};

        const float m00=c[0][0], m10=c[0][1], m20=c[0][2];
        const float m01=c[1][0], m11=c[1][1], m21=c[1][2];
        const float m02=c[2][0], m12=c[2][1], m22=c[2][2];
        float w, x, y, z;
        const float tr = m00 + m11 + m22;
        if (tr > 0.0f)                       { float q = 0.5f / std::sqrt(tr + 1.0f); w = 0.25f / q; x = (m21-m12)*q; y = (m02-m20)*q; z = (m10-m01)*q; }
        else if (m00 > m11 && m00 > m22)     { float q = 2.0f * std::sqrt(1.0f + m00 - m11 - m22); w = (m21-m12)/q; x = 0.25f*q; y = (m01+m10)/q; z = (m02+m20)/q; }
        else if (m11 > m22)                  { float q = 2.0f * std::sqrt(1.0f + m11 - m00 - m22); w = (m02-m20)/q; x = (m01+m10)/q; y = 0.25f*q; z = (m12+m21)/q; }
        else                                 { float q = 2.0f * std::sqrt(1.0f + m22 - m00 - m11); w = (m10-m01)/q; x = (m02+m20)/q; y = (m12+m21)/q; z = 0.25f*q; }
        out.rot.w = w; out.rot.x = x; out.rot.y = y; out.rot.z = z;
    }

    bool ExtractImageBytes(const cgltf_image* img, const std::string& gltfPath,
                       std::vector<unsigned char>& out)
{
    if (img->buffer_view) {                                   // .glb: bytes live in the BIN chunk
        const cgltf_buffer_view* bv = img->buffer_view;
        const unsigned char* p = (const unsigned char*)bv->buffer->data + bv->offset;
        out.assign(p, p + bv->size);
        return true;
    }
    if (!img->uri) return false;

    if (std::strncmp(img->uri, "data:", 5) == 0) {            // base64 data URI
        const char* comma = std::strchr(img->uri, ',');
        if (!comma) return false;
        const char* b64 = comma + 1;
        size_t len = std::strlen(b64);
        size_t pad = (len >= 1 && b64[len-1] == '=') + (len >= 2 && b64[len-2] == '=');
        size_t size = len / 4 * 3 - pad;
        void* decoded = nullptr;
        cgltf_options opt = {};
        if (cgltf_load_buffer_base64(&opt, size, b64, &decoded) != cgltf_result_success) return false;
        out.assign((unsigned char*)decoded, (unsigned char*)decoded + size);
        std::free(decoded);
        return true;
    }

    std::string uri = img->uri;                               // external file next to the .gltf
    cgltf_decode_uri(uri.data());
    uri.resize(std::strlen(uri.c_str()));
    size_t slash = gltfPath.find_last_of("/\\");
    std::string dir = (slash == std::string::npos) ? "" : gltfPath.substr(0, slash + 1);
    std::ifstream f(dir + uri, std::ios::binary);
    if (!f) return false;
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

// Identify the format from the bytes themselves; don't trust the file name or mime type.
const char* SniffExtension(const std::vector<unsigned char>& b) {
    if (b.size() >= 8 && b[0] == 0x89 && b[1] == 'P' && b[2] == 'N' && b[3] == 'G') return ".png";
    if (b.size() >= 3 && b[0] == 0xFF && b[1] == 0xD8 && b[2] == 0xFF)              return ".jpg";
    return nullptr;                                           // webp, ktx2, etc.: unsupported by stb
}

    std::string ResolveNodeName(const cgltf_node* node, const cgltf_data* data) {
        if (!node->name || node->name[0] == '\0') {
        size_t index = node - data->nodes;
            return "node_" + std::to_string(index);
        }
        return node->name;
    }

    bool ReadSampler(const cgltf_animation_sampler* samp,cgltf_animation_path_type path,std::vector<float>& times,std::vector<float>& values){
        if (!samp || !samp->input || !samp->output) return false;
        if (samp->interpolation == cgltf_interpolation_type_cubic_spline) return false;

        const cgltf_accessor* in  = samp->input;
        const cgltf_accessor* out = samp->output;

        // input: scalar floats, at least one key
        if (in->type != cgltf_type_scalar || in->component_type != cgltf_component_type_r_32f) return false;
        if (in->count == 0) return false;

        // linear/step pair keys 1:1
        if (out->count != in->count) return false;

        // output shape depends on the property being animated
        switch (path) {
            case cgltf_animation_path_type_translation:
            case cgltf_animation_path_type_scale:
                if (out->type != cgltf_type_vec3 || out->component_type != cgltf_component_type_r_32f) return false;
                break;
            case cgltf_animation_path_type_rotation:
                if (out->type != cgltf_type_vec4) return false;
                if (out->component_type != cgltf_component_type_r_32f && !out->normalized) return false;
                break;
            default:
                return false;
        }


        times.resize(in->count);
        values.resize(out->count * cgltf_num_components(out->type));

        if (cgltf_accessor_unpack_floats(in,  times.data(),  times.size())  != times.size())  return false;
        if (cgltf_accessor_unpack_floats(out, values.data(), values.size()) != values.size()) return false;
        return true;
    }


    std::optional<animation::AnimationClip> ImportAnimation(const cgltf_animation& anim,const cgltf_data* data ){
        animation::AnimationClip clip;
        std::unordered_map<const cgltf_node*,size_t> temp; //index of clip.tracks
        clip.name = anim.name ? anim.name : "";
        float start,end;


        for (size_t c = 0; c < anim.channels_count; c++) {
            const cgltf_animation_channel& channel = anim.channels[c];
            cgltf_node* node = channel.target_node;
            if (!node) {continue;}
            if (!temp.contains(node)) {
                animation::Track track;
                track.name = ResolveNodeName(node,data);
                clip.tracks.push_back(track);
                temp.try_emplace(node,clip.tracks.size()-1);
            }
            animation::Track& track = clip.tracks.at(temp.at(node));
            const cgltf_animation_sampler* samp = channel.sampler;
            const cgltf_accessor* inputAccessor = samp->input;
            const cgltf_accessor* outputAccessor = samp->output;
            std::vector<float> times, values;
            if (!ReadSampler(samp, channel.target_path, times, values)) return std::nullopt;

            auto mode = (samp->interpolation == cgltf_interpolation_type_step) ? animation::InterpolationMode::Constant : animation::InterpolationMode::Linear;

            switch (channel.target_path) {
                case cgltf_animation_path_type_translation:
                    for (size_t k = 0; k < times.size(); ++k) {
                        float frame = times[k] * clip.fps;
                        track.position.keyframes.push_back(
                            { frame, mathpp::vec3f{ values[3*k], values[3*k+1], values[3*k+2] }, mode });
                    }break;

                case cgltf_animation_path_type_scale:
                    for (size_t k = 0; k < times.size(); ++k) {
                        float frame = times[k] * clip.fps;
                        track.scale.keyframes.push_back(
                            { frame, mathpp::vec3f{ values[3*k], values[3*k+1], values[3*k+2] }, mode });
                    }break;

               case cgltf_animation_path_type_rotation:
                    for (size_t k = 0; k < times.size(); ++k) {
                        float frame = times[k] * clip.fps;
                        track.rotation.keyframes.push_back({frame,mathpp::quatf{values[4*k +3],values[4*k],values[4*k+1],values[4*k+2]},mode});
                    }break;
            }
            auto result = std::minmax_element(times.begin(), times.end());
            float m = *result.first;
            float e = *result.second;
            start = std::min(start,m);
            end = std::max(end,m);
        }
        clip.start = start * clip.fps;
        clip.end = end * clip.fps;
        return clip;
    }
}

GLTFModel LoadGLTF(const std::string& path) {
    GLTFModel model;

    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result r = cgltf_parse_file(&options, path.c_str(), &data);
    if (r != cgltf_result_success) { model.error = "cannot read file (cgltf error " + std::to_string(int(r)) + ")"; return model; }
    r = cgltf_load_buffers(&options, data, path.c_str());
    if (r != cgltf_result_success) { model.error = "cannot load buffers"; cgltf_free(data); return model; }
    if (cgltf_validate(data) != cgltf_result_success) { model.error = "file failed validation"; cgltf_free(data); return model; }
    if (data->skins_count > 1) model.warning = "model has several skins, only the first is used";
    if (data->skins_count > 0) model.armature = BuildArmatureFromSkin(&data->skins[0]);
    std::vector<std::vector<int>> meshPrims(data->meshes_count);
    std::cout << "[gltf] file has " << data->materials_count << " materials, "
          << data->textures_count << " textures, "
          << data->images_count << " images\n";

    std::vector<int> imageRemap(data->images_count, -1);          // -1 = not tried, -2 = failed
    model.materials.resize(data->materials_count);

    auto resolveImage = [&](const cgltf_texture* tex) -> int {
        if (!tex) return -1;
        if (!tex->image) { model.warning += "a texture uses an unsupported image type; "; return -1; }
        size_t idx = tex->image - data->images;
        if (imageRemap[idx] == -1) {
            GLTFImage img;
            const char* ext = nullptr;
            if (ExtractImageBytes(tex->image, path, img.bytes) && (ext = SniffExtension(img.bytes))) {
                img.extension = ext;
                model.images.push_back(std::move(img));
                imageRemap[idx] = int(model.images.size()) - 1;
            } else {
                imageRemap[idx] = -2;
                model.warning += "a texture couldn't be read or has an unsupported format; ";
            }
        }
        return imageRemap[idx] >= 0 ? imageRemap[idx] : -1;
    };

    for (size_t i = 0; i < data->materials_count; ++i) {
        const cgltf_material& cm = data->materials[i];
        GLTFMaterial& gm = model.materials[i];

        gm.hasPbr = cm.has_pbr_metallic_roughness;
        if (cm.has_pbr_metallic_roughness) {
            const auto& pbr = cm.pbr_metallic_roughness;
            std::memcpy(gm.baseColorFactor, pbr.base_color_factor, sizeof(float) * 4);
            gm.metallic  = pbr.metallic_factor;
            gm.roughness = pbr.roughness_factor;
            gm.baseColorImage = resolveImage(pbr.base_color_texture.texture);
        }
        std::memcpy(gm.emissiveFactor, cm.emissive_factor, sizeof(float) * 3);
        gm.emissiveImage = resolveImage(cm.emissive_texture.texture);
        gm.uvSet = (gm.baseColorImage >= 0) ? cm.pbr_metallic_roughness.base_color_texture.texcoord
                                    : cm.emissive_texture.texcoord;
    }

    std::cout << "[gltf] extracted images=" << model.images.size()
             << " warning='" << model.warning << "'\n";

    for (size_t m = 0; m < data->meshes_count; ++m) {
        for (size_t p = 0; p < data->meshes[m].primitives_count; ++p) {
            cgltf_primitive& prim = data->meshes[m].primitives[p];
            bool skinned;
            if (prim.type != cgltf_primitive_type_triangles) continue;
            // First find the accessors
            cgltf_accessor* positionAccessor = nullptr;
            cgltf_accessor* normalAccessor   = nullptr;
            cgltf_accessor* texcoordAccessor = nullptr;
            cgltf_accessor* jointsAccessor   = nullptr;
            cgltf_accessor* weightsAccessor  = nullptr;
            const int wantedUV = prim.material ? model.materials[prim.material - data->materials].uvSet : 0;



            for (size_t a = 0; a < prim.attributes_count; ++a) {
                cgltf_attribute& attr = prim.attributes[a];

                switch (attr.type) {
                    case cgltf_attribute_type_position:
                        positionAccessor = attr.data;
                        break;

                    case cgltf_attribute_type_normal:
                        normalAccessor = attr.data;
                        break;

                    case cgltf_attribute_type_texcoord:
                        if (attr.index == wantedUV)
                            texcoordAccessor = attr.data;
                        break;

                    case cgltf_attribute_type_joints:
                        if (attr.index == 0)
                            jointsAccessor = attr.data;
                        break;

                    case cgltf_attribute_type_weights:
                        if (attr.index == 0)
                            weightsAccessor = attr.data;
                        break;

                    default:
                        break;
                }
            }
            skinned = weightsAccessor && jointsAccessor;

            if (!positionAccessor)
                continue;

            size_t vertexCount = positionAccessor->count;
            std::vector<uint32_t> indices;
            if (prim.indices) {
                indices.resize(prim.indices->count);
                for (size_t i = 0; i < prim.indices->count; ++i) {
                    indices[i] = static_cast<uint32_t>(cgltf_accessor_read_index(prim.indices, i));
                }
            } else {
                // non-indexed primitive
                indices.resize(vertexCount);
                for (size_t i = 0; i < vertexCount; ++i) {
                    indices[i] = static_cast<uint32_t>(i);
                }
            }
            if (skinned) {
                std::vector<SkinnedVertex> skinnedVertices;
                skinnedVertices.resize(vertexCount);
                for (size_t v = 0; v < vertexCount; ++v) {
                    SkinnedVertex& vertex = skinnedVertices[v];
                    if (positionAccessor) {
                        float pos[3];
                        cgltf_accessor_read_float(positionAccessor,v,pos,3);
                        vertex.position = {pos[0],pos[1],pos[2]};
                    }
                    if (normalAccessor) {
                        float n[3];
                        cgltf_accessor_read_float(normalAccessor,v,n,3);
                        vertex.normal = {n[0],n[1],n[2]};
                    }
                    if (texcoordAccessor) {
                        float t[2];
                        cgltf_accessor_read_float(texcoordAccessor,v,t,2);
                        vertex.uv = {t[0], 1.0f - t[1]};
                    }
                    if (jointsAccessor) {
                        cgltf_uint joints[4];
                        cgltf_accessor_read_uint(jointsAccessor, v, joints, 4);
                        for (int k = 0; k < 4; ++k) {
                            vertex.boneIndices[k] = static_cast<int>(joints[k]);
                        }
                    }
                    if (weightsAccessor) {
                        float weights[4];
                        cgltf_accessor_read_float(weightsAccessor, v, weights, 4);
                        float sum = weights[0] + weights[1] + weights[2] + weights[3];
                        for (int k = 0; k < 4; ++k) {
                            vertex.boneWeights[k] = (sum > 0.0001f) ? (weights[k] / sum) : weights[k];
                        }
                    }
                }
                model.meshes.emplace_back(skinnedVertices, indices);
                meshPrims[m].push_back(static_cast<int>(model.meshes.size()) - 1);
                model.meshMaterial.push_back(prim.material ? int(prim.material - data->materials) : -1);
            }
            else {
                std::vector<Vertex> vertices;
                vertices.resize(vertexCount);
                for (size_t v = 0; v < vertexCount; ++v) {
                    Vertex& vertex = vertices[v];
                    if (positionAccessor) {
                        float pos[3];
                        cgltf_accessor_read_float(positionAccessor,v,pos,3);
                        vertex.position = {pos[0],pos[1],pos[2]};
                    }
                    if (normalAccessor) {
                        float n[3];
                        cgltf_accessor_read_float(normalAccessor,v,n,3);
                        vertex.normal = {n[0],n[1],n[2]};
                    }
                    if (texcoordAccessor) {
                        float t[2];
                        cgltf_accessor_read_float(texcoordAccessor,v,t,2);
                        vertex.uv = {t[0], 1.0f - t[1]};
                    }
                }
                model.meshes.emplace_back(vertices, indices);
                meshPrims[m].push_back(static_cast<int>(model.meshes.size()) - 1);
                model.meshMaterial.push_back(prim.material ? int(prim.material - data->materials) : -1);
            }
        }
    }

    if (data->animations_count > 0) {
        model.animations.reserve(data->animations_count);
        for (size_t i = 0; i < data->animations_count; ++i) {
            auto a = ImportAnimation(data->animations[i],data);
            if (a.has_value()) {
                model.animations.push_back(std::move(*a));
            }
            else {
                std::cerr << "Animation failed to load" << std::endl;
            }
        }
    }
    model.nodes.reserve(data->nodes_count);
    for (size_t n = 0; n < data->nodes_count; ++n) {
        const cgltf_node& cn = data->nodes[n];
        GLTFNode gn;
        gn.rot.w = 1.0f; gn.rot.x = 0.0f; gn.rot.y = 0.0f; gn.rot.z = 0.0f;
        gn.name   = ResolveNodeName(&cn,data);
        gn.parent = cn.parent ? static_cast<int>(cn.parent - data->nodes) : -1;
        gn.skin   = cn.skin   ? static_cast<int>(cn.skin   - data->skins) : -1;

        if (cn.has_matrix) {
            DecomposeNodeMatrix(cn.matrix, gn);
        } else {
            if (cn.has_translation) gn.pos = {cn.translation[0], cn.translation[1], cn.translation[2]};
            if (cn.has_scale)       gn.scale = {cn.scale[0], cn.scale[1], cn.scale[2]};
            if (cn.has_rotation) {                       // glTF order is x,y,z,w; mathpp is w,x,y,z
                gn.rot.x = cn.rotation[0]; gn.rot.y = cn.rotation[1];
                gn.rot.z = cn.rotation[2]; gn.rot.w = cn.rotation[3];
            }
        }
        if (cn.mesh) gn.meshes = meshPrims[cn.mesh - data->meshes];
        model.nodes.push_back(std::move(gn));

    }
    for (size_t s = 0; s < data->skins_count; ++s)
        for (size_t j = 0; j < data->skins[s].joints_count; ++j)
            model.nodes[data->skins[s].joints[j] - data->nodes].isJoint = true;

    if (model.meshes.empty()) model.error = "no triangle meshes found";

    cgltf_free(data);   // safe: everything was copied into vectors
    return model;
}