#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform LightSettings
{
	vec3 lightPos;
} lightUBO;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 positionWorld;
layout(location = 3) in vec3 normalCamera;
layout(location = 4) in vec3 eyeDirection;
layout(location = 5) in vec3 lightDirection;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D tex;

void main() {
    vec3 texColor = texture(tex, texCoord).xyz;
    float dist = length(lightUBO.lightPos - positionWorld);
    vec3 n = normalize(normalCamera);
    vec3 l = normalize(lightDirection);
    float cosTheta = clamp( dot(n,l), 0, 1);

    vec3 E = normalize(eyeDirection);
    vec3 R = reflect(-l, n);
    float cosAlpha = clamp( dot(E,R), 0, 1);

    outColor =
        vec4(texColor * vec3(0.1,0.1,0.1),1) +
        vec4(texColor * vec3(1,1,1) * 200 * cosTheta / (dist*dist), 1.0f) +
        vec4(vec3(0.3,0.3,0.3) * vec3(1,1,1) * 200 * pow(cosAlpha, 5) / (dist*dist), 1);
}
