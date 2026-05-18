
uniform sampler2D tex_0;
uniform bool u_TextureA8;

// passed from vertex shader
in vec2 Texcoord;
in vec4 Color;

// result
out vec4 FinalColor;

// entry point
void main() 
{
    vec4 textureColor = texture(tex_0, Texcoord);
    if (u_TextureA8)
    {
        FinalColor = vec4(1.0f, 1.0f, 1.0f, textureColor.r) * Color; 
    }
	else
    {
        FinalColor = vec4(1.0, 1.0, 1.0, 1.0) * textureColor * Color; 
    }
}
