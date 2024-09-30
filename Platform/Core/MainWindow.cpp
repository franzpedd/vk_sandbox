#include "MainWindow.h"

#include "Input.h"
#include "PlatformDetection.h"
#include "Event/KeyboardEvent.h"
#include "Event/MouseEvent.h"
#include "Event/WindowEvent.h"

#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>
#include <Common/Util/Memory.h>
#include <Engine/Core/Application.h>

#if defined RENDERER_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <stb_image.h>

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

        COSMOS_ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");

#if defined(RENDERER_VULKAN)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        s_Instance->mNativeWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
        COSMOS_ASSERT(s_Instance->mNativeWindow != nullptr, "The main window could not be created");
#endif
        GLFWimage icons[1] = {};
        icons[0].pixels = stbi_load(GetAssetSubDir("Texture/icon.png").c_str(), &icons->width, &icons->height, 0, 4);
        glfwSetWindowIcon(s_Instance->mNativeWindow, 1, icons);
        stbi_image_free(icons[0].pixels);

        COSMOS_LOG(Logger::Todo, "FUNC:StaleResizeFramebuffer this is wacky but we can't create a swapchain with the window minimized, therefore we must stalle the window somehow. I have choosen to wait the application until it's not minimized anymore but this may desired in some cases");
    
        s_Instance->SetCallbacks();
    }

	void MainWindow::Shutdown()
	{
        glfwDestroyWindow(s_Instance->mNativeWindow);
        glfwTerminate();

		delete s_Instance;
		s_Instance = nullptr;
	}

	MainWindow& MainWindow::GetRef()
	{
		if (!s_Instance) {
            COSMOS_LOG(Logger::Error, "MainWindow is not initialized or has been de-initialized\n");
		}

		return *s_Instance;
	}

	MainWindow::MainWindow(Engine::Application* application)
        : mApplication(application)
	{

	}

	MainWindow::~MainWindow()
	{
	}

	void MainWindow::OnUpdate()
	{
        glfwPollEvents();
	}

    void MainWindow::ToggleCursor(bool hide)
    {
        if (hide) {
            glfwSetInputMode(mNativeWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            return;
        }

        glfwSetInputMode(mNativeWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

    void MainWindow::SetCallbacks()
    {
        glfwSetWindowUserPointer(mNativeWindow, this);

        glfwSetErrorCallback([](int code, const char* msg) {
            COSMOS_LOG(Logger::Error, "[GLFW Internal Error]:[Code:%d]:[Message:%s]", code, msg);
            });

        glfwSetFramebufferSizeCallback(mNativeWindow, [](GLFWwindow* window, int width, int height) {
            s_Instance->StaleResizeFramebuffer();
            });

        glfwSetWindowSizeCallback(mNativeWindow, [](GLFWwindow* window, int width, int height) {
            s_Instance->HintResize(true);
            });

        glfwSetWindowIconifyCallback(mNativeWindow, [](GLFWwindow* window, int iconified) {
            s_Instance->HintResize(true);
            });

        glfwSetKeyCallback(mNativeWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            switch (action)
            {
                case GLFW_PRESS: {
                    Shared<KeyboardPressEvent> event = CreateShared<KeyboardPressEvent>((Keycode)key, (Keymod)mods);
                    MainWindow& mainWindow = *(MainWindow*)glfwGetWindowUserPointer(window);
                    mainWindow.mApplication->OnEvent(event);
                    break;
                }

                case GLFW_RELEASE: {
                    Shared<KeyboardReleaseEvent> event = CreateShared<KeyboardReleaseEvent>((Keycode)key, (Keymod)mods);
                    MainWindow& mainWindow = *(MainWindow*)glfwGetWindowUserPointer(window);
                    mainWindow.mApplication->OnEvent(event);
                    break;
                }

                default: { break; }
            }
            });

        glfwSetMouseButtonCallback(mNativeWindow, [](GLFWwindow* window, int button, int action, int mods) {
            switch (action)
            {
                case GLFW_PRESS: {
                    Shared<MousePressEvent> event = CreateShared<MousePressEvent>((Buttoncode)button);
                    MainWindow& mainWindow = *(MainWindow*)glfwGetWindowUserPointer(window);
                    mainWindow.mApplication->OnEvent(event);
                    break;
                }

                case GLFW_RELEASE: {
                    Shared<MouseReleaseEvent> event = CreateShared<MouseReleaseEvent>((Buttoncode)button);
                    MainWindow& mainWindow = *(MainWindow*)glfwGetWindowUserPointer(window);
                    mainWindow.mApplication->OnEvent(event);
                    break;
                }

                default: { break; }
            }
            });

        glfwSetScrollCallback(mNativeWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
            Shared<MouseWheelEvent> event = CreateShared<MouseWheelEvent>((float)yoffset);
            MainWindow& mainWindow = *(MainWindow*)glfwGetWindowUserPointer(window);
            mainWindow.mApplication->OnEvent(event);
            });

        glfwSetCursorPosCallback(mNativeWindow, [](GLFWwindow* window, double xpos, double ypos) {
            MainWindow& mainWindow = *(MainWindow*)glfwGetWindowUserPointer(window);

            float offSetX = (float)xpos - mainWindow.mLastCursorPosX;
            float offSetY = (float)ypos - mainWindow.mLastCursorPosY;
            mainWindow.mLastCursorPosX = (float)xpos;
            mainWindow.mLastCursorPosY = (float)ypos;

            Shared<MouseMoveEvent> event = CreateShared<MouseMoveEvent>(offSetX, offSetY);
            mainWindow.mApplication->OnEvent(event);
            });

        glfwSetWindowCloseCallback(mNativeWindow, [](GLFWwindow* window) {
            s_Instance->HintQuit(true);

            Shared<WindowCloseEvent> event = CreateShared<WindowCloseEvent>();
            MainWindow& mainWindow = *(MainWindow*)glfwGetWindowUserPointer(window);
            mainWindow.mApplication->OnEvent(event);
            });
    }

    const char** MainWindow::GetInstanceExtensions(unsigned int* count)
    {
#if defined RENDERER_VULKAN
        return glfwGetRequiredInstanceExtensions(count);
#endif
    }

    void MainWindow::GetFrameBufferSize(int* width, int* height)
    {
        glfwGetFramebufferSize(mNativeWindow, width, height);
    }

    void MainWindow::StaleResizeFramebuffer()
    {
        while (glfwGetWindowAttrib(mNativeWindow, GLFW_ICONIFIED))
        {
            glfwWaitEvents();
        }
    }
}