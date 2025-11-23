#version 430

layout( location = 0 ) in vec3 inTexCoords;

layout( location = 0 ) out vec4 fragColor;

layout( set = 1, binding = 0 ) uniform samplerCube skybox;

void main() {
    fragColor = texture( skybox, inTexCoords );
}
