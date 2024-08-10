#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 3) in uvec4 aBoneIds;
layout(location = 4) in vec4 aWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;


out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;
flat out uvec4 BoneIds0;
out vec4 Weights0;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 finalBonesMatrices[50]; // Adjust the size as needed
uniform mat4 nodeMatrix;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);


	// no se por que lo hace
    //vec4 locPos = model * nodeMatrix * vec4(aPos, 1.0);
	//vec3 outWorldPos = locPos.xyz / locPos.w;
    //gl_Position = projection * view * vec4(outWorldPos, 1.0);

	TexCoord0 = aTexCoords;
	Normal0 = aNormal;
	LocalPos0 = aPos;
	BoneIds0 = aBoneIds;
	Weights0 = aWeights;

}