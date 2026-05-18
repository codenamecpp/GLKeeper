
// constants
uniform mat4 u_view_proj;

// attributes
in vec3 in_position0;
in vec4 in_color0;
in vec2 in_texcoord0;

// pass to fragment shader
out vec2 Texcoord;
out vec4 Color;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    Color = in_color0;

    // transformed position
    gl_Position = u_view_proj * vec4(in_position0, 1.0f);
}
