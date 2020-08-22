#version 430 core

in float tessCoordY;

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, min(tessCoordY+0.2, 1.0), 0.0, 1.0);
};