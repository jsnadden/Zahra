#include "zpch.h"
#include "Maths.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Zahra::Maths
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& eulers, glm::vec3& scale)
	{
		glm::mat4 Matrix(transform);

		// normalise the matrix (i.e. make sure final row is (0,0,0,1), as expected)
		{
			if (glm::epsilonEqual(Matrix[3][3], static_cast<float>(0), glm::epsilon<float>())) return false;

			if (glm::epsilonNotEqual(Matrix[0][3], static_cast<float>(0), glm::epsilon<float>()) ||
				glm::epsilonNotEqual(Matrix[1][3], static_cast<float>(0), glm::epsilon<float>()) ||
				glm::epsilonNotEqual(Matrix[2][3], static_cast<float>(0), glm::epsilon<float>()))
			{
				Matrix[0][3] = Matrix[1][3] = Matrix[2][3] = static_cast<float>(0);
				Matrix[3][3] = static_cast<float>(1);
			}
		}

		// extract translation vector
		{
			translation = glm::vec3(Matrix[3]);
			Matrix[3] = glm::vec4(0, 0, 0, Matrix[3].w);
		}

		// get 3x3 part
		glm::vec3 Minor[3], cross;
		for (glm::length_t i = 0; i < 3; i++)
			for (glm::length_t j = 0; j < 3; j++)
				Minor[i][j] = Matrix[i][j];

		// extract scale
		{
			scale.x = glm::length(Minor[0]);
			Minor[0] = glm::detail::scale(Minor[0], static_cast<float>(1));
			scale.y = glm::length(Minor[1]);
			Minor[1] = glm::detail::scale(Minor[1], static_cast<float>(1));
			scale.z = glm::length(Minor[2]);
			Minor[2] = glm::detail::scale(Minor[2], static_cast<float>(1));
		}

		// the 3x3 minor should now be an orthonormal frame, but we still need to account for its chirality:
		cross = glm::cross(Minor[1], Minor[2]);
		if (glm::dot(Minor[0], cross) < 0) // triple product
		{
			for (glm::length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<float>(-1);
				Minor[i] *= static_cast<float>(-1);
			}
		}

		eulers.y = glm::asin(-Minor[0][2]);
		if (glm::cos(eulers.y) != 0)
		{
			eulers.x = glm::atan(Minor[1][2], Minor[2][2]);
			eulers.z = glm::atan(Minor[0][1], Minor[0][0]);
		}
		else
		{
			eulers.x = glm::atan(-Minor[2][0], Minor[1][1]);
			eulers.z = 0; // wlog, thanks to gimbal lock
		}

		return true;

	}

}



