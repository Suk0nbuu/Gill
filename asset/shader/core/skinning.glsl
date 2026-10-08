
layout (location = 3) in ivec4 aBoneIndices;
layout (location = 4) in vec4 aBoneWeights;

layout(std140) uniform Skin { mat4 boneMatrices[64]; };

mat4 GetSkinMatrix() {
    float total = aBoneWeights.x + aBoneWeights.y + aBoneWeights.z + aBoneWeights.w;
    if (total < 0.0001) return mat4(1.0);
    return (aBoneWeights.x * boneMatrices[aBoneIndices.x]
    + aBoneWeights.y * boneMatrices[aBoneIndices.y]
    + aBoneWeights.z * boneMatrices[aBoneIndices.z]
    + aBoneWeights.w * boneMatrices[aBoneIndices.w]) / total;
}

vec3 GetSkinnedPosition(vec3 localPos) {
    mat4 skin = GetSkinMatrix();
    return (skin * vec4(localPos, 1.0)).xyz;
}

vec3 GetSkinnedNormal(vec3 localNormal) {
    mat4 skin = GetSkinMatrix();
    // plain skinMatrix, not inverse-transpose — correct as long as bones don't carry non-uniform scale, per earlier note
    return normalize((skin * vec4(localNormal, 0.0)).xyz);
}