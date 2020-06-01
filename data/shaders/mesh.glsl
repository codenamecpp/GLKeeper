
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
    vec4 position = vec4(in_pos, 1.0f);
    gl_Position = view_projection_matrix * position;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform sampler2D tex_0;
uniform int material_color_mode;

// passed from vertex shader
in vec2 Texcoord;
in vec4 FragColor;

// result
out vec4 FinalColor;

vec4 fetch_color(vec2 uv)
{
	vec4 texelColor = texture(tex_0, uv);
	if (material_color_mode == 0) // default
	{
		return texelColor * FragColor;
	}
	if (material_color_mode == 1) // mix
	{
		return clamp(texelColor + FragColor, 0.0f, 1.0f);
	}
	if (material_color_mode == 2) // vertex
	{
		return FragColor;
	}

	return texelColor;
}

// entry point
void main() 
{
	vec4 texelColor = fetch_color(Texcoord);
    FinalColor = texelColor;
}

#endif
