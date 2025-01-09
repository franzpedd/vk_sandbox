#pragma once

#include <Common/Math/Math.h>
#include "Input.h"

// forward declarations
struct GLFWwindow;
namespace Cosmos::Platform { class EventBase; }
namespace Cosmos::Engine { class Application; }

namespace Cosmos::Platform
{
	class MainWindow
	{
	public:

		// delete copy constructor
		MainWindow(const MainWindow&) = delete;

		// delete assignment constructor
		MainWindow& operator=(const MainWindow&) = delete;

	public:

		// initializes the main window
		static void Initialize(Engine::Application* application, const char* title, int width, int height, bool fullScreen = true);

		// terminates the main window
		static void Shutdown();

		// returns main window singleton instance
		static MainWindow& GetRef();

	private:

		// constructor
		MainWindow(Engine::Application* application);

		// destructor
		~MainWindow();

	public:

		// returns the pointer to the native window
		inline GLFWwindow* GetNativeWindow() { return mNativeWindow; }

		// sets the should resize variable to a value
		inline void HintResize(bool value) { mShouldResizeWindow = value; }

		// returns if window resize event was called
		inline bool ShouldResize() { return mShouldResizeWindow; }

		// sets the should resize variable to a value
		inline void HintQuit(bool value) { mShouldQuit = value; }

		// returns if window close event was called
		inline bool ShouldQuit() { return mShouldQuit; }

	public:

		// updates the window input events
		void OnUpdate();

		// enables or disables the cursor
		void ToggleCursor(bool hide);

		// returns the current window aspect ratio
		float GetAspectRatio();

		// sets the callbacks from glfw
		void SetCallbacks();

		// returns if key is currently pressed
		bool IsKeyDown(Keycode key);

	public: // vulkan

		// returns the instance extensions used by the windows
		const char** GetInstanceExtensions(unsigned int* count);

		// returns the framebuffer size
		glm::vec2 GetFrameBufferSize();

		// returns the window size
		glm::vec2 GetWindowSize();

		// returns the cursor current position
		glm::vec2 GetCursorPos();

		// returns the cursor current position
		//glm::vec2 GetViewportCursorPos(glm::vec2 viewportPos, glm::vec2 viewportSize);

		// returns the relative cursor position if using a smaller viewport somewhere on the screen
		glm::vec2 GetViewportCursorPos(const glm::vec2& viewportPosition, const glm::vec2& viewportSize);

		// recreates the window
		void StaleResizeFramebuffer();

	private:

		Engine::Application* mApplication;
		GLFWwindow* mNativeWindow = nullptr;
		bool mShouldQuit = false;
		bool mShouldResizeWindow = false;
		float mLastCursorPosX = 0.0f;
		float mLastCursorPosY = 0.0f;
	};
}