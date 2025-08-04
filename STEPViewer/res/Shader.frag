#version 130

in highp vec3 _EyespaceNormal;
in lowp vec3 _Diffuse;
in lowp float _EnableLighting;
in lowp float _EnableTexture;
in mediump vec2 _UV;

uniform vec3 LightPosition = vec3(0.25, 0.25, 1);
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Transparency;
uniform float Shininess = 50;
uniform vec3 AmbientLightWeighting = vec3(0.4, 0.4, 0.4);
uniform vec3 DiffuseLightWeighting = vec3(0.95, 0.95, 0.95);
uniform vec3 SpecularLightWeighting = vec3(0.15, 0.15, 0.15);
uniform sampler2D Sampler;

void main()
{
    // Texture rendering path
    if (_EnableTexture > 0.5) {
        gl_FragColor = texture2D(Sampler, _UV);
        return;
    }

    // Lighting path
    vec3 color = AmbientMaterial;
    if (_EnableLighting > 0.5) {
        vec3 N = normalize(_EyespaceNormal);
        vec3 L = normalize(LightPosition);
        vec3 E = vec3(0.0, 0.0, 1.0);
        vec3 H = normalize(L + E);

        float df = max(dot(N, L), 0.0);
        float sf = pow(max(dot(N, H), 0.0), Shininess);

        color =
            (AmbientMaterial * AmbientLightWeighting) +
            (df * _Diffuse * DiffuseLightWeighting) +
            (sf * SpecularMaterial * SpecularLightWeighting);
    }

    gl_FragColor = vec4(color, Transparency);
}