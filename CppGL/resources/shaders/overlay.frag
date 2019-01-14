#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
    vec4 texColour = texture(texture1, TexCoord);

    if(texColour.a < 0.1)
        discard;

    FragColor = texColour;
}

