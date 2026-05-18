// constants
uniform mat4 u_view_proj;

uniform float u_waveTime;
uniform float u_waveWidth;
uniform float u_waveHeight;
uniform float u_waterLine; // base height

// attributes
in vec2 in_texcoord0;
in vec3 in_position0;

// pass to fragment shader
out vec2 Texcoord;

// entry point
void main() 
{
	Texcoord = in_texcoord0;

    // transformed position
    vec4 position = vec4(in_position0, 1.0f);

    // base waterline
    position.y += u_waterLine;

    // lava waves
    if (u_waveHeight > 0.0f)
    {
        position.y = position.y +
            sin(u_waveWidth * position.x + u_waveTime * u_waveHeight) * 
            cos(u_waveWidth * position.z + u_waveTime) * u_waveHeight;
    }
    // water waves
    else 
    {
        position.y = position.y +
            sin(1.0f * position.x + u_waveTime) * 
            cos(1.0f * position.z + u_waveTime) * 0.01f;
    }

    gl_Position = u_view_proj * position;
}