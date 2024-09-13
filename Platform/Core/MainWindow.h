#pragma once

// forward declarations
struct SDL_Window;
struct SDL_Surface;
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
		inline SDL_Window* GetNativeWindow() { return mNativeWindow; }

		// sets the should quit variable to a value
		inline void HintQuit(bool value) { mShouldQuit = value; }

		// returns if window close event was called
		inline bool ShouldQuit() { return mShouldQuit; }

		// sets the should resize variable to a value
		inline void HintResize(bool value) { mShouldResizeWindow = value; }

		// returns if window resize event was called
		inline bool ShouldResize() { return mShouldResizeWindow; }

	public:

		// updates the window input events
		void OnUpdate();

		// enables or disables the cursor
		void ToggleCursor(bool hide);

		// returns the current window aspect ratio
		float GetAspectRatio();

		// gets the relative position with the monitor 
		void GetRelativePosition(int* x, int* y);

	public: // vulkan

		// returns the instance extensions used by the windows
		void GetInstanceExtensions(unsigned int* count, const char** names);

		// creates a window surface for drawing into it
		void CreateSurface(void* instance, void** surface);

		// returns the framebuffer size
		void GetFrameBufferSize(int* width, int* height);

		// recreates the window
		void StaleResizeFramebuffer();

	private:

		Engine::Application* mApplication;
		SDL_Window* mNativeWindow = nullptr;
		SDL_Surface* mIconSurface = nullptr;
		bool mShouldQuit = false;
		bool mShouldResizeWindow = false;
	};
}