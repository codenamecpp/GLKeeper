
#define DIFFUSE_TEXTURE tex_0

uniform sampler2D DIFFUSE_TEXTURE;

// passed from vertex shader
in vec2 Texcoord;
in vec4 FragColor;
in vec3 FragIllumination;
in vec3 InPos;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	vec4 texelColor = texture(DIFFUSE_TEXTURE, Texcoord);
    if (texelColor.a < 0.185f)
        discard; // translucency

    // temporary color correction 
    // >>
    FinalColor = texelColor * (smoothstep(-2.0f, 2.0f, InPos.y));
    FinalColor += FragColor; // addtitive
    // <<

    //FinalColor = texelColor + FragColor; // addtitive
}