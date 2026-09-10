#version 330 core

in vec3 WorldPos;
in vec3 Normal; 
out vec4 FragColor;

uniform float time; 

vec3 cameraPos = vec3(0.0, 3.0, 5.0);
vec3 sunDir = normalize(vec3(sin(time), 0.8, -0.5)); 

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(a, b, u.x) + 
           (c - a) * u.y * (1.0 - u.x) + 
           (d - b) * u.x * u.y;
}

void main()
{
    //Color gradient
    vec3 shallowColor = vec3(0.0, 0.6, 0.7);
    vec3 deepColor = vec3(0.0, 0.1, 0.4);
    float dist = distance(cameraPos, WorldPos);
    float depthFactor = clamp((dist - 2.0) / 10.0, 0.0, 1.0);
    vec3 albedo = mix(shallowColor, deepColor, depthFactor);

    //Crest foam
    //Strict height mask: only trigger on the top 10% of the wave peaks
    float crestMask = smoothstep(0.20, 0.28, WorldPos.y);

    //High-frequency noise (scaled up to 6.0) to create sea spray
    vec2 noiseCoord = WorldPos.xz * 6.0;
    noiseCoord.x += time * 0.8;
    noiseCoord.y -= time * 0.4;
    
    float fbmNoise = noise(noiseCoord) * 0.6 + noise(noiseCoord * 2.5) * 0.4;

    //Multiply height mask by noise to subtract coverage
    float foamIntensity = crestMask * fbmNoise;
    float foamAmount = smoothstep(0.25, 0.45, foamIntensity);

    vec3 foamColor = vec3(1.0, 1.0, 1.0); 
    vec3 waterWithFoam = mix(albedo, foamColor, foamAmount);

    //Specular and Fresnel highlights
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - WorldPos);
    
    vec3 reflectDir = reflect(-sunDir, norm);
    float specularAmount = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    specularAmount *= (1.0 - foamAmount); 
    vec3 specularColor = vec3(1.0, 1.0, 1.0) * specularAmount * 0.6;

    float fresnelFactor = pow(max(1.0 - dot(norm, viewDir), 0.0), 3.0);
    fresnelFactor *= (1.0 - foamAmount); 
    vec3 fresnelColor = vec3(0.5, 0.8, 0.9) * fresnelFactor * 0.6;

    vec3 finalColor = waterWithFoam + specularColor + fresnelColor;
    FragColor = vec4(finalColor, 1.0);
}