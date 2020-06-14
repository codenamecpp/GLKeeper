
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;
uniform mat4 model_matrix;

uniform float mix_frames;

// attributes
in vec3 in_pos_frame0;
in vec3 in_pos_frame1;
in vec2 in_texcoord;

// pass to fragment shader
out vec2 Texcoord;
out vec4 FragColor;

// entry point
void main() 
{
	Texcoord = in_texcoord;

    vec4 v0 = view_projection_matrix * model_matrix * vec4(in_pos_frame0, 1.0);
    vec4 v1 = view_projection_matrix * model_matrix * vec4(in_pos_frame1, 1.0);

    gl_Position = mix(v0, v1, mix_frames);
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

