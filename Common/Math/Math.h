#pragma once

#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 26495 26451)
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/epsilon.hpp>

#if defined(_WIN32)
#pragma warning(pop)
#endif

namespace Cosmos
{
	// decomposes a model matrix to translations, rotation and scale components
	bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}