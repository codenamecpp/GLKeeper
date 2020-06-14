
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec2 in_texcoord;
in vec3 in_pos;
in vec4 in_color;

// pass to fragment shader
out vec2 Texcoord;
out vec4 FragColor;

// entry point
void main() 
{
	Texcoord = in_texcoord;
    FragColor = in_color;

    // transformed position
    vec4 position = vec4(in_pos, 1.0);
    gl_Position = view_projection_matrix * position;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform sampler2D tex_0;
uniform vec4 material_color;

// passed from vertex shader
in vec2 Texcoord;
in vec4 FragColor;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	vec4 texelColor = texture(tex_0, Texcoord);
	texelColor *= FragColor;
	texelColor *= material_color;
    FinalColor = texelColor;
}

#endif
