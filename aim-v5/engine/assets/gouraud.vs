#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vertexOutputColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos; // camera.position
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	vec3 ObjWorldPos = vec3(model * vec4(aPos, 1.0));
	vec3 NormalFromWorldSpace = mat3(transpose(inverse(model))) * aNormal; 

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(NormalFromWorldSpace);
    vec3 lightDir = normalize(lightPos - ObjWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 1.0;
    vec3 viewDir = normalize(viewPos - ObjWorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vertexOutputColor = (ambient + diffuse + specular);
}

/*

If I move the camera away from the cube and look at it diagonally I can clearly see the individiaul triangles of the mesh

So what do we see?
You can see (for yourself or in the provided image) the clear distinction of the two triangles at the front of the 
cube. This 'stripe' is visible because of fragment interpolation. From the example image we can see that the top-right 
vertex of the cube's front face is lit with specular highlights. Since the top-right vertex of the bottom-right triangle is 
lit and the other 2 vertices of the triangle are not, the bright values interpolates to the other 2 vertices. The same 
happens for the upper-left triangle. Since the intermediate fragment colors are not directly from the light source 
but are the result of interpolation, the lighting is incorrect at the intermediate fragments and the top-left and 
bottom-right triangle collide in their brightness resulting in a visible stripe between both triangles.

This effect will become more apparent when using more complicated shapes.
*/
