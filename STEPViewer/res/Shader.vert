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

out highp vec3 _EyespaceNormal;
out lowp vec3 _Diffuse;
out lowp float _EnableLighting;
out lowp float _EnableTexture;
out mediump vec2 _UV;

void main()
{
    _EyespaceNormal = NormalMatrix * Normal;
    _Diffuse = DiffuseMaterial;

    _EnableLighting = EnableLighting;
    _EnableTexture = EnableTexture;
    _UV = UV;

    gl_Position = ProjectionMatrix * ModelViewMatrix * Position;
    gl_PointSize = 5.0;
}