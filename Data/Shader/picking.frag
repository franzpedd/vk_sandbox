#version 450
#extension GL_ARB_gpu_shader_int64 : enable

layout(push_constant) uniform constants
{
    uint selected;
    uint64_t id;
	mat4 model;
} pushConstant;

layout(set = 0, binding = 0) uniform ubo_camera
{
    vec2 mousepos;
    mat4 view;
    mat4 proj;
} camera;

layout(binding = 1) uniform sampler2D colorMapSampler;

layout(location = 0) out uvec2 outColor;

void main()
{
    // we're going to separate our uint64_t into a vec2 of float, latter on CPU code we're going to read it back
    // extract lower 32 bits
    uint lowerBits = uint(pushConstant.id & 0xFFFFFFFFUL); 

    // extract upper 32 bits
    uint upperBits = uint(pushConstant.id >> 32);         

    // convert to uvec2 by packing the parts as floats
    outColor = uvec2(uint(lowerBits), uint(upperBits));
}