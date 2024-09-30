#if defined RENDERER_VULKAN
#include "Instance.h"

#include <Common/Debug/Logger.h>
#include <Platform/Core/MainWindow.h>

namespace Cosmos::Renderer::Vulkan
{
	Instance::Instance(std::string engineName, std::string appName, bool validations, std::string appVersion, std::string vulkanVersion)
		: mValidations(validations)
	{
		// we first initialize volk
		COSMOS_ASSERT(volkInitialize() == VK_SUCCESS, "Failed to initialize volk");

		uint32_t appV[3] = { 0, 0, 1 };
		if (appVersion.size() == 5) {
			appV[0] = (uint32_t)appVersion[0] - '0';
			appV[1] = (uint32_t)appVersion[2] - '0';
			appV[2] = (uint32_t)appVersion[4] - '0';
		}

		uint32_t vkV[3] = { 1, 2, 0 };
		if (vulkanVersion.size() == 5) {
			vkV[0] = vulkanVersion[0] - '0';
			vkV[1] = vulkanVersion[2] - '0';
			vkV[2] = vulkanVersion[4] - '0';
		}

		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = appName.c_str();
		applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, appV[0], appV[1], appV[2]);
		applicationInfo.pEngineName = engineName.c_str();
		applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, appV[0], appV[1], appV[2]);
		applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, vkV[0], vkV[1], vkV[2]);

		auto extensions = GetRequiredExtensions();

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI = {};
		VkInstanceCreateInfo instanceCI = {};
		instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCI.flags = 0;
#if defined PLATFORM_APPLE
		instanceCI.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
		instanceCI.pApplicationInfo = &applicationInfo;

		if (validations)
		{
			instanceCI.enabledLayerCount = (uint32_t)mValidationList.size();
			instanceCI.ppEnabledLayerNames = mValidationList.data();

			debugUtilsCI = {};
			debugUtilsCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsCI.pNext = nullptr;
			debugUtilsCI.flags = 0;
			debugUtilsCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsCI.pfnUserCallback = DebugCallback;
			debugUtilsCI.pUserData = nullptr;

			instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsCI;
		}

		else
		{
			instanceCI.pNext = nullptr;
			instanceCI.enabledLayerCount = 0;
			instanceCI.ppEnabledLayerNames = nullptr;
		}

		instanceCI.enabledExtensionCount = (uint32_t)extensions.size();
		instanceCI.ppEnabledExtensionNames = extensions.data();
		COSMOS_ASSERT(vkCreateInstance(&instanceCI, nullptr, &mInstance) == VK_SUCCESS, "Failed to create Vulkan Instance");

		// send instance to volk
		volkLoadInstance(mInstance);

		if (validations)
		{
			debugUtilsCI = {};
			debugUtilsCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsCI.pNext = nullptr;
			debugUtilsCI.flags = 0;
			debugUtilsCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsCI.pfnUserCallback = DebugCallback;
			debugUtilsCI.pUserData = nullptr;
			COSMOS_ASSERT(vkCreateDebugUtilsMessengerEXT(mInstance, &debugUtilsCI, nullptr, &mDebugMessenger) == VK_SUCCESS, "Failed to create Vulkan Debug Messenger");
		}
	}

	Instance::~Instance()
	{
		if (mValidations) {
			vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
		}

		vkDestroyInstance(mInstance, nullptr);
	}

	std::vector<const char*> Instance::GetRequiredExtensions()
	{
		uint32_t count = 0;
		const char** glfwExtensions = Platform::MainWindow::GetRef().GetInstanceExtensions(&count);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + count);
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#if defined PLATFORM_APPLE
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

		if (mValidations)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		return extensions;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallback, void* pUserData)
	{
		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			COSMOS_LOG(Logger::Error, "Validation Layer : % s", pCallback->pMessage);
			return VK_FALSE;
		}

		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			COSMOS_LOG(Logger::Warn, "Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			COSMOS_LOG(Logger::Info, "Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			COSMOS_LOG(Logger::Trace, "Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		return VK_FALSE;
	}
}

#endif