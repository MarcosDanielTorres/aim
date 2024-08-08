#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 inJointIndices;
layout(location = 4) in vec4 inJointWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;


out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;
flat out ivec4 BoneIds0;
out vec4 Weights0;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 jointMatrices[50]; // Adjust the size as needed

void main() {
	// Calculate skinned matrix from weights and joint indices of the current vertex
	mat4 skinMat = 
		inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
		inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
		inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
		inJointWeights.w * jointMatrices[int(inJointIndices.w)];

	gl_Position = projection * view * model * skinMat * vec4(aPos.xyz, 1.0);
	//Normal0 = aNormal;
	Normal0 = normalize(transpose(inverse(mat3(view * model * skinMat))) * aNormal);

	TexCoord0 = aTexCoords;
	LocalPos0 = aPos;
}