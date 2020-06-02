
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

#define colorsTex tex_1

// constants
uniform mat4 view_projection_matrix;
uniform mat4 model_matrix;
uniform sampler2D colorsTex;

// attributes
in vec3 in_pos;
//in vec3 in_normal;
in vec2 in_texcoord;
in ivec2 in_tile_coord;

// pass to fragment shader
out vec2 Texcoord;
out vec3 FragIllumination;
out vec4 FragColor;
out vec3 InPos;

// entry point
void main() 
{
	Texcoord = in_texcoord;
    InPos = in_pos;

	FragColor = texelFetch(colorsTex, in_tile_coord, 0);

    vec4 vertexPosition = view_projection_matrix * model_matrix * vec4(in_pos, 1.0f);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

#define diffuseTex tex_0

uniform sampler2D diffuseTex;

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
	vec4 texelColor = texture(diffuseTex, Texcoord);
    if (texelColor.a < 0.185f)
        discard; // translucency

    // temporary color correction 
    // >>
    FinalColor = texelColor * (smoothstep(-2.0f, 2.0f, InPos.y));
    FinalColor += FragColor; // addtitive
    // <<

    //FinalColor = texelColor + FragColor; // addtitive
}

#endif
