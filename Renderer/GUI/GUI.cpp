#include "GUI.h"

#include "Icon.h"
#include "Theme.h"
#include "Widget.h"
#include "Core/Context.h"
#include "Core/Defines.h"
#include "Vulkan/Device.h"
#include "Vulkan/Instance.h"
#include "Vulkan/Renderpass.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/Texture.h"
#include "Wrapper/imgui.h"

#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>

#include <Platform/Core/MainWindow.h>
#include <Platform/Event/EventBase.h>

#include <backends/imgui_impl_sdl2.cpp>
#include <backends/imgui_impl_vulkan.cpp>

namespace Cosmos::Renderer
{
	static GUI* s_Instance = nullptr;
	static ImFont* sIconFA = nullptr;
	static ImFont* sIconLC = nullptr;

	void GUI::Initialize()
	{
		if (s_Instance) {
			COSMOS_LOG(Logger::Warn, "Attempting to initialize MainWindow when it's already initialized\n");
			return;
		}

		s_Instance = new GUI();
	}

	void GUI::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	GUI& GUI::GetRef()
	{
		if (!s_Instance) {
			COSMOS_LOG(Logger::Error, "MainWindow has not been initialized\n");
		}

		return *s_Instance;
	}

	GUI::GUI()
	{
		auto& renderer = Context::GetRef();
		renderer.GetRenderpassesLibraryRef().Insert("UI", CreateShared<Vulkan::Renderpass>(renderer.GetDevice(), "UI", VK_SAMPLE_COUNT_1_BIT));

		CreateResources();
		SetupBackend();
	}

	GUI::~GUI()
	{
		auto& renderer = Context::GetRef();
		vkDeviceWaitIdle(renderer.GetDevice()->GetLogicalDevice());
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();

		for (auto& widget : mWidgets.GetElementsRef()) {
			delete widget;
		}

		mWidgets.GetElementsRef().clear();

		ImGui::DestroyContext();
	}

	void GUI::OnUpdate()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnUpdate();
		}

		// end frame
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void GUI::OnRender()
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnRender();
		}
	}

	void GUI::OnEvent(Shared<Platform::EventBase> event)
	{
		// resize the ui
		if (event->GetType() == Platform::EventType::WindowResize)
		{
			auto& renderer = Context::GetRef();
			auto& renderpass = renderer.GetRenderpassesLibraryRef().GetRef("UI");

			vkDeviceWaitIdle(renderer.GetDevice()->GetLogicalDevice());

			// recreate frame buffers
			for (auto framebuffer : renderpass->GetFramebuffersRef()) {
				vkDestroyFramebuffer(renderer.GetDevice()->GetLogicalDevice(), framebuffer, nullptr);
			}

			renderpass->GetFramebuffersRef().resize(renderer.GetSwapchain()->GetImageViews().size());

			for (size_t i = 0; i < renderer.GetSwapchain()->GetImageViews().size(); i++)
			{
				VkImageView attachments[] = { renderer.GetSwapchain()->GetImageViews()[i] };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = renderpass->GetRenderpassRef();
				framebufferCI.attachmentCount = 1;
				framebufferCI.pAttachments = attachments;
				framebufferCI.width = renderer.GetSwapchain()->GetExtent().width;
				framebufferCI.height = renderer.GetSwapchain()->GetExtent().height;
				framebufferCI.layers = 1;
				COSMOS_ASSERT(vkCreateFramebuffer(renderer.GetDevice()->GetLogicalDevice(), &framebufferCI, nullptr, &renderpass->GetFramebuffersRef()[i]) == VK_SUCCESS, "Failed to create framebuffer");
			}
		}

		// resize ui objects
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnEvent(event);
		}
	}

	void GUI::HandleInputEvent(SDL_Event* e)
	{
		ImGui_ImplSDL2_ProcessEvent(e);
	}

	void* GUI::AddTexture(Shared<Vulkan::Texture2D> texture)
	{
		ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
		ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
		
		// create descriptor set
		VkDescriptorSet descriptorSet;
		{
			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = v->DescriptorPool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &bd->DescriptorSetLayout;
			VkResult err = vkAllocateDescriptorSets(v->Device, &alloc_info, &descriptorSet);
			check_vk_result(err);
		}
		
		// update descriptor set
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = texture->GetSampler();
			desc_image[0].imageView = texture->GetView();
			desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = descriptorSet;
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			vkUpdateDescriptorSets(v->Device, 1, write_desc, 0, nullptr);
		}
		
		return descriptorSet;
	}

	void GUI::AddWidget(Widget* widget)
	{
		mWidgets.Push(widget);
	}

	void GUI::ToggleCursor(bool hide)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (hide)
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			return;
		}

		io.ConfigFlags ^= ImGuiConfigFlags_NoMouse;
	}

	void GUI::SetImageCount(uint32_t count)
	{
		ImGui_ImplVulkan_SetMinImageCount(count);
	}

	void GUI::DrawBackendData(void* commandBuffer)
	{
		auto* data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(data, (VkCommandBuffer)commandBuffer);
	}

	void GUI::CreateResources()
	{
		auto& renderer = Context::GetRef();
		auto& renderpass = renderer.GetRenderpassesLibraryRef().GetRef("UI");

		// render pass
		VkAttachmentDescription attachment = {};
		attachment.format = renderer.GetSwapchain()->GetSurfaceFormat().format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachment = {};
		colorAttachment.attachment = 0;
		colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;
		COSMOS_ASSERT(vkCreateRenderPass(renderer.GetDevice()->GetLogicalDevice(), &info, nullptr, &renderpass->GetRenderpassRef()) == VK_SUCCESS, "Failed to create render pass");

		// command pool
		Vulkan::Device::QueueFamilyIndices indices = renderer.GetDevice()->FindQueueFamilies
		(
			renderer.GetDevice()->GetPhysicalDevice(),
			renderer.GetDevice()->GetSurface()
		);

		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		COSMOS_ASSERT(vkCreateCommandPool(renderer.GetDevice()->GetLogicalDevice(), &cmdPoolInfo, nullptr, &renderpass->GetCommandPoolRef()) == VK_SUCCESS, "Failed to create command pool");

		// command buffers
		renderpass->GetCommandfuffersRef().resize(CONCURENTLY_RENDERED_FRAMES);

		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandPool = renderpass->GetCommandPoolRef();
		cmdBufferAllocInfo.commandBufferCount = (uint32_t)renderpass->GetCommandfuffersRef().size();
		COSMOS_ASSERT(vkAllocateCommandBuffers(renderer.GetDevice()->GetLogicalDevice(), &cmdBufferAllocInfo, renderpass->GetCommandfuffersRef().data()) == VK_SUCCESS, "Failed to allocate command buffers");

		// frame buffers
		renderpass->GetFramebuffersRef().resize(renderer.GetSwapchain()->GetImageViews().size());

		for (size_t i = 0; i < renderer.GetSwapchain()->GetImageViews().size(); i++)
		{
			VkImageView attachments[] = { renderer.GetSwapchain()->GetImageViews()[i] };

			VkFramebufferCreateInfo framebufferCI = {};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = renderpass->GetRenderpassRef();
			framebufferCI.attachmentCount = 1;
			framebufferCI.pAttachments = attachments;
			framebufferCI.width = renderer.GetSwapchain()->GetExtent().width;
			framebufferCI.height = renderer.GetSwapchain()->GetExtent().height;
			framebufferCI.layers = 1;
			COSMOS_ASSERT(vkCreateFramebuffer(renderer.GetDevice()->GetLogicalDevice(), &framebufferCI, nullptr, &renderpass->GetFramebuffersRef()[i]) == VK_SUCCESS, "Failed to create framebuffer");
		}
	}

	void GUI::SetupBackend()
	{
		// initial config
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		if (io.BackendFlags | ImGuiBackendFlags_PlatformHasViewports && io.BackendFlags | ImGuiBackendFlags_RendererHasViewports)
		{
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}

		static const ImWchar iconRanges1[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		static const ImWchar iconRanges2[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
		constexpr float iconSize = 13.0f;
		constexpr float fontSize = 18.0f;

		ImFontConfig iconCFG;
		iconCFG.MergeMode = true;
		iconCFG.GlyphMinAdvanceX = iconSize;
		iconCFG.PixelSnapH = true;

		io.Fonts->Clear();
		LoadFont(fontSize);

		sIconFA = io.Fonts->AddFontFromFileTTF(GetAssetSubDir("Font/icon-awesome.ttf").c_str(), iconSize, &iconCFG, iconRanges1);
		sIconLC = io.Fonts->AddFontFromFileTTF(GetAssetSubDir("Font/icon-lucide.ttf").c_str(), iconSize, &iconCFG, iconRanges2);
		io.Fonts->Build();

		io.IniFilename = "UI.ini";
		io.WantCaptureMouse = true;

		ImGui::StyleColorsDark();
		StyleColorsSpectrum();

		// create descriptor pool
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		auto& renderer = Context::GetRef();
		auto& renderpass = renderer.GetRenderpassesLibraryRef().GetRef("UI");

		VkDescriptorPoolCreateInfo poolCI = {};
		poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCI.pNext = nullptr;
		poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolCI.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		poolCI.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		poolCI.pPoolSizes = poolSizes;
		COSMOS_ASSERT(vkCreateDescriptorPool(renderer.GetDevice()->GetLogicalDevice(), &poolCI, nullptr, &renderpass->GetDescriptorPoolRef()) == VK_SUCCESS, "Failed to create descriptor pool for the User Interface");

		// sdl and vulkan initialization
		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForVulkan(Platform::MainWindow::GetRef().GetNativeWindow());

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = renderer.GetInstance()->GetInstance();
		initInfo.PhysicalDevice = renderer.GetDevice()->GetPhysicalDevice();
		initInfo.Device = renderer.GetDevice()->GetLogicalDevice();
		initInfo.Queue = renderer.GetDevice()->GetGraphicsQueue();
		initInfo.DescriptorPool = renderpass->GetDescriptorPoolRef();
		initInfo.MinImageCount = renderer.GetSwapchain()->GetImageCount();
		initInfo.ImageCount = renderer.GetSwapchain()->GetImageCount();
		initInfo.MSAASamples = renderpass->GetMSAA();
		initInfo.Allocator = nullptr;
		initInfo.RenderPass = renderpass->GetRenderpassRef();
		ImGui_ImplVulkan_Init(&initInfo);

		// upload fonts
		ImGui_ImplVulkan_CreateFontsTexture();
	}
}