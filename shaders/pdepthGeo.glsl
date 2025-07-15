#version 330 core

#define NR_POINT_LIGHTS ${NR_POINT_LIGHTS=1}

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 shadowTransforms[6 * NR_POINT_LIGHTS];
uniform int lightIndex;

out vec4 FragPos;

void main() {
    int baseLayer = lightIndex * 6;
    for (int face = 0; face < 6; ++face) {
        gl_Layer = baseLayer + face;
        for (int i = 0; i < 3; ++i) {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowTransforms[baseLayer + face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
