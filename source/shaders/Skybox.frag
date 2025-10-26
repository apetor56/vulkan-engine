#version 450

layout( location = 0 ) in vec3 inTexCoords;

layout( location = 0 ) out vec4 fragColor;

layout( set = 1, binding = 0 ) uniform samplerCube skybox;

void main() {
    vec3 envColor = texture( skybox, inTexCoords ).rgb;
    envColor = envColor / ( envColor + vec3( 1.0 ) );
    fragColor = vec4( envColor, 1.0 );
}
