#define TERRAIN_COLOR_TEXTURE tex_1

// constants
uniform mat4 u_view_proj;
uniform mat4 u_model_matrix;
uniform mat3 u_norm_matrix;
uniform sampler2D TERRAIN_COLOR_TEXTURE;

// attributes
in vec3 in_position0;
in vec3 in_normal0;
in vec2 in_texcoord0;
in ivec2 in_tile_coord;

// pass to fragment shader
out vec2 Texcoord;
out vec3 FragIllumination;
out vec4 FragColor;
out vec3 InPos;

struct Light {
    vec3 position;
    vec3 color;
    float radius;
};

uniform Light u_Light;

vec3 DirectIllumination(vec3 P, vec3 N, vec3 lightCentre, float lightRadius, vec3 lightColour, float cutoff)
{
    // calculate normalized light vector and distance to sphere light surface
    float r = lightRadius;
    vec3 L = lightCentre - P;
    float distance = length(L);
    float d = max(distance - r, 0);
    L /= distance;
     
    // calculate basic attenuation
    float denom = d/r + 1;
    float attenuation = 1 / (denom*denom);
     
    // scale and bias attenuation such that:
    //   attenuation == 0 at extent of max influence
    //   attenuation == 1 when d == 0
    attenuation = (attenuation - cutoff) / (1 - cutoff);
    attenuation = max(attenuation, 0);
     
    float dot = max(dot(L, N), 0);
    return lightColour * dot * attenuation;
}

vec3 NaiveIllumination(vec3 P, vec3 N, vec3 lightCenter, float lightRaius, vec3 lightColor)
{
    float distanceToLightCenter = length(lightCenter - P);
    float maxLightRadius = lightRaius * 0.55f;

    if (distanceToLightCenter < lightRaius)
    {
        if (distanceToLightCenter < maxLightRadius)
        {
            return lightColor;
        }

        return lightColor * (1.0f - ((distanceToLightCenter - maxLightRadius) / (lightRaius - maxLightRadius)));
    }

    return vec3(0.0f, 0.0f, 0.0f);
}

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    InPos = in_position0;

    //FragIllumination = DirectIllumination(in_position0, in_normal0, 
    //    u_Light.position, 
    //    u_Light.radius, 
    //    u_Light.color, 0.92f);
    
    FragColor = texelFetch(TERRAIN_COLOR_TEXTURE, in_tile_coord, 0);

    FragIllumination = vec3(1.0f, 1.0f, 1.0f);

    //FragIllumination = NaiveIllumination(in_position0, in_normal0, 
    //    u_Light.position, 
    //    u_Light.radius, 
    //    u_Light.color);

    vec4 vertexPosition = u_view_proj * u_model_matrix * vec4(in_position0, 1.0f);
    gl_Position = vertexPosition;
}