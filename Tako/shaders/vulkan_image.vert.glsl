#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 projection;
} ubo;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;

layout(location = 0) out vec2 texcoord_out;

layout( push_constant ) uniform constants
{
	mat4 model;
} PushConstants;

void main() {
    gl_Position = normalize(ubo.projection * PushConstants.model * vec4(position, 0.0, 1.0));
    texcoord_out = texcoord;
}
