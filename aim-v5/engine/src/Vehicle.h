#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <string>

#include "AssimpLoader.h"
#include "PhysicsSystem.h"

namespace Vehicle {

	struct Vehicle {
		std::string name{ "None" };
		glm::vec3 pos{};
		glm::vec3 center_of_mass{};
		bool rear_wheeled = false;

		int model_index{ -1 };
		int collider_model_index{ -1 };

		void create_vehicle(std::string name, glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0), int model_index, int collider_model_index, PhysicsSystem& physics_system) {
			
		}
	};
}