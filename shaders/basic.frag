#version 330 core

in vec3 WorldPos;
in vec3 Normal; 
out vec4 FragColor;

uniform float time; 

vec3 cameraPos = vec3(0.0, 3.0, 5.0);
vec3 sunDir = normalize(vec3(sin(time), 0.8, -0.5)); 

void main()
{
    vec3 shallowColor = vec3(0.0, 0.6, 0.7);
    vec3 deepColor = vec3(0.0, 0.1, 0.4);
    float dist = distance(cameraPos, WorldPos);
    float depthFactor = clamp((dist - 2.0) / 10.0, 0.0, 1.0);
    vec3 albedo = mix(shallowColor, deepColor, depthFactor);

    vec2 uv = WorldPos.xz;
    float warp1 = sin(uv.x * 0.8 + time * 0.6) * cos(uv.y * 0.8 - time * 0.4);
    float warp2 = cos(uv.x * 1.1 - time * 0.5) * sin(uv.y * 1.3 + time * 0.7);
    vec2 warpedUV = uv + vec2(warp1, warp2) * 1.2; // Stretches and twists the space

    float foamNoise = sin(warpedUV.x * 3.0 - time * 1.2) * cos(warpedUV.y * 3.0 + time);
    foamNoise += sin(warpedUV.x * 7.0 + time * 2.0) * cos(warpedUV.y * 7.0 - time * 1.5) * 0.4;
    foamNoise = (foamNoise + 1.4) / 2.8;

    // but the noise decides where it breaks apart
    float distortedHeight = WorldPos.y + (foamNoise * 0.12 - 0.06);
    float foamAmount = smoothstep(0.14, 0.19, distortedHeight);
    
    vec3 foamColor = vec3(1.0, 1.0, 1.0); 
    vec3 waterWithFoam = mix(albedo, foamColor, foamAmount);

    vec3 n = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - WorldPos);
    
    vec3 reflectDir = reflect(-sunDir, n);
    float specularAmount = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    specularAmount *= (1.0 - foamAmount); 
    vec3 specularColor = vec3(1.0, 1.0, 1.0) * specularAmount * 0.6;

    float fresnelFactor = pow(max(1.0 - dot(n, viewDir), 0.0), 3.0);
    fresnelFactor *= (1.0 - foamAmount); 
    vec3 fresnelColor = vec3(0.5, 0.8, 0.9) * fresnelFactor * 0.6;

    vec3 finalColor = waterWithFoam + specularColor + fresnelColor;
    FragColor = vec4(finalColor, 1.0);
}