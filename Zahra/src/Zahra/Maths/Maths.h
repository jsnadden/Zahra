#pragma once

#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

namespace Zahra
{
	namespace Maths
	{
		bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& eulers, glm::vec3& scale);
	}
}
