
// constants
uniform mat4 u_view_proj;
uniform mat4 u_model_matrix;
uniform mat3 u_norm_matrix;

uniform float u_mixFrames;

// attributes
in vec3 in_position0;
in vec3 in_position1;
in vec2 in_texcoord0;

// pass to fragment shader
out vec2 Texcoord;
out vec4 FragColor;

// entry point
void main() 
{
	Texcoord = in_texcoord0;

    vec4 v0 = u_view_proj * u_model_matrix * vec4(in_position0, 1.0f);
    vec4 v1 = u_view_proj * u_model_matrix * vec4(in_position1, 1.0f);

    gl_Position = mix(v0, v1, u_mixFrames);
}