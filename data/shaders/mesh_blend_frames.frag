uniform sampler2D tex_0;

// passed from vertex shader
in vec2 Texcoord;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	vec4 texelColor = texture(tex_0, Texcoord);
    FinalColor = texelColor; // addtitive
}