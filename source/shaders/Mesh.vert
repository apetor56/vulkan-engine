#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "Structures.glsl"

layout( location = 0 ) out vec3 outColor;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec2 outTexCoord;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
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

    gl_Position = sceneData.projection * sceneData.view * sceneData.model * PushConstants.renderMartix *
                  vec4( vertex.position, 1.0 );

    outColor    = materialData.colorFactors.xyz;
    outNormal   = ( PushConstants.renderMartix * vec4( vertex.normal, 0.0 ) ).xyz;
    outTexCoord = vec2( vertex.uv_x, vertex.uv_y );
}
