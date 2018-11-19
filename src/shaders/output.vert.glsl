#version 430

precision highp float;

out vec2 ex_TexCoord;

const vec4 vertices[6] = vec4[6](vec4(1.0, -1.0, 0.5, 1.0),
                                 vec4(-1.0, -1.0, 0.5, 1.0),
                                 vec4(1.0, 1.0, 0.5, 1.0),
                                 vec4(1.0, 1.0, 0.5, 1.0),
                                 vec4(-1.0, 1.0, 0.5, 1.0),
                                 vec4(-1.0, -1.0, 0.5, 1.0));

void main(void)
{
    ex_TexCoord = (vertices[gl_VertexID].xy + 1.0) * 0.5;
    gl_Position = vertices[gl_VertexID];
}



