#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace aim::Components {
	const float deg_45 = 0.785398f; // 45
	const float deg_90 = 1.5708f; // 45
	const float deg_120 = 2.0944f; // 45

	struct Transform3D {
		glm::vec3 pos;
		glm::vec3 scale;

		glm::vec3 eulerAngles;
		glm::quat rot;

		Transform3D(const glm::vec3& p = glm::vec3(0.0f), const glm::vec3& s = glm::vec3(1.0f), const glm::vec3& e = glm::vec3(0.0f))
			: pos(p), scale(s), eulerAngles(e) {
			this->update_rotation();
		}

		void update_rotation() {
			glm::quat qx = glm::angleAxis(glm::radians(this->eulerAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat qy = glm::angleAxis(glm::radians(this->eulerAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat qz = glm::angleAxis(glm::radians(this->eulerAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
			rot = qz * qy * qx; // Specify order of rotations here
		}
	};
}
