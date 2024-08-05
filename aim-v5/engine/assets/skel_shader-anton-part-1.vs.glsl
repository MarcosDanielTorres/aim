#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in int bone_id;
//layout(location = 3) in ivec4 aBoneIds;
//layout(location = 4) in vec4 aWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;


out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;
//flat out ivec4 BoneIds0;
//out vec4 Weights0;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 bone_matrices[32];

out vec3 colour;
void main() {
	colour = vec3(0.0, 0.0, 0.0);
    //vec4 totalPosition = vec4(0.0f);
    //for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    //{
    //    if(boneIds[i] == -1) 
    //        continue;
    //    if(boneIds[i] >=MAX_BONES) 
    //    {
    //        totalPosition = vec4(aPos,1.0f);
    //        break;
    //    }
    //    vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
    //    totalPosition += localPosition * weights[i];
    //}

    	if (bone_id == 0) {
		colour.r = 1.0;
	} else if (bone_id == 1) {
		colour.g = 1.0;
	} else if (bone_id == 2) {
		colour.b = 1.0;
	}

    gl_Position = projection * view * model * bone_matrices[bone_id] * vec4(aPos, 1.0);
	TexCoord0 = aTexCoords;
	Normal0 = aNormal;
	LocalPos0 = aPos;
	//BoneIds0 = aBoneIds;
	//Weights0 = aWeights;

}