#version 330 core
layout (location = 0) out vec3 gNormal;
layout (location = 1) out vec4 diffuse;

in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D  texture_diffuse;

void main()
{    
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    diffuse = texture(texture_diffuse,TexCoords);
} 