#include "io/gltf/gltf.hpp"
#include "cgltf.h"
#include "render/data/vertex.hpp"
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
                        if (attr.index == 0)
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
                        vertex.uv = {t[0],t[1]};
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
                        vertex.uv = {t[0],t[1]};
                    }
                }
                model.meshes.emplace_back(vertices, indices);
                meshPrims[m].push_back(static_cast<int>(model.meshes.size()) - 1);
            }
        }
    }
    model.nodes.reserve(data->nodes_count);
    for (size_t n = 0; n < data->nodes_count; ++n) {
        const cgltf_node& cn = data->nodes[n];
        GLTFNode gn;
        gn.rot.w = 1.0f; gn.rot.x = 0.0f; gn.rot.y = 0.0f; gn.rot.z = 0.0f;
        gn.name   = cn.name ? cn.name : "";
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