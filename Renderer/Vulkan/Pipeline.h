#pragma once
#if defined RENDERER_VULKAN
#include "Wrapper/vulkan.h"

#include "Core/Vertex.h"
#include <Common/Util/Memory.h>

#include <unordered_map>
#include <vector>

// forward declaration
namespace Cosmos::Renderer::Vulkan { class Device; }
namespace Cosmos::Renderer::Vulkan { class Renderpass; }
namespace Cosmos::Renderer::Vulkan { class Shader; }

namespace Cosmos::Renderer::Vulkan
{
	class Pipeline
	{
	public:

		struct CreateInfo
		{
			// these must be created before creating the Pipeline
			Shared<Renderpass> renderPass;                              // current render pass that the pipeline will be applyed to
			Shared<Shader> vertexShader;								// vertex shader of the pipeline
			Shared<Shader> fragmentShader;								// fragment shader of the pipeline
			std::vector<Vertex::Component> vertexComponents = {};       // components the vertex have
			bool passingVertexData = true;								// disable this when not passing vertex data to the shader
			std::vector<VkDescriptorSetLayoutBinding> bindings = {};    // binding data (buffer, textures, etc)
			std::vector<VkPushConstantRange> pushConstants = {};        // optioanlly push constant when creating pipeline

			// these will be auto generated, but can be previously modified between Pipeline constructor and Build
			std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCI = {};
			VkPipelineVertexInputStateCreateInfo VISCI = {};
			VkPipelineInputAssemblyStateCreateInfo IASCI = {};
			VkPipelineViewportStateCreateInfo VSCI = {};
			VkPipelineRasterizationStateCreateInfo RSCI = {};
			VkPipelineMultisampleStateCreateInfo MSCI = {};
			VkPipelineDepthStencilStateCreateInfo DSSCI = {};
			VkPipelineColorBlendAttachmentState CBAS = {};
			VkPipelineColorBlendStateCreateInfo CBSCI = {};
			VkPipelineDynamicStateCreateInfo DSCI = {};
		};

	public:

		// constructor
		Pipeline(Shared<Device> device, CreateInfo ci, VkPipelineCache pipelineCache);

		// destructor
		~Pipeline();

		// returns a reference to the pipeline specification
		inline CreateInfo& GetCreateInfoRef() { return mCreateInfo; }

		// returns the pipeline cache used on this pipeline
		inline VkPipelineCache GetPipelineCache() const { return mPipelineCache; }

		// returns the descriptor set layout
		inline VkDescriptorSetLayout GetDescriptorSetLayout() const { return mDescriptorSetLayout; }

		// returns the pipeline layout
		inline VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

		// returns the pipeline
		inline VkPipeline GetPipeline() const { return mPipeline; }

	public:

		// constructs the pipeline
		void Build();

	private:

		// returns a filled VISCI structure based on requested components
		VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputState(const std::vector<Vertex::Component> components);

		// returns the binding descriptions VISCI will use
		std::vector<VkVertexInputBindingDescription> GetBindingDescriptions(bool useVertexData = true);

		// returns the attribute descriptions VISCI will use
		std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(const std::vector<Vertex::Component> components);

		// assign correct buffer values for the attribute
		VkVertexInputAttributeDescription GetInputAttributeDescription(uint32_t binding, uint32_t location, Vertex::Component component);

	private:

		Shared<Device> mDevice;
		CreateInfo mCreateInfo = {};
		VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		VkPipeline mPipeline = VK_NULL_HANDLE;

		std::vector<VkVertexInputBindingDescription> mBindingDescriptions = {};
		std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions = {};
	};
}
#endif