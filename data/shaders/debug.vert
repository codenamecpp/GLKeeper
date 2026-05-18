
// constants
uniform mat4 u_view_proj;

// attributes
in vec3 in_position0;
in vec4 in_color0;

// pass to fragment shader
out vec4 Color;

// entry point
void main() 
{
	Color = in_color0;

    gl_Position = u_view_proj * vec4(in_position0, 1.0f);
}