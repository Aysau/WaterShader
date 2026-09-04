#version 330 core

layout (location = 0) in vec3 aPosition; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; 

out vec3 WorldPos; 
out vec3 Normal;

void main()
{
    vec3 pos = aPosition;
    
    float arg1 = pos.x * 1.5 + pos.z * 0.8 + time;
    float arg2 = pos.x * -0.5 + pos.z * 1.2 - time * 1.2;
    float arg3 = pos.x * 3.0 - pos.z * 2.0 + time * 0.5;
    
    float w1 = sin(arg1) * 0.15;
    float w2 = cos(arg2) * 0.10;
    float w3 = sin(arg3) * 0.05;
    pos.y += w1 + w2 + w3;

    float dx = (cos(arg1) * 0.15 * 1.5) + 
               (-sin(arg2) * 0.10 * -0.5) + 
               (cos(arg3) * 0.05 * 3.0);
               
    float dz = (cos(arg1) * 0.15 * 0.8) + 
               (-sin(arg2) * 0.10 * 1.2) + 
               (cos(arg3) * 0.05 * -2.0);

    vec3 localNormal = normalize(vec3(-dx, 1.0, -dz));
    
    Normal = mat3(model) * localNormal;

    vec4 worldPosition = model * vec4(pos, 1.0);
    WorldPos = worldPosition.xyz;
    gl_Position = projection * view * worldPosition;
}