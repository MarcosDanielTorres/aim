#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 FragPosFromViewSpace;
out vec3 NormalFromWorldSpace;
out vec3 NormalFromViewSpace;

uniform vec3 lightPos;
out vec3 LightPosFromViewSpace;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	FragPosFromViewSpace = vec3(view * model * vec4(aPos, 1.0));
	//Normal = aNormal;
	NormalFromWorldSpace = mat3(transpose(inverse(model))) * aNormal; 

	NormalFromViewSpace = mat3(transpose(inverse(view * model))) * aNormal; 
	LightPosFromViewSpace = vec3(view *vec4(lightPos, 1.0)); // lightPos is already in world-space, that's why I don't multiply it by the `model` matrix.
}