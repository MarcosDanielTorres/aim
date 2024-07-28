#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aJoints;
layout(location = 4) in vec4 aWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 jointMatrices[50]; // Adjust the size as needed

void main()
{
   //mat4 skinMatrix = 
   //     aWeights.x * jointMatrices[int(aJoints.x)] +
   //     aWeights.y * jointMatrices[int(aJoints.y)] +
   //     aWeights.z * jointMatrices[int(aJoints.z)] +
   //     aWeights.w * jointMatrices[int(aJoints.w)];

   // vec4 skinnedPos = skinMatrix * vec4(aPos, 1.0);
   // vec3 skinnedNormal = mat3(skinMatrix) * aNormal;

   // FragPos = vec3(model * skinnedPos);
   // Normal = mat3(transpose(inverse(model))) * skinnedNormal;  
   // TexCoords = aTexCoords;
   // 
   // gl_Position = projection * view * model * skinnedPos;

    vec4 skinnedPos = vec4(aPos, 1.0);
    vec3 skinnedNormal = aNormal;

    FragPos = vec3(model * skinnedPos);
    Normal = mat3(transpose(inverse(model))) * skinnedNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * skinnedPos;
}