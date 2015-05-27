#version 130

uniform mat4 mvpMatrix; // not used

in vec4 vertex;

void main(void)
{
    // gl_Position = mvpMatrix * vertex;
    gl_Position = vertex;
}
