#version 330 core
out vec4 FragColor;

uniform vec4 line_color;

void main()
{
    FragColor = line_color;
}