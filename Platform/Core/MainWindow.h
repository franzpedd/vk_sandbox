#pragma once

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

	public: // vulkan

		// returns the instance extensions used by the windows
		const char** GetInstanceExtensions(unsigned int* count);

		// returns the framebuffer size
		void GetFrameBufferSize(int* width, int* height);

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