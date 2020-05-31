
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec2 in_texcoord;
in vec3 in_pos;

// pass to fragment shader
out vec2 Texcoord;

// entry point
void main() 
{
	Texcoord = in_texcoord;

    // transformed position
    vec4 position = vec4(in_pos, 1.0f);
    gl_Position = view_projection_matrix * position;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform sampler2D tex_0;

// passed from vertex shader
in vec2 Texcoord;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	vec4 texelColor = texture(tex_0, Texcoord);
    FinalColor = texelColor;
}

#endif
