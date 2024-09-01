#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 3) in uvec4 inJointIndices; // esto no andaba si lo dejaba como uvec4
// en realidad anda con uvec4 pero por alguna razon me hizo confundir esto. creo que no guarde y pense que no andaba con uvec4
layout(location = 4) in vec4 inJointWeights;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
flat out ivec4 BoneIds0;
out vec4 Weights0;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 jointMatrices[250]; // Adjust the size as needed
uniform int jointCount; // si le dejo uint y el jointcount es uint32_t no anda. Si dejo int y el jointcount sigue siendo uint32_t anda.
uniform mat4 nodeMatrix;

void main() {
	vec4 locPos;
	if (jointCount > 0) {
		mat4 skinMat = 
			inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
			inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
			inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
			inJointWeights.w * jointMatrices[int(inJointIndices.w)];

		locPos = model * nodeMatrix * skinMat * vec4(aPos, 1.0);
		Normal = normalize(transpose(inverse(mat3(model * nodeMatrix * skinMat))) * aNormal);
	}else{
		locPos = model * nodeMatrix * vec4(aPos, 1.0);
		Normal = normalize(transpose(inverse(mat3(model * nodeMatrix))) * aNormal);
	}


	vec4 outWorldPos = locPos;
	gl_Position = projection * view * outWorldPos;

    TexCoords = aTexCoords;
	FragPos = vec3(outWorldPos);
}