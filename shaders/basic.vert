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
    //We have to use the original, undisplaced grid coordinates for the math.
    //If we feed displaced XZ coordinates into the next wave, the mesh will tear.
    vec2 gridPoint = aPosition.xz; 
    
    vec3 n = vec3(0.0, 1.0, 0.0);

    //Wave 1 (Main sweeping wave)
    vec2 d1 = normalize(vec2(1.0, 0.5)); // Direction
    float q1 = 0.5;  // Steepness
    float a1 = 0.15; // Amplitude
    float w1 = 1.5;  // Frequency
    float s1 = 1.0;  // Speed
    
    float phase1 = dot(d1, gridPoint) * w1 + time * s1;
    float cos1 = cos(phase1);
    float sin1 = sin(phase1);
    
    //Displace Position (X, Y, Z)
    pos.x += q1 * a1 * d1.x * cos1;
    pos.z += q1 * a1 * d1.y * cos1;
    pos.y += a1 * sin1;
    
    //Accumulate Normal Derivatives
    n.x -= d1.x * w1 * a1 * cos1;
    n.z -= d1.y * w1 * a1 * cos1;
    n.y -= q1 * w1 * a1 * sin1;

    //Wave 2 (Secondary crossing wave)
    vec2 d2 = normalize(vec2(0.5, -0.8));
    float q2 = 0.5;
    float a2 = 0.10;
    float w2 = 2.5;
    float s2 = 1.2;
    
    float phase2 = dot(d2, gridPoint) * w2 + time * s2;
    float cos2 = cos(phase2);
    float sin2 = sin(phase2);
    
    pos.x += q2 * a2 * d2.x * cos2;
    pos.z += q2 * a2 * d2.y * cos2;
    pos.y += a2 * sin2;
    
    n.x -= d2.x * w2 * a2 * cos2;
    n.z -= d2.y * w2 * a2 * cos2;
    n.y -= q2 * w2 * a2 * sin2;

    //Wave 3 (High-frequency choppy wave)
    vec2 d3 = normalize(vec2(-0.2, 0.3));
    float q3 = 0.5;
    float a3 = 0.05;
    float w3 = 4.0;
    float s3 = 2.0;
    
    float phase3 = dot(d3, gridPoint) * w3 + time * s3;
    float cos3 = cos(phase3);
    float sin3 = sin(phase3);
    
    pos.x += q3 * a3 * d3.x * cos3;
    pos.z += q3 * a3 * d3.y * cos3;
    pos.y += a3 * sin3;
    
    n.x -= d3.x * w3 * a3 * cos3;
    n.z -= d3.y * w3 * a3 * cos3;
    n.y -= q3 * w3 * a3 * sin3;

    vec3 localNormal = normalize(n);
    Normal = mat3(model) * localNormal;

    vec4 worldPosition = model * vec4(pos, 1.0);
    WorldPos = worldPosition.xyz;
    gl_Position = projection * view * worldPosition;
}