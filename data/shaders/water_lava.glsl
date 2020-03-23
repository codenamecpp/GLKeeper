
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;
uniform vec4 wave_params; // x - wave_time, y - wave_width, z - wave_height, w - water_line

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

    // base waterline
    position.y += wave_params.w;

    // lava waves
    if (wave_params.z > 0.0f)
    {
        position.y = position.y +
            sin(wave_params.y * position.x + wave_params.x * wave_params.z) * 
            cos(wave_params.y * position.z + wave_params.x) * wave_params.z;
    }
    // water waves
    else 
    {
        position.y = position.y +
            sin(1.0f * position.x + wave_params.x) * 
            cos(1.0f * position.z + wave_params.x) * 0.01f;
    }

    gl_Position = view_projection_matrix * position;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform sampler2D tex_0;
uniform float translucency;

// passed from vertex shader
in vec2 Texcoord;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	FinalColor = vec4(texture(tex_0, Texcoord).rgb, translucency); 
}

#endif