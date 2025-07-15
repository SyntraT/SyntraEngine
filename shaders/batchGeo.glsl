#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 sideViews[6];

in vec3 fragPos[];
in vec3 normal[];
in vec2 texCoord[];

out vec3 gFragPos;
out vec3 gNormal;
out vec2 gTexCoord;

void main() {
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face;

        for (int i = 0; i < 3; ++i) {
            gFragPos = fragPos[i];
            gNormal = normal[i];
            gTexCoord = texCoord[i];

            gl_Position = sideViews[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}