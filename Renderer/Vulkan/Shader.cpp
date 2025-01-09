#if defined RENDERER_VULKAN
#include "Shader.h"

#include "Device.h"
#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26439)
#endif
#include <shaderc/shaderc.hpp>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#include <fstream>
#include <sstream>

namespace Cosmos::Renderer::Vulkan
{
	Shader::Shader(Shared<Device> device, ShaderType type, const char* name, const char* path)
		: mDevice(device), mType(type), mName(name), mPath(path)
	{
		// reads raw shader
		std::ifstream read(path);
		std::stringstream buffer;

		buffer << read.rdbuf();
		read.close();

		std::string auxname = path;
		auxname += ".spv";

		// writes compiled shared
		std::vector<uint32_t> bin = Compile(buffer.str().c_str(), type, false);
		std::ofstream write(auxname.c_str(), std::ios::out | std::ios::binary);

		if (write.is_open())
		{
			write.write(reinterpret_cast<char*>(&bin[0]), bin.size() * sizeof(bin[0]));
			write.close();
		}

		CreateShaderModule(ReadSPIRV());
		CreateShaderStage();
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(mDevice->GetLogicalDevice(), mShaderModule, nullptr);
	}

	std::vector<char> Shader::ReadSPIRV()
	{
		std::string name = mPath;
		name += ".spv";

		std::ifstream bin(name, std::ios::ate | std::ios::binary);

		size_t fileSize = (size_t)bin.tellg();
		std::vector<char> buffer(fileSize);

		bin.seekg(0);
		bin.read(buffer.data(), fileSize);
		bin.close();

		return buffer;
	}

	std::vector<uint32_t> Shader::Compile(const char* source, ShaderType type, bool optimize)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_size);
		}

		shaderc::SpvCompilationResult res = compiler.CompileGlslToSpv(source, (shaderc_shader_kind)type, mPath, options);
		std::ostringstream msg;
		msg << "Failed to Compile shader " << mPath << "Details: " << res.GetErrorMessage();

		COSMOS_ASSERT(res.GetCompilationStatus() == shaderc_compilation_status_success, msg.str().c_str());

		return { res.cbegin(), res.cend() };
	}

	void Shader::CreateShaderModule(const std::vector<char>& binary)
	{
		VkShaderModuleCreateInfo shaderModuleCI = {};
		shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCI.pNext = nullptr;
		shaderModuleCI.flags = 0;
		shaderModuleCI.codeSize = binary.size();
		shaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(binary.data());

		COSMOS_ASSERT(vkCreateShaderModule(mDevice->GetLogicalDevice(), &shaderModuleCI, nullptr, &mShaderModule) == VK_SUCCESS, "Failed to create shader module");
	}

	void Shader::CreateShaderStage()
	{
		mShaderStageCI = {};
		mShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		mShaderStageCI.pNext = nullptr;
		mShaderStageCI.flags = 0;
		mShaderStageCI.pName = "main";
		mShaderStageCI.module = mShaderModule;
		mShaderStageCI.pSpecializationInfo = nullptr;

		switch (mType)
		{
			case ShaderType::Vertex:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			}

			case ShaderType::Fragment:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			}

			case ShaderType::Compute:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT;
				break;
			}

			case ShaderType::Geometry:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
				break;
			}

			case ShaderType::TessControl:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				break;
			}

			case ShaderType::TessEvaluation:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				break;
			}
		}
	}
}

#endif