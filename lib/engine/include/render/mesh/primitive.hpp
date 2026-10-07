#pragma once

#include <memory>
#include "mesh.hpp"
#include "render/data/vertex.hpp"

#include "component/primitive.hpp"








Mesh inline CreatePlane() {
    Vertex vert1({-1.0f,0.0f,-1.0f},{0.0f,1.0f,0.0f},{0.0f,0.0f});
    Vertex vert2({1.0f,0.0f,-1.0f},{0.0f,1.0f,0.0f},{1.0f,0.0f});
    Vertex vert3({-1.0f,0.0f,1.0f},{0.0f,1.0f,0.0f},{0.0f,1.0f});
    Vertex vert4({1.0f,0.0f,1.0f},{0.0f,1.0f,0.0f},{1.0f,1.0f});
    std::vector<Vertex> planeVert = {vert1,vert2,vert3,vert4};
    std::vector<unsigned int> planeIndices = {0,1,2,1,2,3};

    Mesh mesh(planeVert,planeIndices);

    return  mesh;
}

inline Mesh CreateCube() {
    std::vector<Vertex> verts = {
        // Front face (+z), normal (0,0,1)
        Vertex({-1,-1, 1}, {0,0,1}, {0,0}),
        Vertex({ 1,-1, 1}, {0,0,1}, {1,0}),
        Vertex({ 1, 1, 1}, {0,0,1}, {1,1}),
        Vertex({-1, 1, 1}, {0,0,1}, {0,1}),

        // Back face (-z), normal (0,0,-1)
        Vertex({ 1,-1,-1}, {0,0,-1}, {0,0}),
        Vertex({-1,-1,-1}, {0,0,-1}, {1,0}),
        Vertex({-1, 1,-1}, {0,0,-1}, {1,1}),
        Vertex({ 1, 1,-1}, {0,0,-1}, {0,1}),

        // Right face (+x), normal (1,0,0)
        Vertex({ 1,-1, 1}, {1,0,0}, {0,0}),
        Vertex({ 1,-1,-1}, {1,0,0}, {1,0}),
        Vertex({ 1, 1,-1}, {1,0,0}, {1,1}),
        Vertex({ 1, 1, 1}, {1,0,0}, {0,1}),

        // Left face (-x), normal (-1,0,0)
        Vertex({-1,-1,-1}, {-1,0,0}, {0,0}),
        Vertex({-1,-1, 1}, {-1,0,0}, {1,0}),
        Vertex({-1, 1, 1}, {-1,0,0}, {1,1}),
        Vertex({-1, 1,-1}, {-1,0,0}, {0,1}),

        // Top face (+y), normal (0,1,0)
        Vertex({-1, 1, 1}, {0,1,0}, {0,0}),
        Vertex({ 1, 1, 1}, {0,1,0}, {1,0}),
        Vertex({ 1, 1,-1}, {0,1,0}, {1,1}),
        Vertex({-1, 1,-1}, {0,1,0}, {0,1}),

        // Bottom face (-y), normal (0,-1,0)
        Vertex({-1,-1,-1}, {0,-1,0}, {0,0}),
        Vertex({ 1,-1,-1}, {0,-1,0}, {1,0}),
        Vertex({ 1,-1, 1}, {0,-1,0}, {1,1}),
        Vertex({-1,-1, 1}, {0,-1,0}, {0,1}),
    };

    std::vector<unsigned int> indices;
    for (unsigned int face = 0; face < 6; face++) {
        unsigned int base = face * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        indices.push_back(base + 0);
    }
    Mesh mesh(verts, indices);
    return mesh;
}


Mesh  inline CreateSphere(unsigned int rings, unsigned int segments,float radius) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    //We need segments and rings to simulate real sphere, because computer cannot generate infinite vertices to cover up the surface of the sphere





    for (int ring = 0; ring <= rings; ++ring) {

        //We calculate the vertical angle

        float phi =
            -mathpp::PI<float> / 2.0f +
            mathpp::PI<float> * static_cast<float>(ring) / static_cast<float>(rings);

        for (int segment = 0; segment <= segments; ++segment) {

            float theta =
                2.0f * mathpp::PI<float> * static_cast<float>(segment) / static_cast<float>(segments);
            // We calculate the theta for every single rings and multiply it by segment because segment acts as scalar steps

            // Position
            float x = radius * cos(phi) * cos(theta);
            float y = radius * sin(phi);
            float z = radius * cos(phi) * sin(theta);

            mathpp::vec3f pos{x, y, z};

            // Normal
            mathpp::vec3f normal =
                mathpp::normalize(pos);

            // UV
            float u = static_cast<float>(segment) / segments;
            float v = static_cast<float>(ring) / rings;

            mathpp::vec2f texCoord{u, v};

            vertices.push_back({
                pos,
                normal,
                texCoord
            });
        }
    }
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {

            unsigned int a =
                ring * (segments + 1) + segment;

            unsigned int b = a + 1;

            unsigned int c =
                (ring + 1) * (segments + 1) + segment;

            unsigned int d = c + 1;

            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);

            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }

    Mesh mesh(vertices,indices);
    return  mesh;
}

Mesh inline CreateCylinder(unsigned int segments,float radius, float height) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    // Top vertices
    for (unsigned int segment = 0; segment < segments; ++segment) {
        float theta = 2.0f * mathpp::PI<float> * static_cast<float>(segment) / static_cast<float>(segments);
        float x = radius * cosf(theta);
        float y = height/2;
        float z = radius * sinf(theta);

        mathpp::vec3f pos{x, y, z};
        float normX = cosf(theta);
        float normY = 0;
        float normZ = sinf(theta);
        mathpp::vec3f normal{normX, normY, normZ};
        float u = static_cast<float>(segment) / segments;
        float v = 1;
        mathpp::vec2f texCoord{u, v};
        vertices.push_back({pos,normal,texCoord});
    }
    //bottom vertices
    for (unsigned int segment = 0; segment < segments; ++segment) {
        float theta = 2.0f * mathpp::PI<float> * static_cast<float>(segment) / static_cast<float>(segments);
        float x = radius * cosf(theta);
        float y = -height/2;
        float z = radius * sinf(theta);
        mathpp::vec3f pos{x, y, z};
        float normX = cosf(theta);
        float normY = 0;
        float normZ = sinf(theta);
        mathpp::vec3f normal{normX, normY, normZ};
        float u = static_cast<float>(segment) / segments;
        float v = 0;
        mathpp::vec2f texCoord{u, v};
        vertices.push_back({pos,normal,texCoord});

    }

    for (unsigned int s = 0; s < segments; ++s) {
        unsigned int top0 = s;
        unsigned int top1 = (s + 1) % segments;

        unsigned int bottom0 = segments + s;
        unsigned int bottom1 = segments + (s + 1) % segments;

        indices.push_back(top0);
        indices.push_back(bottom0);
        indices.push_back(top1);

        indices.push_back(top1);
        indices.push_back(bottom0);
        indices.push_back(bottom1);
    }

    unsigned int topCenterIndex = static_cast<unsigned int>(vertices.size());
    vertices.push_back({ {0.0f, height/2, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f} });

    unsigned int topCapStart = static_cast<unsigned int>(vertices.size());
    for (unsigned int segment = 0; segment < segments; ++segment) {
        float theta = 2.0f * mathpp::PI<float> * static_cast<float>(segment) / static_cast<float>(segments);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        mathpp::vec3f pos{x, height/2, z};
        mathpp::vec3f normal{0.0f, 1.0f, 0.0f}; // flat up, not radial
        mathpp::vec2f texCoord{0.5f + 0.5f*cosf(theta), 0.5f + 0.5f*sinf(theta)};
        vertices.push_back({pos, normal, texCoord});
    }
    for (unsigned int s = 0; s < segments; ++s) {
        unsigned int curr = topCapStart + s;
        unsigned int next = topCapStart + (s + 1) % segments;
        indices.push_back(topCenterIndex);
        indices.push_back(curr);
        indices.push_back(next);
    }

    // --- Bottom cap ---
    unsigned int botCenterIndex = static_cast<unsigned int>(vertices.size());
    vertices.push_back({ {0.0f, -height/2, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f} });

    unsigned int botCapStart = static_cast<unsigned int>(vertices.size());
    for (unsigned int segment = 0; segment < segments; ++segment) {
        float theta = 2.0f * mathpp::PI<float> * static_cast<float>(segment) / static_cast<float>(segments);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        mathpp::vec3f pos{x, -height/2, z};
        mathpp::vec3f normal{0.0f, -1.0f, 0.0f};
        mathpp::vec2f texCoord{0.5f + 0.5f*cosf(theta), 0.5f + 0.5f*sinf(theta)};
        vertices.push_back({pos, normal, texCoord});
    }
    for (unsigned int s = 0; s < segments; ++s) {
        unsigned int curr = botCapStart + s;
        unsigned int next = botCapStart + (s + 1) % segments;
        // note: reversed winding vs top cap, since bottom faces the opposite direction
        indices.push_back(botCenterIndex);
        indices.push_back(next);
        indices.push_back(curr);
    }

    Mesh mesh(vertices, indices);
    return  mesh;
}


std::unique_ptr<Mesh> inline CreateGridPlane(float extent) {
    Vertex vert1({-extent, 0.0f, -extent}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f});
    Vertex vert2({ extent, 0.0f, -extent}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f});
    Vertex vert3({-extent, 0.0f,  extent}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f});
    Vertex vert4({ extent, 0.0f,  extent}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f});
    std::vector<Vertex> vertices = {vert1, vert2, vert3, vert4};
    std::vector<unsigned int> planeIndices = {0, 1, 2, 1, 2, 3};
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(vertices, planeIndices);

    return mesh;
}

Mesh inline CreateCone(unsigned int segments, float radius, float height) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float slantLen = sqrtf(height * height + radius * radius);
    float normalXZScale = height / slantLen;
    float normalYScale = radius / slantLen;

    // Apex vertex (one shared point at the tip — but see note below on why this is actually wrong for shading)
    unsigned int apexIndex = 0;
    vertices.push_back({ {0.0f, height, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f} }); // placeholder normal, fixed below

    // Base ring (side surface, slanted normals)
    unsigned int baseStart = static_cast<unsigned int>(vertices.size());
    for (unsigned int segment = 0; segment < segments; ++segment) {
        float theta = 2.0f * mathpp::PI<float> * static_cast<float>(segment) / static_cast<float>(segments);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        mathpp::vec3f pos{x, 0.0f, z};
        mathpp::vec3f normal{cosf(theta) * normalXZScale, normalYScale, sinf(theta) * normalXZScale};
        mathpp::vec2f texCoord{static_cast<float>(segment) / segments, 0.0f};
        vertices.push_back({pos, normal, texCoord});
    }

    // Side triangles: apex to each base edge
    for (unsigned int s = 0; s < segments; ++s) {
        unsigned int curr = baseStart + s;
        unsigned int next = baseStart + (s + 1) % segments;
        indices.push_back(apexIndex);
        indices.push_back(curr);
        indices.push_back(next);
    }

    // Bottom cap (flat, same fan-from-center technique as the cylinder)
    unsigned int botCenterIndex = static_cast<unsigned int>(vertices.size());
    vertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f} });

    unsigned int botCapStart = static_cast<unsigned int>(vertices.size());
    for (unsigned int segment = 0; segment < segments; ++segment) {
        float theta = 2.0f * mathpp::PI<float> * static_cast<float>(segment) / static_cast<float>(segments);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        vertices.push_back({ {x, 0.0f, z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f*cosf(theta), 0.5f + 0.5f*sinf(theta)} });
    }
    for (unsigned int s = 0; s < segments; ++s) {
        unsigned int curr = botCapStart + s;
        unsigned int next = botCapStart + (s + 1) % segments;
        indices.push_back(botCenterIndex);
        indices.push_back(next);
        indices.push_back(curr);
    }

    Mesh mesh(vertices, indices);
    return mesh;
}


