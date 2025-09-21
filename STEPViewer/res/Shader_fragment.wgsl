struct FragmentInput {
    @location(0) eyespace_normal: vec3<f32>,
    @location(1) diffuse: vec3<f32>,
    @location(2) enable_lighting: f32,
    @location(3) enable_texture: f32,
    @location(4) uv: vec2<f32>,
}

struct FragmentUniforms {
    light_position: vec3<f32>,
    ambient_material: vec3<f32>,
    specular_material: vec3<f32>,
    transparency: f32,
    shininess: f32,
    ambient_light_weighting: vec3<f32>,
    diffuse_light_weighting: vec3<f32>,
    specular_light_weighting: vec3<f32>,
}

@group(0) @binding(1) var<uniform> fragment_uniforms: FragmentUniforms;
@group(0) @binding(2) var texture_sampler: sampler;
@group(0) @binding(3) var diffuse_texture: texture_2d<f32>;

@fragment
fn fs_main(input: FragmentInput) -> @location(0) vec4<f32> {
    // Texture rendering path
    if (input.enable_texture > 0.5) {
        return textureSample(diffuse_texture, texture_sampler, input.uv);
    }

    // Lighting path
    var color = fragment_uniforms.ambient_material;
    
    if (input.enable_lighting > 0.5) {
        let N = normalize(input.eyespace_normal);
        let L = normalize(fragment_uniforms.light_position);
        let E = vec3<f32>(0.0, 0.0, 1.0);
        let H = normalize(L + E);

        let df = max(dot(N, L), 0.0);
        let sf = pow(max(dot(N, H), 0.0), fragment_uniforms.shininess);

        color = (fragment_uniforms.ambient_material * fragment_uniforms.ambient_light_weighting) +
                (df * input.diffuse * fragment_uniforms.diffuse_light_weighting) +
                (sf * fragment_uniforms.specular_material * fragment_uniforms.specular_light_weighting);
    }

    return vec4<f32>(color, fragment_uniforms.transparency);
}