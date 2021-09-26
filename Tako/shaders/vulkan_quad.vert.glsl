#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    vec4 color;
} ubo;

layout(location = 0) in vec2 position;

layout(location = 0) out vec4 fragColor;

layout( push_constant ) uniform constants
{
	mat4 model;
} PushConstants;

void main() {
    gl_Position = normalize(ubo.projection * PushConstants.model * vec4(position, 0.0, 1.0));
    fragColor = ubo.color;
}
