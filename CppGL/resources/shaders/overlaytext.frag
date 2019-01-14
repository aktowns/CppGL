#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoord).r);
    vec4 texColour = vec4(textColor, 1.0) * sampled;

    if(texColour.a < 0.1)
        discard;

    FragColor = texColour;
}

