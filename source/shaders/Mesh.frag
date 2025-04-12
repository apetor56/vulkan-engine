#version 450

#extension GL_GOOGLE_include_directive : require
#include "Structures.glsl"

layout( location = 0 ) in vec3 outColor;
layout( location = 1 ) in vec2 fragTexCoord;

layout( location = 0 ) out vec4 outFragColor;

void main() {
    outFragColor = vec4( outColor, 1.0 );
    // outFragColor = texture( colorTex, fragTexCoord );
}