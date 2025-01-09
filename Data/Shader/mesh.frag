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

layout(location = 0) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    // create the texture
    outColor = texture(colorMapSampler, inFragTexCoord);

    // if it's marked as selected, paint it
    if(pushConstant.selected == 1) {
        outColor *= vec4(0.9059, 0.4275, 0.0353, 0.75);
    }
}