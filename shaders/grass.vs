#version 430 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vV1;
layout (location = 2) in vec4 vV2;
layout (location = 3) in vec4 props;
layout (location = 4) in float age;

out vec4 tcV1;
out vec4 tcV2;
out vec4 tcProps;

out vec3 tcBladeDir;

out float tcAge;
//out vec3 tcBladeUp;

void main()
{
    tcV1 = vV1;
    tcV2 = vV2;
    tcProps = props;

    gl_Position = vPosition;

    float dir = vPosition.w;
    float sinDir = sin(dir);
    float cosDir = cos(dir);
    tcBladeDir = normalize(cross(props.xyz, normalize(vec3(sinDir, sinDir+cosDir, cosDir))));

    tcAge = age;
    //tcAge = 0.1;

};

