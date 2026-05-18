
uniform sampler2D tex_0;

uniform float u_material_opacity;

// passed from vertex shader
in vec2 Texcoord;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	FinalColor = vec4(texture(tex_0, Texcoord).rgb, u_material_opacity); 
}
