#include "Camera.h"

#include <Common/Debug/Logger.h>
#include <Platform/Core/Input.h>
#include <Platform/Event/KeyboardEvent.h>
#include <Platform/Event/MouseEvent.h>

namespace Cosmos::Engine
{
	static Camera* s_Instance = nullptr;

	void Camera::Initialize(float aspectRatio)
	{
		if (s_Instance) {
			COSMOS_LOG(Logger::Warn, "Warning: Attempting to initialize Camera when it's already initialized\n");
			return;
		}

		s_Instance = new Camera();

		// set initial aspect ratio
		s_Instance->SetAspectRatio(aspectRatio);

		// update initial position
		s_Instance->UpdateViewMatrix();
	}

	void Camera::Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	Camera& Camera::GetRef()
	{
		if (!s_Instance) {
			COSMOS_LOG(Logger::Error, "Camera has not been initialized\n");
		}

		return *s_Instance;
	}

	void Camera::SetAspectRatio(float aspect)
	{
		glm::mat4 currentMatrix = mPerspective;
		mPerspective = glm::perspective(glm::radians(mFov), aspect, mZnear, mZfar);
		mPerspective[1][1] *= -1.0f;
		mAspectRatio = aspect;
	}

	glm::mat4& Camera::GetProjectionRef()
	{
		mPerspective = glm::perspectiveRH(glm::radians(mFov), mAspectRatio, mZnear, mZfar);

		if (mFlipY) mPerspective[1][1] *= -1.0f;

		return mPerspective;
	}

	glm::mat4& Camera::GetViewRef()
	{
		return mView;
	}

	void Camera::OnUpdate(double timestep)
	{
		if (!mShouldMove) return;

		mFront.x = -cos(glm::radians(mRotation.x)) * sin(glm::radians(mRotation.y));
		mFront.y = sin(glm::radians(mRotation.x));
		mFront.z = cos(glm::radians(mRotation.x)) * cos(glm::radians(mRotation.y));

		float moveSpeed = (float)timestep * mMovementSpeed * (mShiftPressed ? 2.5f : 1.0f);

		if (mMovingForward) mPosition += mFront * moveSpeed;
		if (mMovingBackward) mPosition -= mFront * moveSpeed;
		if (mMovingLeft) mPosition -= glm::normalize(glm::cross(mFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
		if (mMovingRight) mPosition += glm::normalize(glm::cross(mFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

		UpdateViewMatrix();
	}

	void Camera::OnEvent(Shared<Platform::EventBase> event)
	{
		switch (event->GetType())
		{
			case Platform::EventType::MouseMove:
			{
				if (!mShouldMove) return;

				// avoid scene flip
				if (mRotation.x >= 89.0f) mRotation.x = 89.0f;
				if (mRotation.x <= -89.0f) mRotation.x = -89.0f;
				
				// reset rotation on 360 degrees
				if (mRotation.x >= 360.0f) mRotation.x = 0.0f;
				if (mRotation.x <= -360.0f) mRotation.x = 0.0f;
				if (mRotation.y >= 360.0f) mRotation.y = 0.0f;
				if (mRotation.y <= -360.0f) mRotation.y = 0.0f;

				float x = (float)std::dynamic_pointer_cast<Platform::MouseMoveEvent>(event)->GetXOffset();
				float y = (float)std::dynamic_pointer_cast<Platform::MouseMoveEvent>(event)->GetYOffset();

				Rotate(glm::vec3(-y * mRotationSpeed * 0.5f, x * mRotationSpeed * 0.5f, 0.0f));
				break;
			}

			case Platform::EventType::MouseWheel:
			{
				if (!mShouldMove) return;

				float delta = (float)std::dynamic_pointer_cast<Platform::MouseWheelEvent>(event)->GetDelta();

				mPosition += delta * mMovementSpeed;
				break;
			}

			case Platform::EventType::KeyboardPress:
			{
				auto castedEvent = std::dynamic_pointer_cast<Platform::KeyboardPressEvent>(event);
				Platform::Keycode key = castedEvent->GetKeycode();

				if (key == Platform::KEY_W && mShouldMove) mMovingForward = true;
				if (key == Platform::KEY_S && mShouldMove) mMovingBackward = true;
				if (key == Platform::KEY_A && mShouldMove) mMovingLeft = true;
				if (key == Platform::KEY_D && mShouldMove) mMovingRight = true;
				if (key == Platform::KEY_LEFT_SHIFT && mShouldMove) mShiftPressed = true;

				break;
			}

			case Platform::EventType::KeyboardRelease:
			{
				auto castedEvent = std::dynamic_pointer_cast<Platform::KeyboardReleaseEvent>(event);
				Platform::Keycode key = castedEvent->GetKeycode();

				if (key == Platform::KEY_W && mShouldMove) mMovingForward = false;
				if (key == Platform::KEY_S && mShouldMove) mMovingBackward = false;
				if (key == Platform::KEY_A && mShouldMove) mMovingLeft = false;
				if (key == Platform::KEY_D && mShouldMove) mMovingRight = false;
				if (key == Platform::KEY_LEFT_SHIFT && mShouldMove) mShiftPressed = false;

				break;
			}

			default: break;
		}
	}

	void Camera::UpdateViewMatrix()
	{
		glm::mat4 rotMat = glm::mat4(1.0f);
		glm::mat4 translateMat = glm::mat4(1.0f);
		glm::mat4 scaleMat = glm::mat4(1.0f);

		rotMat = glm::rotate(rotMat, glm::radians(mRotation.x * (mFlipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		rotMat = glm::rotate(rotMat, glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotMat = glm::rotate(rotMat, glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 translation = mPosition;
		glm::vec3 scale = mScale;

		if (mFlipY) translation.y *= -1.0f;

		translateMat = glm::translate(glm::mat4(1.0f), translation);
		scaleMat = glm::scale(glm::mat4(1.0f), scale);

		if (mType == CameraType::FreeLook) mView = scaleMat * rotMat * translateMat;
		else mView = scaleMat * translateMat * rotMat;

		mViewPos = glm::vec4(mPosition, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
	}

	void Camera::Translate(glm::vec3 delta)
	{
		mPosition += delta;
		UpdateViewMatrix();
	}

	void Camera::Rotate(glm::vec3 delta)
	{
		mRotation += delta;
		UpdateViewMatrix();
	}
}