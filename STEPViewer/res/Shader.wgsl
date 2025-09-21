struct VertexInput {
    @location(0) position: vec4<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) eyespace_normal: vec3<f32>,
    @location(1) diffuse: vec3<f32>,
    @location(2) enable_lighting: f32,
    @location(3) enable_texture: f32,
    @location(4) uv: vec2<f32>,
}

struct Uniforms {
    projection_matrix: mat4x4<f32>,
    model_view_matrix: mat4x4<f32>,
    normal_matrix: mat3x3<f32>,
    diffuse_material: vec3<f32>,
    enable_lighting: f32,
    enable_texture: f32,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    
    output.eyespace_normal = uniforms.normal_matrix * input.normal;
    output.diffuse = uniforms.diffuse_material;
    output.enable_lighting = uniforms.enable_lighting;
    output.enable_texture = uniforms.enable_texture;
    output.uv = input.uv;
    
    output.position = uniforms.projection_matrix * uniforms.model_view_matrix * input.position;
    
    return output;
}