
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec3 in_pos;
in vec4 in_color;

// pass to fragment shader
out vec4 FragColor;

// entry point
void main() 
{
	FragColor = in_color;

    gl_Position = view_projection_matrix * vec4(in_pos, 1.0f);
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

// passed from vertex shader
in vec4 FragColor;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	FinalColor = FragColor;
}

#endif
