#version 330 core
layout (location = 0) in vec4 vertex; 

out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform vec3 position;

void main()
{
    // Screen space
    //gl_Position = projection * view * vec4(position, 1.0f);
    //gl_Position /= gl_Position.w;
    //gl_Position.xy += vertex.xy * vec2(0.05, 0.05);
    vec3 ws = position
		        + cameraRight * vertex.x * 0.1
		        + cameraUp    * vertex.y * 0.1;
    ws.y += 1.2;
    gl_Position = projection * view * vec4(ws, 1.0f);

    TexCoord = vertex.zw;
}

