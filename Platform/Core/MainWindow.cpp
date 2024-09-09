#include "MainWindow.h"

#include "Input.h"
#include "PlatformDetection.h"
#include "Event/KeyboardEvent.h"
#include "Event/MouseEvent.h"
#include "Event/WindowEvent.h"

#include <Common/Debug/Logger.h>
#include <Engine/Core/Application.h>
#include <Renderer/GUI/GUI.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26819)
#endif

#if defined(PLATFORM_LINUX) && defined(RENDERER_VULKAN)
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_scancode.h>
#elif defined(PLATFORM_WINDOWS) && defined(RENDERER_VULKAN)
#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_scancode.h>
#endif

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

namespace Cosmos::Platform
{
	static MainWindow* s_Instance = nullptr;

	void MainWindow::Initialize(Engine::Application* application, const char* title, int width, int height, bool fullScreen)
	{
		if (s_Instance) {
			COSMOS_LOG(Logger::Warn, "Warning: Attempting to initialize MainWindow when it's already initialized\n");
			return;
		}

		s_Instance = new MainWindow(application);

#if defined(RENDERER_VULKAN)
		s_Instance->mNativeWindow = SDL_CreateWindow
		(
			title,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			fullScreen ? SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
		);
#endif

        COSMOS_LOG(Logger::Todo, " FUNC:StaleResizeFramebuffer this is wacky but we can't create a swapchain with the window minimized, therefore we must stalle the window somehow \
            I have choosen to wait the application until it's not minimized anymore but this may desired in some cases");
	}

	void MainWindow::Shutdown()
	{
		SDL_DestroyWindow(s_Instance->mNativeWindow);

		delete s_Instance;
		s_Instance = nullptr;
	}

	MainWindow& MainWindow::GetRef()
	{
		if (!s_Instance) {
            COSMOS_LOG(Logger::Error,"MainWindow has not been initialized\n");
		}

		return *s_Instance;
	}

	MainWindow::MainWindow(Engine::Application* application)
        : mApplication(application)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
            COSMOS_LOG(Logger::Error,"Could not initialize SDL. Error: %s", SDL_GetError());
			return;
		}

		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
	}

	MainWindow::~MainWindow()
	{
		SDL_Quit();
	}

	void MainWindow::OnUpdate()
	{
		SDL_Event SDL_E;

		while (SDL_PollEvent(&SDL_E))
		{
            Renderer::GUI::GetRef().HandleInputEvent(&SDL_E);

            switch (SDL_E.type)
			{
                // input
                case SDL_KEYDOWN:
                {
                    Shared<KeyboardPressEvent> event = CreateShared<KeyboardPressEvent>((Keycode)SDL_E.key.keysym.sym, (Keymod)SDL_E.key.keysym.mod);
                    mApplication->OnEvent(event);

                    break;
                }

                case SDL_KEYUP:
                {
                    Shared<KeyboardReleaseEvent> event = CreateShared<KeyboardReleaseEvent>((Keycode)SDL_E.key.keysym.sym, (Keymod)SDL_E.key.keysym.mod);
                    mApplication->OnEvent(event);

                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                {
                    Shared<MousePressEvent> event = CreateShared<MousePressEvent>((Buttoncode)SDL_E.button.button);
                    mApplication->OnEvent(event);

                    break;
                }

                case SDL_MOUSEBUTTONUP:
                {
                    Shared<MouseReleaseEvent> event = CreateShared<MouseReleaseEvent>((Buttoncode)SDL_E.button.button);
                    mApplication->OnEvent(event);

                    break;
                }

                case SDL_MOUSEWHEEL:
                {
                    Shared<MouseWheelEvent> event = CreateShared<MouseWheelEvent>(SDL_E.wheel.y);
                    mApplication->OnEvent(event);

                    break;
                }

                case SDL_MOUSEMOTION:
                {
                    Shared<MouseMoveEvent> event = CreateShared<MouseMoveEvent>(SDL_E.motion.xrel, SDL_E.motion.yrel);
                    mApplication->OnEvent(event);

                    break;
                }

                // window
                case SDL_QUIT:
                {
                    HintQuit(true);

                    Shared<WindowCloseEvent> event = CreateShared<WindowCloseEvent>();
                    mApplication->OnEvent(event);

                    break;
                }

                case SDL_WINDOWEVENT:
                {
                    if (SDL_E.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || SDL_E.window.event == SDL_WINDOWEVENT_MINIMIZED)
                    {
                        HintResize(true);
                    
                        Shared<WindowResizeEvent> event = CreateShared<WindowResizeEvent>(SDL_E.window.data1, SDL_E.window.data2);
                        mApplication->OnEvent(event);
                    }

                    break;
                }

                // not handling other events
                default: break;
			}
		}
	}

    void MainWindow::ToggleCursor(bool hide)
    {
        if (hide) {
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetRelativeMouseMode(SDL_TRUE);

            return;
        }

        SDL_ShowCursor(SDL_ENABLE);
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    float MainWindow::GetAspectRatio()
    {
        int32_t width = 0;
        int32_t height = 0;
        GetFrameBufferSize(&width, &height);

        if (height == 0) // avoid division by 0
        {
            return 1.0f;
        }

        float aspect = ((float)width / (float)height);
        return aspect;
    }

    void MainWindow::GetRelativePosition(int* x, int* y)
    {
        SDL_GetWindowPosition(mNativeWindow, x, y);
    }

    void MainWindow::GetInstanceExtensions(unsigned int* count, const char** names)
    {
#if defined RENDERER_VULKAN
        SDL_Vulkan_GetInstanceExtensions(mNativeWindow, count, names);
#endif
    }

    void MainWindow::CreateSurface(void* instance, void** surface)
    {
#if defined RENDERER_VULKAN
        if (!SDL_Vulkan_CreateSurface(mNativeWindow, (VkInstance)instance, (VkSurfaceKHR*)(surface)))
        {
            COSMOS_LOG(Logger::Error, "The creation of a SDL Window Surface for Vulkan.Error:% s\n", SDL_GetError());
        }
#endif
    }

    void MainWindow::GetFrameBufferSize(int* width, int* height)
    {
#if defined RENDERER_VULKAN
        SDL_Vulkan_GetDrawableSize(mNativeWindow, width, height);
#endif
    }

    void MainWindow::StaleResizeFramebuffer()
    {
        SDL_Event e;
        while (SDL_GetWindowFlags(mNativeWindow) & SDL_WINDOW_MINIMIZED)
        {
            SDL_WaitEvent(&e);
        }
    }
}