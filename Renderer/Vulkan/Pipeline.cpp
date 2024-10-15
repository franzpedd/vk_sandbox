#if defined RENDERER_VULKAN
#include "Pipeline.h"

#include "Device.h"
#include "Renderpass.h"
#include "Shader.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Renderer::Vulkan
{
    Pipeline::Pipeline(Shared<Device> device, CreateInfo ci, VkPipelineCache pipelineCache)
        : mDevice(device), mCreateInfo(ci), mPipelineCache(pipelineCache)
    {
        // assign shader stages
        mCreateInfo.shaderStagesCI = { mCreateInfo.vertexShader->GetShaderStageCreateInfoRef(),  mCreateInfo.fragmentShader->GetShaderStageCreateInfoRef() };

        // descriptor set
        VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
        descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descSetLayoutCI.pNext = nullptr;
        descSetLayoutCI.flags = 0;
        descSetLayoutCI.bindingCount = (uint32_t)mCreateInfo.bindings.size();
        descSetLayoutCI.pBindings = mCreateInfo.bindings.data();
        COSMOS_ASSERT(vkCreateDescriptorSetLayout(mDevice->GetLogicalDevice(), &descSetLayoutCI, nullptr, &mDescriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor set layout");

        // pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.pNext = nullptr;
        pipelineLayoutCI.flags = 0;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &mDescriptorSetLayout;
        pipelineLayoutCI.pushConstantRangeCount = (uint32_t)mCreateInfo.pushConstants.size();
        pipelineLayoutCI.pPushConstantRanges = mCreateInfo.pushConstants.data();
        COSMOS_ASSERT(vkCreatePipelineLayout(mDevice->GetLogicalDevice(), &pipelineLayoutCI, nullptr, &mPipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout");

        // vertex input state
        mCreateInfo.VISCI = GetPipelineVertexInputState(mCreateInfo.vertexComponents);
        // input assembly state
        mCreateInfo.IASCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mCreateInfo.IASCI.pNext = nullptr;
        mCreateInfo.IASCI.flags = 0;
        mCreateInfo.IASCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        mCreateInfo.IASCI.primitiveRestartEnable = VK_FALSE;
        // viewport state
        mCreateInfo.VSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        mCreateInfo.VSCI.pNext = nullptr;
        mCreateInfo.VSCI.flags = 0;
        mCreateInfo.VSCI.viewportCount = 1;
        mCreateInfo.VSCI.pViewports = nullptr; // using dynamic viewport
        mCreateInfo.VSCI.scissorCount = 1;
        mCreateInfo.VSCI.pScissors = nullptr; // using dynamic scissor
        // rasterization state
        mCreateInfo.RSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mCreateInfo.RSCI.pNext = nullptr;
        mCreateInfo.RSCI.flags = 0;
        mCreateInfo.RSCI.polygonMode = VK_POLYGON_MODE_FILL;
        mCreateInfo.RSCI.cullMode = VK_CULL_MODE_NONE;
        mCreateInfo.RSCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        mCreateInfo.RSCI.depthClampEnable = VK_FALSE;
        mCreateInfo.RSCI.rasterizerDiscardEnable = VK_FALSE;
        mCreateInfo.RSCI.lineWidth = 1.0f;
        // multisampling state
        mCreateInfo.MSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        mCreateInfo.MSCI.pNext = nullptr;
        mCreateInfo.MSCI.flags = 0;
        mCreateInfo.MSCI.rasterizationSamples = mCreateInfo.renderPass->GetMSAA();
        mCreateInfo.MSCI.sampleShadingEnable = VK_FALSE;
        // depth stencil state
        mCreateInfo.DSSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        mCreateInfo.DSSCI.pNext = nullptr;
        mCreateInfo.DSSCI.flags = 0;
        mCreateInfo.DSSCI.depthTestEnable = VK_TRUE;
        mCreateInfo.DSSCI.depthWriteEnable = VK_TRUE;
        mCreateInfo.DSSCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        mCreateInfo.DSSCI.back.compareOp = VK_COMPARE_OP_ALWAYS;
        // color blend
        mCreateInfo.CBAS.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        mCreateInfo.CBAS.blendEnable = VK_FALSE;
        mCreateInfo.CBAS.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        mCreateInfo.CBAS.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        mCreateInfo.CBAS.colorBlendOp = VK_BLEND_OP_ADD;
        mCreateInfo.CBAS.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        mCreateInfo.CBAS.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        mCreateInfo.CBAS.alphaBlendOp = VK_BLEND_OP_ADD;
        // color blend state
        mCreateInfo.CBSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        mCreateInfo.CBSCI.pNext = nullptr;
        mCreateInfo.CBSCI.flags = 0;
        mCreateInfo.CBSCI.attachmentCount = 1;
        mCreateInfo.CBSCI.pAttachments = &mCreateInfo.CBAS;
        mCreateInfo.CBSCI.logicOpEnable = VK_FALSE;
        mCreateInfo.CBSCI.logicOp = VK_LOGIC_OP_COPY;
        mCreateInfo.CBSCI.blendConstants[0] = 0.0f;
        mCreateInfo.CBSCI.blendConstants[1] = 0.0f;
        mCreateInfo.CBSCI.blendConstants[2] = 0.0f;
        mCreateInfo.CBSCI.blendConstants[3] = 0.0f;
        // dynamic state
        mCreateInfo.DSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        mCreateInfo.DSCI.pNext = nullptr;
        mCreateInfo.DSCI.flags = 0;
        mCreateInfo.DSCI.dynamicStateCount = (uint32_t)mCreateInfo.dynamicStates.size();
        mCreateInfo.DSCI.pDynamicStates = mCreateInfo.dynamicStates.data();
    }

    Pipeline::~Pipeline()
    {
        vkDeviceWaitIdle(mDevice->GetLogicalDevice());

        vkDestroyPipeline(mDevice->GetLogicalDevice(), mPipeline, nullptr);
        vkDestroyPipelineLayout(mDevice->GetLogicalDevice(), mPipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(mDevice->GetLogicalDevice(), mDescriptorSetLayout, nullptr);
    }

    void Pipeline::Build()
    {
        // pipeline creation
        VkGraphicsPipelineCreateInfo pipelineCI = {};
        pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCI.pNext = nullptr;
        pipelineCI.flags = 0;
        pipelineCI.stageCount = (uint32_t)mCreateInfo.shaderStagesCI.size();
        pipelineCI.pStages = mCreateInfo.shaderStagesCI.data();
        pipelineCI.pVertexInputState = &mCreateInfo.VISCI;
        pipelineCI.pInputAssemblyState = &mCreateInfo.IASCI;
        pipelineCI.pViewportState = &mCreateInfo.VSCI;
        pipelineCI.pRasterizationState = &mCreateInfo.RSCI;
        pipelineCI.pMultisampleState = &mCreateInfo.MSCI;
        pipelineCI.pDepthStencilState = &mCreateInfo.DSSCI;
        pipelineCI.pColorBlendState = &mCreateInfo.CBSCI;
        pipelineCI.pDynamicState = &mCreateInfo.DSCI;
        pipelineCI.layout = mPipelineLayout;
        pipelineCI.renderPass = mCreateInfo.renderPass->GetRenderpassRef();
        pipelineCI.subpass = 0;
        COSMOS_ASSERT(vkCreateGraphicsPipelines(mDevice->GetLogicalDevice(), mPipelineCache, 1, &pipelineCI, nullptr, &mPipeline) == VK_SUCCESS, "Failed to create graphics pipeline");
    }

    VkPipelineVertexInputStateCreateInfo Pipeline::GetPipelineVertexInputState(const std::vector<Vertex::Component> components)
    {
        mBindingDescriptions = GetBindingDescriptions(mCreateInfo.passingVertexData);
        mAttributeDescriptions = GetAttributeDescriptions(components);

        VkPipelineVertexInputStateCreateInfo VISCI = {};
        VISCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VISCI.pNext = nullptr;
        VISCI.flags = 0;
        VISCI.vertexBindingDescriptionCount = (uint32_t)mBindingDescriptions.size();
        VISCI.pVertexBindingDescriptions = mBindingDescriptions.data();
        VISCI.vertexAttributeDescriptionCount = (uint32_t)mAttributeDescriptions.size();
        VISCI.pVertexAttributeDescriptions = mAttributeDescriptions.data();

        return VISCI;
    }

    std::vector<VkVertexInputBindingDescription> Pipeline::GetBindingDescriptions(bool useVertexData)
    {
        if (!useVertexData)
            return std::vector<VkVertexInputBindingDescription>{};

        mBindingDescriptions.clear();
        mBindingDescriptions.resize(1);

        mBindingDescriptions[0].binding = 0;
        mBindingDescriptions[0].stride = sizeof(Vertex);
        mBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return mBindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Pipeline::GetAttributeDescriptions(const std::vector<Vertex::Component> components)
    {
        std::vector<VkVertexInputAttributeDescription> result = {};
        uint32_t binding = 0;
        uint32_t location = 0;

        for (auto component : components)
        {
            // "converting" Vertex::Component into uint32_t
            location = component;
            result.push_back(GetInputAttributeDescription(binding, location, component));
        }

        return result;
    }

    VkVertexInputAttributeDescription Pipeline::GetInputAttributeDescription(uint32_t binding, uint32_t location, Vertex::Component component)
    {
        switch (component)
        {
            case Vertex::Component::POSITION: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
            case Vertex::Component::NORMAL: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
            case Vertex::Component::UV: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
            case Vertex::Component::JOINT: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_UINT, offsetof(Vertex, joint) });
            case Vertex::Component::WEIGHT: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight) });
            case Vertex::Component::COLOR: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color) });
        }

        return VkVertexInputAttributeDescription();
    }

}
#endif