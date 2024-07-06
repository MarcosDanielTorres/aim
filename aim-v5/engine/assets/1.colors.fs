#version 330 core
out vec4 FragColor;

in vec3 NormalFromWorldSpace;
in vec3 NormalFromViewSpace;
in vec3 FragPos;
in vec3 FragPosFromViewSpace;
in vec3 LightPosFromViewSpace;
in vec2 TexCoords;
  
uniform vec3 objectColor;
// uniform vec3 lightColor; lo saco de aca por lo meto en `struct Light`
// uniform vec3 lightPos; lo saco de aca por lo meto en `struct Light`
uniform vec3 viewPos;

struct Material {
    // value of colors the object has for each type of lighting
    //vec3 ambient; // borro a la mierda el ambient porque es igual al diffuse y lo voy a traer desde el diffuse map
    //vec3 diffuse;

    sampler2D diffuse;
    //vec3 specular;
    sampler2D specular;
    // value of colors the object has for each type of lighting
    float shininess;
};

uniform Material material;


struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;  

void main() {
    //world space
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    //vec3 ambient = material.ambient * light.ambient; esto al final lo comento porque lo tengo que leer del diffuse map
    // esto estaba antes de usar un color por cada etapa
    //float ambientStrength = 0.1;
    //vec3 ambient = ambientStrength * lightColor;
    // esto estaba antes de usar un color por cada etapa
  	
    // diffuse 
    vec3 norm = normalize(NormalFromWorldSpace);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
    // esto estaba antes de usar un color por cada etapa
    //vec3 diffuse = diff * lightColor;
    // esto estaba antes de usar un color por cada etapa
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.00001), material.shininess); // pow controls the shininess
    vec3 specular = texture(material.specular, TexCoords).rgb * spec * light.specular;  
    //vec3 specular = (material.specular * spec) * light.specular;  
    // esto estaba antes de usar un color por cada etapa
    //float specularStrength = 0.5;
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // pow controls the shininess
    //vec3 specular = specularStrength * spec * lightColor;  
    // esto estaba antes de usar un color por cada etapa
        

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);


    /*
    // view space
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;    
    
     // diffuse 
    vec3 norm = normalize(NormalFromViewSpace);
    vec3 lightDir = normalize(LightPosFromViewSpace - FragPosFromViewSpace);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(-FragPosFromViewSpace); // the viewer is always at (0,0,0) in view-space, so viewDir is (0,0,0) - Position => -Position
                                                     // otherwise I need to pass the `camera.position`
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; 
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
    */

}