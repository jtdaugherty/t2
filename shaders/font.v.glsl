#version 110

attribute vec4 vertex;
uniform int width;
uniform int height;
varying vec2 TexCoords;

void main()
{
    mat4 projection = mat4(
        vec4(2.0 / float(width), 0, 0, 0),
        vec4(0, 2.0 / float(height), 0, 0),
        vec4(0, 0, -1, 0),
        vec4(-1, -1, 0, 1)
        );

    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
