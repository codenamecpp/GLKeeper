
uniform sampler2D tex_0;

uniform float u_material_opacity;
uniform vec3 u_material_base_color;

// passed from vertex shader
in vec2 Texcoord;
in vec3 InPos;
in vec3 FragColor;

// result
out vec4 FinalColor;

// entry point
void main() 
{
	vec3 texelColor = texture(tex_0, Texcoord).rgb;
	texelColor *= FragColor;
	texelColor *= u_material_base_color;
    FinalColor = vec4(texelColor, u_material_opacity);
}
