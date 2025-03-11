#version 450
#extension GL_EXT_buffer_reference : require

struct Vertex {
    vec4 position;
    vec4 color;
    vec2 texCoord;
    vec2 uv;
};

layout( buffer_reference, std430 ) readonly buffer VertexBuffer {
    Vertex[] vertices;
};

layout( push_constant ) uniform Constants {
    mat4 worldMatrix;
    VertexBuffer vertexBuffer;
}
pushConstants;

layout( binding = 0 ) uniform uniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
}
ubo;

layout( location = 0 ) out vec4 fragColor;
layout( location = 1 ) out vec2 fragTexCoord;

void main() {
    Vertex vertex = pushConstants.vertexBuffer.vertices[ gl_VertexIndex ];
    gl_Position   = pushConstants.worldMatrix * ubo.projection * ubo.view * ubo.model * vertex.position;
    fragColor     = vertex.color;
    fragTexCoord  = vertex.texCoord;
}