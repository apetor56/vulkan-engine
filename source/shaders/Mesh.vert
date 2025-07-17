#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "Structures.glsl"

layout( location = 0 ) out vec3 outWorldPos;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec2 outTexCoords;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
    vec4 tangent;
};

layout( buffer_reference, std430 ) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout( push_constant ) uniform constants {
    mat4 renderMartix;
    VertexBuffer vertexBuffer;
}
pushConstants;

void main() {
    Vertex vertex = pushConstants.vertexBuffer.vertices[ gl_VertexIndex ];

    outWorldPos  = mat3( sceneData.model * pushConstants.renderMartix ) * vertex.position;
    
    //only for uniform scaling
    outNormal    = mat3( sceneData.model * pushConstants.renderMartix ) * vertex.normal;
    outTexCoords = vec2( vertex.uv_x, vertex.uv_y );

    gl_Position = sceneData.projection * sceneData.view * sceneData.model * pushConstants.renderMartix *
                  vec4( vertex.position, 1.0f );
}
