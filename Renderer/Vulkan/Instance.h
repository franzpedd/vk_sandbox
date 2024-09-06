#pragma once
#if defined RENDERER_VULKAN

#include "Wrapper/vulkan.h"
#include <string>
#include <vector>

namespace Cosmos::Renderer::Vulkan
{
	class Instance
	{
	public:

		// constructor
		Instance(std::string engineName, std::string appName, bool validations, std::string appVersion, std::string vulkanVersion);

		// destructor
		~Instance();

	public:

		// returns a reference to the vulkan instance
		inline VkInstance GetInstance() const { return mInstance; }

		// returns a reference to the vulkan debug utils messenger
		inline VkDebugUtilsMessengerEXT GetDebugger() { return mDebugMessenger; }

		// returns if validations are enabled
		inline bool GetValidations() const { return mValidations; }

		// returns listed validations
		inline const std::vector<const char*> GetValidationsList() const { return mValidationList; }

	public:

		// returns the required extensions by the vkinstance
		std::vector<const char*> GetRequiredExtensions();

		// internal errors callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallback, void* pUserData);

	private:

		bool mValidations = true;
		const std::vector<const char*> mValidationList = { "VK_LAYER_KHRONOS_validation" };
		VkInstance mInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
	};
}

#endif