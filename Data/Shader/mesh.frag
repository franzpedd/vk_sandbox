#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_gpu_shader_int64 : enable

layout(push_constant) uniform constants
{
    uint64_t id;
	mat4 model;
} pushConstant;

layout(binding = 4) uniform sampler2D colorMapSampler;

layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    // create the texture
    outColor = texture(colorMapSampler, inFragTexCoord);
}