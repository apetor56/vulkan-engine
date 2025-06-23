#version 450

#extension GL_GOOGLE_include_directive : require
#include "Structures.glsl"

layout( location = 0 ) in vec3 inColor;
layout( location = 1 ) in vec3 inWorldNormal;
layout( location = 2 ) in vec2 inTexCoord;

layout( location = 0 ) out vec4 outFragColor;

void main() {
    float lightIntensity  = max( dot( inWorldNormal, normalize( sceneData.directionToLight.xyz ) ), 0.0f );
    float ambientStrenght = 0.1;
    vec3 ambient          = ambientStrenght * sceneData.lightColor.xyz;
    vec3 diffuse          = lightIntensity * sceneData.lightColor.xyz;
    vec3 color            = inColor * texture( colorTex, inTexCoord ).xyz;

    outFragColor = vec4( ( ambient + diffuse ) * color, 1.0f );
}