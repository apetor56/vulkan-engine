#version 430 core

#extension GL_GOOGLE_include_directive : require

#include "Structures.glsl"

const vec3 cubeVertices[] = vec3[](
    // Front face
    vec3( -0.5, -0.5, 0.5 ), vec3( 0.5, -0.5, 0.5 ), vec3( 0.5, 0.5, 0.5 ), vec3( 0.5, 0.5, 0.5 ),
    vec3( -0.5, 0.5, 0.5 ), vec3( -0.5, -0.5, 0.5 ),

    // Back face
    vec3( -0.5, -0.5, -0.5 ), vec3( -0.5, 0.5, -0.5 ), vec3( 0.5, 0.5, -0.5 ), vec3( 0.5, 0.5, -0.5 ),
    vec3( 0.5, -0.5, -0.5 ), vec3( -0.5, -0.5, -0.5 ),

    // Left face
    vec3( -0.5, 0.5, 0.5 ), vec3( -0.5, 0.5, -0.5 ), vec3( -0.5, -0.5, -0.5 ), vec3( -0.5, -0.5, -0.5 ),
    vec3( -0.5, -0.5, 0.5 ), vec3( -0.5, 0.5, 0.5 ),

    // Right face
    vec3( 0.5, 0.5, 0.5 ), vec3( 0.5, -0.5, 0.5 ), vec3( 0.5, -0.5, -0.5 ), vec3( 0.5, -0.5, -0.5 ),
    vec3( 0.5, 0.5, -0.5 ), vec3( 0.5, 0.5, 0.5 ),

    // Bottom face
    vec3( -0.5, -0.5, -0.5 ), vec3( 0.5, -0.5, -0.5 ), vec3( 0.5, -0.5, 0.5 ), vec3( 0.5, -0.5, 0.5 ),
    vec3( -0.5, -0.5, 0.5 ), vec3( -0.5, -0.5, -0.5 ),

    // Top face
    vec3( -0.5, 0.5, -0.5 ), vec3( -0.5, 0.5, 0.5 ), vec3( 0.5, 0.5, 0.5 ), vec3( 0.5, 0.5, 0.5 ),
    vec3( 0.5, 0.5, -0.5 ), vec3( -0.5, 0.5, -0.5 ) );

layout( location = 0 ) out vec3 outTexCoords;

void main() {
    outTexCoords  = cubeVertices[ gl_VertexIndex ];
    vec4 position = sceneData.projection * mat4( mat3( sceneData.view ) ) * vec4( cubeVertices[ gl_VertexIndex ], 1.0 );
    gl_Position   = position.xyww;
}