#version 430 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vV1;
layout (location = 2) in vec4 vV2;
layout (location = 3) in vec4 props;

out vec4 tcV1;
out vec4 tcV2;
out vec4 tcProps;

void main()
{
    tcV1 = vV1;
    tcV2 = vV2;
    tcProps = props;

    gl_Position = vPosition;
};

