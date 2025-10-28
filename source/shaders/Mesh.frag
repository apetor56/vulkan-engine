#version 450

#extension GL_GOOGLE_include_directive : require
#include "Structures.glsl"

layout( location = 0 ) in vec3 inWorldPos;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec2 inTexCoords;

layout( location = 0 ) out vec4 outFragColor;

const float PI = 3.14159265359;

float distributionGGX( float normalHalfwayDotMax, float roughness ) {
    float alphaFactor = pow( roughness, 4.0f );
    float denominator = pow( normalHalfwayDotMax, 2.0f ) * ( alphaFactor - 1.0f ) + 1.0f;
    denominator       = PI * denominator * denominator;

    return alphaFactor / denominator;
}

float geometrySchlickGGX( float normalViewDot, float roughness ) {
    float k           = pow( roughness + 1, 2.0f ) / 8.0f;
    float denominator = normalViewDot * ( 1.0 - k ) + k;

    return normalViewDot / denominator;
}

float geometrySmith( float normalViewDotMax, float normalLightDotMax, float roughness ) {
    return geometrySchlickGGX( normalViewDotMax, roughness ) * geometrySchlickGGX( normalLightDotMax, roughness );
}

vec3 fresnelSchlick( float halfwayViewDot, vec3 baseReflectivity ) {
    return baseReflectivity + ( 1.0 - baseReflectivity ) * pow( 1.0 - halfwayViewDot, 5.0 );
}

vec3 lightPositions[ 4 ] = vec3[]( vec3( -14.756604f, 3.4603605f, -5.237836f ), vec3( -15.001932f, 3.1396596f, 3.4824698f ),
                                   vec3( 11.690615f, 3.6053026f, 3.3117452f ), vec3( 11.677476f, 3.4518013f, -5.332671f ) );
vec3 lightColors[ 4 ]    = vec3[]( vec3( 5.0f, 3.0f, 7.0f ), vec3( 3.0f, 5.0f, 3.0f ),
                                vec3( 3.0f, 9.0f, 4.0f ), vec3( 13.0f, 5.0f, 5.0f ) );

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture( normalMap, inTexCoords ).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx( inWorldPos );
    vec3 Q2  = dFdy( inWorldPos );
    vec2 st1 = dFdx( inTexCoords );
    vec2 st2 = dFdy( inTexCoords );

    vec3 N   = normalize( inNormal );
    vec3 T   = normalize( Q1 * st2.t - Q2 * st1.t );
    vec3 B   = -normalize( cross( N, T ) );
    mat3 TBN = mat3( T, B, N );

    return normalize( TBN * tangentNormal );
}

void main() {
    float metallic  = texture( metallicRoughnessMap, inTexCoords ).b * materialData.metallicRoughnessFactors.x;
    float roughness = texture( metallicRoughnessMap, inTexCoords ).g * materialData.metallicRoughnessFactors.y;
    vec3 albedo     = texture( albedoMap, inTexCoords ).rgb;

    //vec3 normal        = getNormalFromMap();
    vec3 normal        = normalize( inNormal );
    vec3 viewDirection = normalize( sceneData.cameraPosition - inWorldPos );
    float normalViewDotMax    = max( dot( normal, viewDirection ), 0.0 );

    vec3 baseReflectivity = vec3( 0.04 );
    baseReflectivity      = mix( baseReflectivity, albedo, metallic );

    vec3 outRadiance = vec3( 0.0 );

    for ( int i = 0; i < 4; ++i ) {
        vec3 lightDir = normalize( lightPositions[ i ] - inWorldPos );
        vec3 halfway  = normalize( viewDirection + lightDir );

        float normalLightDotMax   = max( dot( normal, lightDir ), 0.0 );
        float normalHalfwayDotMax = max( dot( normal, halfway ), 0.0 );
        float halfwayViewDot      = dot( halfway, viewDirection );

        float dist        = length( lightPositions[ i ] - inWorldPos );
        float attenuation = 1.0 / pow( dist, 2.0 );
        vec3 radiance     = lightColors[ i ] * 6.0f * attenuation;

        float normalDistribution = distributionGGX( normalHalfwayDotMax, roughness );
        float geometry           = geometrySmith( normalViewDotMax, normalLightDotMax, roughness );
        vec3 fresnel             = fresnelSchlick( max( halfwayViewDot, 0.0 ), baseReflectivity );

        vec3 specular = normalDistribution * geometry * fresnel / ( 4.0 * normalViewDotMax * normalLightDotMax + 0.0001 );
        vec3 diffuse  = albedo / PI;
        vec3 kS       = fresnel;
        vec3 kD       = ( 1.0 - kS ) * ( 1.0 - metallic );

        outRadiance += ( kD * diffuse + specular ) * radiance * normalLightDotMax;
    }

    const float ao  = 0.98;
    vec3 kS         = fresnelSchlick( normalViewDotMax, baseReflectivity );
    vec3 kD         = ( 1.0 - kS ) * ( 1.0 - metallic );
    vec3 irradiance = texture( irradianceMap, normal ).rgb;
    vec3 diffuse    = irradiance * albedo;
    vec3 ambient    = ( kD * diffuse ) * ao;

    vec3 color   = ambient + outRadiance;
    color        = color / ( color + vec3( 1.0 ) );
    outFragColor = vec4( color, 1.0 );
}