#pragma once
#if defined RENDERER_VULKAN
#include "Wrapper/vulkan.h"

#include <Common/Util/Memory.h>
#include <vector>

// forward declaration
namespace Cosmos::Renderer::Vulkan { class Device; }

namespace Cosmos::Renderer::Vulkan
{
	enum class ShaderType // using shaderc values but only including it on cpp file
	{
		Vertex = 0,
		Fragment = 1,
		Compute = 2,
		Geometry = 3,
		TessControl = 4,
		TessEvaluation = 5
	};

	class Shader
	{
	public:

		// constructor
		Shader(Shared<Device> device, ShaderType type, const char* name, const char* path);

		// destructor
		~Shader();

		// returns the shader type
		inline ShaderType GetType() { return mType; }

		// returnsa reference to the shader name
		inline const char* GetName() { return mName; }

		// returns a reference to the shader path
		inline const char* GetPath() { return mPath; }

		// returns the shader module
		inline VkShaderModule GetModule() { return mShaderModule; }

		// returns a reference to the shader stage info
		VkPipelineShaderStageCreateInfo& GetShaderStageCreateInfoRef() { return mShaderStageCI; }

	private:

		// reads and returns shader's binary
		std::vector<char> ReadSPIRV();

		// compiles and returns a source shader
		std::vector<uint32_t> Compile(const char* source, ShaderType type, bool optimize = false);

		// creates the shader's module of the spir-v binary
		void CreateShaderModule(const std::vector<char>& binary);

		// creates the shaders tage specification
		void CreateShaderStage();

	private:

		Shared<Device> mDevice;
		ShaderType mType;
		const char* mName;
		const char* mPath;
		VkShaderModule mShaderModule = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo mShaderStageCI = {};
	};
}

#endif