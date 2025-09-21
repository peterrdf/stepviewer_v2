struct FragmentInput {
    @location(0) use_blinn_phong_model: f32,
    @location(1) world_position: vec4<f32>,
    @location(2) transformed_normal: vec3<f32>,
    @location(3) use_texture: f32,
    @location(4) texture_coord: vec2<f32>,
}

struct FragmentUniforms {
    point_lighting_location: vec3<f32>,
    ambient_light_weighting: vec3<f32>,
    specular_light_weighting: vec3<f32>,
    diffuse_light_weighting: vec3<f32>,
    material_shininess: f32,
    contrast: f32,
    brightness: f32,
    gamma: f32,
    material_ambient_color: vec3<f32>,
    transparency: f32,
    material_diffuse_color: vec3<f32>,
    material_specular_color: vec3<f32>,
    material_emissive_color: vec3<f32>,
}

@group(0) @binding(1) var<uniform> fragment_uniforms: FragmentUniforms;
@group(0) @binding(2) var texture_sampler: sampler;
@group(0) @binding(3) var diffuse_texture: texture_2d<f32>;

@fragment
fn fs_main(input: FragmentInput) -> @location(0) vec4<f32> {
    if (input.use_texture > 0.5) {
        return textureSample(diffuse_texture, texture_sampler, input.texture_coord);
    } else {
        if (input.use_blinn_phong_model > 0.5) {
            let light_direction = normalize(fragment_uniforms.point_lighting_location - input.world_position.xyz);
            let normal = normalize(input.transformed_normal);

            let eye_direction = normalize(-input.world_position.xyz);
            let reflection_direction = reflect(-light_direction, normal);

            let specular_light_brightness = pow(max(dot(reflection_direction, eye_direction), 0.0), fragment_uniforms.material_shininess);
            let specular_light_weighting = fragment_uniforms.specular_light_weighting * specular_light_brightness;

            let diffuse_light_brightness = max(dot(normal, light_direction), 0.0);
            let diffuse_light_weighting = fragment_uniforms.diffuse_light_weighting * diffuse_light_brightness;

            let color = vec4<f32>(
                fragment_uniforms.material_ambient_color * fragment_uniforms.ambient_light_weighting +
                fragment_uniforms.material_diffuse_color * diffuse_light_weighting +
                fragment_uniforms.material_specular_color * specular_light_weighting +
                fragment_uniforms.material_emissive_color,
                fragment_uniforms.transparency
            );

            var new_color = vec4<f32>(0.0, 0.0, 0.0, fragment_uniforms.transparency);
            new_color.r = (pow(color.r, fragment_uniforms.gamma) - 0.5) * fragment_uniforms.contrast + fragment_uniforms.brightness + 0.5;
            new_color.g = (pow(color.g, fragment_uniforms.gamma) - 0.5) * fragment_uniforms.contrast + fragment_uniforms.brightness + 0.5;
            new_color.b = (pow(color.b, fragment_uniforms.gamma) - 0.5) * fragment_uniforms.contrast + fragment_uniforms.brightness + 0.5;

            return new_color;
        } else {
            return vec4<f32>(fragment_uniforms.material_ambient_color, fragment_uniforms.transparency);
        }
    }
}