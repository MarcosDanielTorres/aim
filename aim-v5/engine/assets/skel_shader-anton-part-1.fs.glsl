#version 330 core

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 LocalPos0;
//flat in ivec4 BoneIds0;
//in vec4 Weights0;
in vec3 colour;
uniform int gDisplayBoneIndex;

out vec4 FragColor;

void main() {

	//bool found = false;
	//for(int i = 0; i < 4; i++) {
	//	if (BoneIds0[i] == gDisplayBoneIndex) {
	//		if (Weights0[i] >= 0.9) {
	//			FragColor = vec4(1.0, 0.0, 0.0, 0.0) * Weights0[i];
	//		} else if(Weights0[i] >= 0.1 && Weights0[i] <= 0.9) {
	//			FragColor = vec4(0.0, 1.0, 0.0, 0.0) * Weights0[i];
	//		}else if (Weights0[i] >= 0.05 && Weights0[i] <= 0.1) {
	//			FragColor = vec4(1.0, 1.0, 0.0, 0.0) * Weights0[i];
	//		}

	//		found = true;
	//		break;
	//	}
	//}
	//if (!found) {
	//	FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	//}
	FragColor = vec4(colour, 1.0);
}