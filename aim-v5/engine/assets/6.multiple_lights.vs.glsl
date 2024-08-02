#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
//layout (location = 3) in int bone_id;


layout(location = 3) in vec4 aJoints;
layout(location = 4) in vec4 aWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 boneTransforms[50]; // Adjust the size as needed
uniform mat4 bone_matrices[32];

//void main()
//{
//   mat4 skinMatrix = 
//        aWeights.x * boneTransforms[int(aJoints.x)] +
//        aWeights.y * boneTransforms[int(aJoints.y)] +
//        aWeights.z * boneTransforms[int(aJoints.z)] +
//        aWeights.w * boneTransforms[int(aJoints.w)];
//
//   //mat4 skinMatrix = boneTransforms[int(aJoints.x)] * aWeights.x +
//   //         boneTransforms[int(aJoints.y)] * aWeights.y +
//   //         boneTransforms[int(aJoints.z)] * aWeights.z +
//   //         boneTransforms[int(aJoints.w)] * aWeights.w;
//
//    vec4 skinnedPos = skinMatrix * vec4(aPos, 1.0);
//    vec3 skinnedNormal = mat3(skinMatrix) * aNormal;
//
//    FragPos = vec3(model * skinnedPos);
//    Normal = mat3(transpose(inverse(model))) * skinnedNormal;  
//    TexCoords = aTexCoords;
//    
//    gl_Position = projection * view * model * skinnedPos;
//
//   // vec4 skinnedPos = vec4(aPos, 1.0);
//   // vec3 skinnedNormal = aNormal;
//
//   // FragPos = vec3(model * skinnedPos);
//   // Normal = mat3(transpose(inverse(model))) * skinnedNormal;
//   // TexCoords = aTexCoords;
//
//   // gl_Position = projection * view * model * bone_matrices[bone_id] * skinnedPos;
//}







//#version 330 core
//
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec2 aTexCoords;
//layout(location = 3) in ivec4 aJoints;
//layout(location = 4) in vec4 aWeights;
//
//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
//uniform mat4 boneTransforms[100]; // Adjust the size according to the maximum number of bones in your model
//
//out vec2 TexCoords;
//out vec3 FragPos;
//out vec3 Normal;
//
void main() {
    // Skinning: Calculate the final position and normal based on the bone transformations
   mat4 skinningMatrix = 
        aWeights.x * boneTransforms[int(aJoints.x)] +
        aWeights.y * boneTransforms[int(aJoints.y)] +
        aWeights.z * boneTransforms[int(aJoints.z)] +
        aWeights.w * boneTransforms[int(aJoints.w)];

    vec4 skinnedPosition = skinningMatrix * vec4(aPos, 1.0);
    vec3 skinnedNormal = mat3(skinningMatrix) * aNormal;

    // Apply the model, view, and projection transformations
    FragPos = vec3(model * skinnedPosition);
    Normal = mat3(transpose(inverse(model))) * skinnedNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * skinnedPosition;
}