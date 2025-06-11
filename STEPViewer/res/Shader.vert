#version 130

in vec4 Position;
in vec3 Normal;
in vec2 UV;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform vec3 DiffuseMaterial;
uniform float EnableLighting = 1;
uniform float EnableTexture = 0;
uniform mat4 LightSpaceMatrix;

out highp vec3 EyespaceNormal;
out lowp vec3 Diffuse;
out lowp float _EnableLighting;
out lowp float _EnableTexture;
out mediump vec2 _UV;
out vec4 FragPosLightSpace;

void main()
{
    EyespaceNormal = NormalMatrix * Normal;
    Diffuse = DiffuseMaterial;

    _EnableLighting = EnableLighting;
    _EnableTexture = EnableTexture;
    _UV = UV;

    gl_Position = ProjectionMatrix * ModelViewMatrix * Position;
    gl_PointSize = 5.0;

    FragPosLightSpace = LightSpaceMatrix * vec4(Position.xyz, 1.0);
}