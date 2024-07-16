#pragma once
namespace aim::Components {
	struct Transform3D {
		glm::vec3 pos;
		glm::vec3 scale;

		Transform3D(const glm::vec3& p = glm::vec3(0.0f), const glm::vec3& s = glm::vec3(1.0f))
			: pos(p), scale(s) {}
	};
}
