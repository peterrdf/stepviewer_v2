#version 130

in highp vec3 EyespaceNormal;
in lowp vec3 Diffuse;
in lowp float _EnableLighting;
in lowp float _EnableTexture;
in mediump vec2 _UV;
in vec4 FragPosLightSpace;

uniform vec3 LightPosition = vec3(10.0, 20.0, 10.0);
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Transparency;
uniform float Shininess = 50;
uniform vec3 AmbientLightWeighting = vec3(0.4, 0.4, 0.4);
uniform vec3 DiffuseLightWeighting = vec3(0.95, 0.95, 0.95);
uniform vec3 SpecularLightWeighting = vec3(0.15, 0.15, 0.15);
uniform sampler2D Sampler;
uniform sampler2D shadowMap;
uniform vec2 shadowMapSize; // Size of the shadow map texture
uniform vec3 LightDirection;
uniform float IsShadowPass; // 1.0 for shadow pass, 0.0 for normal rendering

out vec4 FragColor;

// Function to calculate shadow using PCF (Percentage Closer Filtering)
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Add bias to prevent shadow acne
    float bias = max(0.05 * (1.0 - dot(normalize(EyespaceNormal), normalize(LightDirection))), 0.005);
    
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / shadowMapSize;
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
    if (IsShadowPass > 0.5) {
        // Only depth is written, no color output needed
        return;
    }

    if (_EnableTexture > 0.5) {
        FragColor = texture2D(Sampler, _UV);
        return;
    }

    vec3 color = AmbientMaterial;
    if (_EnableLighting > 0.5) {
        vec3 N = normalize(EyespaceNormal);
        vec3 L = normalize(LightPosition);
        vec3 E = vec3(0.0, 0.0, 1.0);
        vec3 H = normalize(L + E);

        float df = max(dot(N, L), 0.0);
        float sf = pow(max(dot(N, H), 0.0), Shininess);

        color =
            (AmbientMaterial * AmbientLightWeighting) +
            (df * Diffuse * DiffuseLightWeighting) +
            (sf * SpecularMaterial * SpecularLightWeighting);
    }

    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 lighting = (AmbientMaterial + (1.0 - shadow) * (DiffuseLightWeighting + SpecularLightWeighting)) * color;
    FragColor = vec4(lighting, Transparency);
}