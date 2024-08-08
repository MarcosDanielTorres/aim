#version 330 core

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 LocalPos0;
flat in ivec4 BoneIds0;
in vec4 Weights0;

uniform int gDisplayBoneIndex;

out vec4 FragColor;

void main() {
	FragColor = vec4(0.8, 0.8, 0.8, 1.0);
}