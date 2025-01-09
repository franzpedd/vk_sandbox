#pragma once

#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 26495 26451)
#endif

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_inverse.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/intersect.hpp>
#include <gtx/string_cast.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/matrix_decompose.hpp>
#include <gtc/epsilon.hpp>

#if defined(_WIN32)
#pragma warning(pop)
#endif

#define MACRO_MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MACRO_MIN(a,b)  (((a) < (b)) ? (a) : (b))

namespace Cosmos
{
	// decomposes a model matrix to translations, rotation and scale components
	bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

	// returns the average radius of a given object
	float GetAverageRadius(const glm::mat4& transform, const glm::vec3& scale);

	// casts a ray given general window properties
	void RayCast(glm::vec2 mousePos, float winWidth, float winHeight, const glm::mat4& projection, const glm::mat4& view, glm::vec3& origin, glm::vec3& dir);

	// casts a ray given viewport with custom sizes
	void RayCastViewport(glm::vec2 mousePos, glm::vec2 viewportSize, glm::vec2 viewportOffset, float winWidth, float winHeight, const glm::mat4& projection, const glm::mat4& view, glm::vec3& origin, glm::vec3& dir);

	// tests if a ray can eventually intersects with an object given object and ray properties
	bool RayCollide(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const glm::mat4& modelMatrix, float& distance);

	// returns if a ray collides with an object
	bool RayCollide2(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& objectPos, float maxCollisionDistance);

	// returns if there's a collision with an object along a line segment
	bool RayCollide3(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& objectPos, float maxCollisionDistance);

	// returns if a ray collides with an sphere radius
	bool RaySphereCollide(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& sphereCenter, float sphereRadius, float& t);

	// returns if a ray collides with an axis-aligned bounding box
	bool RayAABBCollide(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& tMin);
}