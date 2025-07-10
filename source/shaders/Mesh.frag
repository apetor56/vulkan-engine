#version 450

#extension GL_GOOGLE_include_directive : require
#include "Structures.glsl"

layout( location = 0 ) in vec3 inColor;
layout( location = 1 ) in vec3 inWorldNormal;
layout( location = 2 ) in vec2 inTexCoord;

layout( location = 0 ) out vec4 outFragColor;

void main() {
    vec3 unitDirectionToLight = normalize( sceneData.directionToLight.xyz );
    vec3 objectColor          = inColor * texture( colorTex, inTexCoord ).xyz;

    float ambientStrenght = 0.1f;
    vec3 ambient          = ambientStrenght * sceneData.lightColor.xyz;

    float lightIntensity = max( dot( inWorldNormal, unitDirectionToLight ), 0.0f );
    vec3 diffuse         = lightIntensity * sceneData.lightColor.xyz;

    outFragColor = vec4( ( ambient + diffuse ) * objectColor, 1.0f );
}