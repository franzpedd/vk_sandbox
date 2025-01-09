#pragma once

#include <Common/Math/Math.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Platform { class EventBase; }

namespace Cosmos::Engine
{
	enum CameraType : unsigned int
	{
		LookAt = 0,
		FreeLook
	};

	class Camera
	{
	public:

		// delete copy constructor
		Camera(const Camera&) = delete;

		// delete assignment constructor
		Camera& operator=(const Camera&) = delete;

	public:

		// initializes the main window
		static void Initialize(float aspectRatio);

		// terminates the main window
		static void Shutdown();

		// returns main window singleton instance
		static Camera& GetRef();

	private:

		// constructor
		Camera() = default;

		// destructor
		~Camera() = default;
		
	public:

		// enables/disables the camera movement
		inline void SetMove(bool value) { mShouldMove = value; }

		// query if camera movement is enabled
		inline bool CanMove() const { return mShouldMove; }

		// returns camera's mode
		inline CameraType GetType() const { return mType; }

		// sets a new camera's mode
		inline void SetType(CameraType type) { mType = type; }

		// sets a new aspect ratio for the camera
		void SetAspectRatio(float aspect);

		// returns the camera's aspect ratio
		inline float GetAspectRatio() const { return mAspectRatio; }

		// returns the camera field of view
		inline float GetFov() const { return mFov; }

		// returns near value
		inline float GetNear() const { return mZnear; }

		// returns far value
		inline float GetFar() const { return mZfar; }

		// returns the current camera rotation
		inline glm::vec3& GetRotationRef() { return mRotation; }

		// returns the view position correctly (X and Z are right)
		inline glm::vec3 GetViewPos() { return mViewPos; }

		// returns the camera front vector
		inline glm::vec3 GetFront() { return mFront; }

	public:

		// returns a reference to the perspective/projection matrix, using defaults aspect ratio
		glm::mat4& GetProjectionRef();

		// returns a copy of the projection matrix
		const glm::mat4 GetProjection();

		// returns a reference to the view matrix
		glm::mat4& GetViewRef();

		// returns a copy of the view matrix
		const glm::mat4 GetView();
	public:

		// updates the camera
		void OnUpdate(float timestep);

		// event handling
		void OnEvent(Shared<Platform::EventBase> event);

	private:

		// updates the camera's view
		void UpdateViewMatrix();

		// translates the camera
		void Translate(glm::vec3 delta);

		// rotates the camera
		void Rotate(glm::vec3 delta);

	private:

		CameraType mType = CameraType::FreeLook;
		bool mFlipY = true;
		bool mShouldMove = false;
		float mPitch = 0.0f;
		float mFov = 45.0f;
		float mZnear = 0.1f;
		float mZfar = 256.0f;
		float mMovementSpeed = 1.0f;
		float mRotationSpeed = 1.0f;
		float mAspectRatio = 1.0f;

		glm::mat4 mPerspective = glm::mat4(1.0f);
		glm::mat4 mView = glm::mat4(1.0f);

		glm::vec3 mRotation = glm::vec3(0.0f);
		glm::vec3 mPosition = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 mScale = glm::vec3(1.0f);
		glm::vec3 mViewPos = glm::vec4(0.0f);
		glm::vec3 mFront = glm::vec3(0.0f, 0.0f, -1.0f);

		// W A S D
		bool mShiftPressed = false;
		bool mMovingForward = false;
		bool mMovingBackward = false;
		bool mMovingLeft = false;
		bool mMovingRight = false;
	};
}