#include "MathUtils.h"
#include <cstdlib>

glm::vec3 MathUtils::GetRandomVector3( float min, float max ) {
	return glm::vec3(
		min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min))),
		min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min))),
		min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)))
	);
}