struct VertexInput {
    @location(0) vertex_position: vec3<f32>,
    @location(1) vertex_normal: vec3<f32>,
    @location(2) texture_coord: vec2<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) use_blinn_phong_model: f32,
    @location(1) world_position: vec4<f32>,
    @location(2) transformed_normal: vec3<f32>,
    @location(3) use_texture: f32,
    @location(4) texture_coord: vec2<f32>,
}

struct VertexUniforms {
    use_blinn_phong_model: f32,
    use_texture: f32,
    mv_matrix: mat4x4<f32>,
    p_matrix: mat4x4<f32>,
    n_matrix: mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> uniforms: VertexUniforms;

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    
    let vertex = uniforms.mv_matrix * vec4<f32>(input.vertex_position, 1.0);
    output.world_position = vertex;
    output.position = uniforms.p_matrix * vertex;
    
    if (uniforms.use_texture > 0.5) {
        output.texture_coord = input.texture_coord;
    } else {
        if (uniforms.use_blinn_phong_model > 0.5) {
            output.transformed_normal = (uniforms.n_matrix * vec4<f32>(input.vertex_normal, 1.0)).xyz;
        }
    }
    
    output.use_blinn_phong_model = uniforms.use_blinn_phong_model;
    output.use_texture = uniforms.use_texture;
    
    return output;
}