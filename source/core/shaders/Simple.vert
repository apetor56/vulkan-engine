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

layout( set = 0, binding = 0 ) uniform SceneData {
    mat4 model;
    mat4 view;
    mat4 projection;
}
sceneData;

layout( location = 0 ) out vec4 fragColor;

void main() {
    Vertex vertex = pushConstants.vertexBuffer.vertices[ gl_VertexIndex ];
    gl_Position = pushConstants.worldMatrix * sceneData.projection * sceneData.view * sceneData.model * vertex.position;
    fragColor   = vertex.color;
}