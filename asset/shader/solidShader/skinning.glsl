
layout (location = 3) in ivec4 aBoneIndices;
layout (location = 4) in vec4 aBoneWeights;

uniform mat4 boneMatrices[64]; // cap — revisit with a UBO/SSBO once real rigs exceed this

mat4 GetSkinMatrix() {
    return aBoneWeights.x * boneMatrices[aBoneIndices.x]
    + aBoneWeights.y * boneMatrices[aBoneIndices.y]
    + aBoneWeights.z * boneMatrices[aBoneIndices.z]
    + aBoneWeights.w * boneMatrices[aBoneIndices.w];
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