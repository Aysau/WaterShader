#version 330 core

layout (location = 0) in vec3 aPosition; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform int normalMode; //0 = Approx, 1 = Exact, 2 = Finite Difference

out vec3 WorldPos; 
out vec3 Normal;

const vec2 d1 = vec2(0.8944, 0.4472); // normalize(1.0, 0.5)
const float q1 = 0.5, a1 = 0.15, w1 = 1.5, s1 = 1.0;

const vec2 d2 = vec2(0.5299, -0.8479); // normalize(0.5, -0.8)
const float q2 = 0.5, a2 = 0.10, w2 = 2.5, s2 = 1.2;

const vec2 d3 = vec2(-0.5547, 0.8320); // normalize(-0.2, 0.3)
const float q3 = 0.5, a3 = 0.05, w3 = 4.0, s3 = 2.0;

//Calculate exact physical position of a Gerstner wave
vec3 getWavePosition(vec2 gridPoint) {
    vec3 p = vec3(gridPoint.x, 0.0, gridPoint.y);
    
    float phase1 = dot(d1, gridPoint) * w1 + time * s1;
    p.x += q1 * a1 * d1.x * cos(phase1);
    p.z += q1 * a1 * d1.y * cos(phase1);
    p.y += a1 * sin(phase1);
    
    float phase2 = dot(d2, gridPoint) * w2 + time * s2;
    p.x += q2 * a2 * d2.x * cos(phase2);
    p.z += q2 * a2 * d2.y * cos(phase2);
    p.y += a2 * sin(phase2);
    
    float phase3 = dot(d3, gridPoint) * w3 + time * s3;
    p.x += q3 * a3 * d3.x * cos(phase3);
    p.z += q3 * a3 * d3.y * cos(phase3);
    p.y += a3 * sin(phase3);
    
    return p;
}

void main()
{
    vec2 gridPoint = aPosition.xz;
    vec3 displacedPos = getWavePosition(gridPoint);
    vec3 localNormal = vec3(0.0, 1.0, 0.0);

    //Mode 0: GPU Gems Approximation (Summed Normals)
    if (normalMode == 0) {
        float phase1 = dot(d1, gridPoint) * w1 + time * s1;
        float phase2 = dot(d2, gridPoint) * w2 + time * s2;
        float phase3 = dot(d3, gridPoint) * w3 + time * s3;

        localNormal.x = -(d1.x * w1 * a1 * cos(phase1) + d2.x * w2 * a2 * cos(phase2) + d3.x * w3 * a3 * cos(phase3));
        localNormal.z = -(d1.y * w1 * a1 * cos(phase1) + d2.y * w2 * a2 * cos(phase2) + d3.y * w3 * a3 * cos(phase3));
        localNormal.y = 1.0 - (q1 * w1 * a1 * sin(phase1) + q2 * w2 * a2 * sin(phase2) + q3 * w3 * a3 * sin(phase3));
        localNormal = normalize(localNormal);
    }
    //Mode 1: Exact Analytical Cross Product
    else if (normalMode == 1) {
        vec3 Tx = vec3(1.0, 0.0, 0.0);
        vec3 Tz = vec3(0.0, 0.0, 1.0);
        
        float p1 = dot(d1, gridPoint) * w1 + time * s1;
        float p2 = dot(d2, gridPoint) * w2 + time * s2;
        float p3 = dot(d3, gridPoint) * w3 + time * s3;

        // Accumulate derivatives for Tx
        Tx.x -= (q1 * a1 * w1 * d1.x * d1.x * sin(p1)) + (q2 * a2 * w2 * d2.x * d2.x * sin(p2)) + (q3 * a3 * w3 * d3.x * d3.x * sin(p3));
        Tx.y += (a1 * w1 * d1.x * cos(p1)) + (a2 * w2 * d2.x * cos(p2)) + (a3 * w3 * d3.x * cos(p3));
        Tx.z -= (q1 * a1 * w1 * d1.x * d1.y * sin(p1)) + (q2 * a2 * w2 * d2.x * d2.y * sin(p2)) + (q3 * a3 * w3 * d3.x * d3.y * sin(p3));
        
        // Accumulate derivatives for Tz
        Tz.x -= (q1 * a1 * w1 * d1.y * d1.x * sin(p1)) + (q2 * a2 * w2 * d2.y * d2.x * sin(p2)) + (q3 * a3 * w3 * d3.y * d3.x * sin(p3));
        Tz.y += (a1 * w1 * d1.y * cos(p1)) + (a2 * w2 * d2.y * cos(p2)) + (a3 * w3 * d3.y * cos(p3));
        Tz.z -= (q1 * a1 * w1 * d1.y * d1.y * sin(p1)) + (q2 * a2 * w2 * d2.y * d2.y * sin(p2)) + (q3 * a3 * w3 * d3.y * d3.y * sin(p3));
        
        localNormal = normalize(cross(Tz, Tx));
    }
    //Mode 2: Finite Difference Reference
    else if (normalMode == 2) {
        float eps = 0.01;
        vec3 posDx = getWavePosition(gridPoint + vec2(eps, 0.0));
        vec3 posDz = getWavePosition(gridPoint + vec2(0.0, eps));
        
        vec3 Tx = posDx - displacedPos;
        vec3 Tz = posDz - displacedPos;
        localNormal = normalize(cross(Tz, Tx));
    }

    Normal = mat3(model) * localNormal;
    vec4 worldPosition = model * vec4(displacedPos, 1.0);
    WorldPos = worldPosition.xyz;
    gl_Position = projection * view * worldPosition;
}