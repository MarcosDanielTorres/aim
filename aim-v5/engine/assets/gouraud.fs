#version 330 core
out vec4 FragColor;

in vec3 vertexOutputColor;
uniform vec3 objectColor;

void main()
{
    FragColor = vec4(vertexOutputColor * objectColor, 1.0);
}
