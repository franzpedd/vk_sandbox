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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec2 outFragTexCoord;

void main()
{
    // set vertex position on world
    gl_Position = camera.proj * camera.view * pushConstant.model * vec4(inPosition, 1.0);

    // output variables for the fragment shader
    outFragTexCoord = inTexCoord;
}