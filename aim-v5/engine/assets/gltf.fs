#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec4 baseColorFactor;
uniform sampler2D baseColorTexture;
uniform bool hasTexture = false;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    vec3 ambient = 0.1 * lightColor;

    vec3 color = (ambient + diffuse + specular) * baseColorFactor.rgb;
    if (hasTexture) {
        color *= texture(baseColorTexture, texCoords).rgb;
    }

    FragColor = vec4(color, baseColorFactor.a);
}
