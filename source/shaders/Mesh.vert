#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "Structures.glsl"

layout( location = 0 ) out vec3 outColor;
layout( location = 1 ) out vec2 fragTexCoord;

struct Vertex {
    vec4 position;
    vec4 color;
    vec2 texCoord;
    vec2 uv;
};

layout( buffer_reference, std430 ) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout( push_constant ) uniform constants {
    mat4 renderMartix;
    VertexBuffer vertexBuffer;
}
PushConstants;

void main() {
    Vertex vertex = PushConstants.vertexBuffer.vertices[ gl_VertexIndex ];

    gl_Position =
        sceneData.projection * sceneData.view * sceneData.model * PushConstants.renderMartix * vertex.position;

    outColor     = materialData.colorFactors.xyz;
    fragTexCoord = vertex.texCoord;
}
