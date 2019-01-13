#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

struct SamplerActive {
    bool diffuse;
    bool specular;
    bool normal;
    bool height;
};

uniform SamplerActive samplerActive;

void main()
{    
    if(samplerActive.diffuse) {
        FragColor = texture(texture_diffuse1, TexCoords);
    } else if (samplerActive.specular) {
        FragColor = texture(texture_specular1, TexCoords);
    } else if (samplerActive.normal) {
        FragColor = texture(texture_normal1, TexCoords);
    } else if (samplerActive.height) {
        FragColor = texture(texture_height1, TexCoords);
    } else {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
}

