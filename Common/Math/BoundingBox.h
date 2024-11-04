#pragma once

#include "Math.h"

namespace Cosmos
{
	class BoundingBox
	{
	public:

		// constructor
		BoundingBox(glm::vec3 min = glm::vec3(0.0f), glm::vec3 max = glm::vec3(1.0f));

		// destructor
		~BoundingBox() = default;

	public:

		// returns bb's max value
		inline glm::vec3 GetMax() const { return mMax; }

		// sets a new max value
		inline void SetMax(glm::vec3 max) { mMax = max; }

		// returns bb's min value
		inline glm::vec3 GetMin() const { return mMin; }

		// sets a new min value
		inline void SetMin(glm::vec3 min) { mMin = min; }

		// returns if bounding box was validated
		inline bool IsValid() const { return mValidated; }

		// sets the validation status of the bounding box
		inline void SetValid(bool value) { mValidated = value; }

	public:

		// calculates and returns the axis-align bounding box of this bounding box
		BoundingBox GetAABB(glm::mat4 mat);

	private:

		glm::vec3 mMin = glm::vec3(0.0f);
		glm::vec3 mMax = glm::vec3(1.0f);
		bool mValidated = false;
	};
}