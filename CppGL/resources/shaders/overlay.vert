#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform vec3 position;

void main()
{
    vec3 ws = position + cameraRight * aPos.x * 0.125f 
                       + cameraUp    * aPos.y * 0.125f;

    gl_Position = projection * view * vec4(ws, 1.0f);                   
    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}

