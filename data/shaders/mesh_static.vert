// constants
uniform mat4 u_view_proj;
uniform mat4 u_model_matrix;
uniform mat3 u_norm_matrix;

// attributes
in vec3 in_position0;
in vec3 in_normal0;
in vec2 in_texcoord0;
in vec4 in_color0;

// pass to fragment shader
out vec2 Texcoord;
out vec3 FragColor;
out vec3 InPos;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    InPos = in_position0;
	FragColor = in_color0.rgb;

    vec4 vertexPosition = u_view_proj * u_model_matrix * vec4(in_position0, 1.0f);
    gl_Position = vertexPosition;
}
