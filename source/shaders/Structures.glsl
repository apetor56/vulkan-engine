layout( set = 0, binding = 0 ) uniform SceneData {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 lightColor;
    vec3 lightPosition;
    int alignment1;
    vec3 cameraPosition;
    int alignment2;
}
sceneData;

layout( set = 1, binding = 0 ) uniform GLTFMaterialData {
    vec4 colorFactors;
    vec4 metallicRoughnessFactors;
}
materialData;

layout( set = 1, binding = 1 ) uniform sampler2D albedoMap;
layout( set = 1, binding = 2 ) uniform sampler2D normalMap;
layout( set = 1, binding = 3 ) uniform sampler2D metallicRoughnessMap;

layout( set = 2, binding = 0 ) uniform samplerCube irradianceMap;
