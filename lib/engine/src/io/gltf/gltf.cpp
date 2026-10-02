#include "io/gltf/gltf.hpp"
#include "cgltf.h"
#include "render/data/vertex.hpp"

GLTFModel LoadGLTF(const std::string& path) {
    GLTFModel model;

    cgltf_options options = {};
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success) return model;
    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) { cgltf_free(data); return model; }
    if (cgltf_validate(data) != cgltf_result_success) { cgltf_free(data); return model; }

    if (data->skins_count > 0) {
        model.armature = BuildArmatureFromSkin(&data->skins[0]);   // yours
    }

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
            }
        }
    }

    cgltf_free(data);   // safe: everything was copied into vectors
    return model;
}