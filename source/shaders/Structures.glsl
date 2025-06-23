layout( set = 0, binding = 0 ) uniform SceneData {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 ambientColor;
    vec4 directionToLight;
    vec4 lightColor;
}
sceneData;

layout( set = 1, binding = 0 ) uniform GLTFMaterialData {
    vec4 colorFactors;
    vec4 metalRoughnessFactors;
}
materialData;

layout( set = 1, binding = 1 ) uniform sampler2D colorTex;
