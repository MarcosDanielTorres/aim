#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct DirectionalLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define NR_POINT_LIGHTS 4
//#define LIGHT_TEXTURES

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirectionalLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform vec4 baseColorFactor;

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 norm = normalize(Normal);	
	vec3 viewDir = normalize(viewPos - FragPos);

	// phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    // phase 3: spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    

    // depth visualization
    float near = 0.1; 
    float far  = 100.0; 
    float z = gl_FragCoord.z * 2.0 - 1.0; // back to ndc
    float linearized_depth = (2.0 * near * far) / (far + near - z * (far - near));
    float depth = linearized_depth / far; // because most values will range from 0 to 100 (near to far) most values will be white so dividing it by far (100) normalizes them to [0, 1]
    FragColor = vec4(vec3(depth), 1.0);
    vec4 depthVec4 = vec4(vec3(pow(depth, 1.4)), 1.0);
    //FragColor = vec4(result, 1.0) *  (1 - depthVec4) + depthVec4; // use the depth to add fog
    // depth visualization

    // Apply baseColorFactor to the final color result
    //result *= baseColorFactor.rgb;

    // standard behaviour
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    // These are commented so I dont use a diffuse and specular texture. Note should be done in everywhere where textures are used (currently 3 other functions)
    #ifdef LIGHT_TEXTURES
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    #else
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    #endif
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    // These are commented so I dont use a diffuse and specular texture. Note should be done in everywhere where textures are used (currently 3 other functions)
    #ifdef LIGHT_TEXTURES
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    #else
    vec3 ambient = light.ambient ;
    vec3 diffuse = light.diffuse * diff ;
    vec3 specular = light.specular * spec ;
    #endif
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    // These are commented so I dont use a diffuse and specular texture. Note should be done in everywhere where textures are used (currently 3 other functions)
    #ifdef LIGHT_TEXTURES
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    #else
    vec3 ambient = light.ambient ;
    vec3 diffuse = light.diffuse * diff ;
    vec3 specular = light.specular * spec ;
    #endif
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}









//#version 330 core
//out vec4 FragColor;
//
//struct DirectionalLight {
//    vec3 direction;
//    vec3 ambient;
//};
//
//struct PointLight {
//    vec3 position;
//    float constant;
//    float linear;
//    float quadratic;
//    vec3 ambient;
//};
//
//struct SpotLight {
//    vec3 position;
//    vec3 direction;
//    float cutOff;
//    float outerCutOff;
//    float constant;
//    float linear;
//    float quadratic;
//    vec3 ambient;
//};
//
//#define NR_POINT_LIGHTS 4
//
//in vec3 FragPos;
//in vec3 Normal;
//
//uniform vec3 viewPos;
//uniform DirectionalLight dirLight;
//uniform PointLight pointLights[NR_POINT_LIGHTS];
//uniform SpotLight spotLight;
//
//uniform vec4 baseColorFactor;
//
//const float PI = 3.14159265359;
//
//vec3 fresnelSchlick(float cosTheta, vec3 F0);
//float DistributionGGX(vec3 N, vec3 H, float roughness);
//float GeometrySchlickGGX(float NdotV, float roughness);
//float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
//vec3 calcDirectionalLight(DirectionalLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness);
//vec3 calcPointLight(PointLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 fragPos);
//vec3 calcSpotLight(SpotLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 fragPos);
//
//void main()
//{
//    vec3 N = normalize(Normal);
//    vec3 V = normalize(viewPos - FragPos);
//
//    // Material properties
//    vec3 albedo = baseColorFactor.rgb;
//    float metallic = 0.5;  // Default metallic value
//    float roughness = 0.5; // Default roughness value
//    float ao = 1.0;        // Assume ambient occlusion is 1.0
//
//    // Calculate lighting
//    vec3 lighting = calcDirectionalLight(dirLight, N, V, albedo, metallic, roughness);
//    for(int i = 0; i < NR_POINT_LIGHTS; i++)
//        lighting += calcPointLight(pointLights[i], N, V, albedo, metallic, roughness, FragPos);
//    lighting += calcSpotLight(spotLight, N, V, albedo, metallic, roughness, FragPos);
//
//    // Apply ambient occlusion
//    lighting = mix(vec3(0.03) * albedo, lighting, ao);
//
//    FragColor = vec4(lighting, baseColorFactor.a);
//}
//
//vec3 fresnelSchlick(float cosTheta, vec3 F0)
//{
//    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}
//
//float DistributionGGX(vec3 N, vec3 H, float roughness)
//{
//    float a = roughness * roughness;
//    float a2 = a * a;
//    float NdotH = max(dot(N, H), 0.0);
//    float NdotH2 = NdotH * NdotH;
//
//    float num = a2;
//    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//    denom = PI * denom * denom;
//
//    return num / max(denom, 0.00001);
//}
//
//float GeometrySchlickGGX(float NdotV, float roughness)
//{
//    float r = (roughness + 1.0);
//    float k = (r * r) / 8.0;
//
//    float num = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//
//    return num / max(denom, 0.00001);
//}
//
//float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
//{
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
//    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
//    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
//
//    return ggx1 * ggx2;
//}
//
//vec3 calcDirectionalLight(DirectionalLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness)
//{
//    vec3 L = normalize(-light.direction);
//    vec3 H = normalize(V + L);
//
//    vec3 radiance = vec3(1.0); // Default light intensity
//
//    vec3 F0 = vec3(0.04);
//    F0 = mix(F0, albedo, metallic);
//
//    float NDF = DistributionGGX(N, H, roughness);
//    float G = GeometrySmith(N, V, L, roughness);
//    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(N, L), 0.0);
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 ambient = light.ambient * albedo;
//    vec3 diffuse = kD * albedo / PI;
//
//    return ambient + (diffuse + specular) * radiance * NdotL;
//}
//
//vec3 calcPointLight(PointLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 fragPos)
//{
//    vec3 L = normalize(light.position - fragPos);
//    vec3 H = normalize(V + L);
//
//    float distance = length(light.position - fragPos);
//    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
//    vec3 radiance = vec3(1.0) * attenuation; // Default light intensity
//
//    vec3 F0 = vec3(0.04);
//    F0 = mix(F0, albedo, metallic);
//
//    float NDF = DistributionGGX(N, H, roughness);
//    float G = GeometrySmith(N, V, L, roughness);
//    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(N, L), 0.0);
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 ambient = light.ambient * albedo;
//    vec3 diffuse = kD * albedo / PI;
//
//    return ambient + (diffuse + specular) * radiance * NdotL;
//}
//
//vec3 calcSpotLight(SpotLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 fragPos)
//{
//    vec3 L = normalize(light.position - fragPos);
//    vec3 H = normalize(V + L);
//
//    float distance = length(light.position - fragPos);
//    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
//    
//    float theta = dot(L, normalize(-light.direction));
//    float epsilon = light.cutOff - light.outerCutOff;
//    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
//    vec3 radiance = vec3(1.0) * attenuation * intensity; // Default light intensity
//
//    vec3 F0 = vec3(0.04);
//    F0 = mix(F0, albedo, metallic);
//
//    float NDF = DistributionGGX(N, H, roughness);
//    float G = GeometrySmith(N, V, L, roughness);
//    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(N, L), 0.0);
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 ambient = light.ambient * albedo;
//    vec3 diffuse = kD * albedo / PI;
//
//    return ambient + (diffuse + specular) * radiance * NdotL;
//}
//
