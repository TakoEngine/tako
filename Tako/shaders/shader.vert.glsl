#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
} ubo;

layout(set = 1, binding = 0) uniform LightSettings
{
	vec3 lightPos;
} lightUBO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 positionWorld;
layout(location = 3) out vec3 normalCamera;
layout(location = 4) out vec3 eyeDirection;
layout(location = 5) out vec3 lightDirection;

layout( push_constant ) uniform constants
{
	mat4 model;
} PushConstants;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.proj * ubo.view * PushConstants.model * vec4(inPosition, 1.0);
    fragColor = inColor;
	texCoord = inTexCoord;

	positionWorld = (PushConstants.model * vec4(inPosition, 1)).xyz;

	vec3 vertexPosCamera = (ubo.view * PushConstants.model * vec4(inPosition,1)).xyz;
	eyeDirection = vec3(0,0,0) - vertexPosCamera;

	vec3 lightPosCamera = (ubo.view * vec4(lightUBO.lightPos, 1)).xyz;
	lightDirection = lightPosCamera + eyeDirection;

	normalCamera = ( ubo.view * PushConstants.model * vec4(inNormal, 0)).xyz;
}
