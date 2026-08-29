// constants
const vec4 HighlightColor = vec4(0.95f, 0.25f, 0.25f, 0.0f);

// uniforms
uniform sampler2D tex_0;
uniform bool u_highlight;

// passed from vertex shader
in vec2 Texcoord;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	vec4 texelColor = texture(tex_0, Texcoord);

	if (u_highlight)
	{
		texelColor = clamp(texelColor + HighlightColor, 0.0f, 1.0f);
	}

    FinalColor = texelColor; // addtitive
}