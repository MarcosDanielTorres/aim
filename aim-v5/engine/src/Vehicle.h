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
		glm::vec3 pos;
		glm::vec3 center_of_mass;
		bool rear_wheeled;
		
		int model_index;
		int collider_model_index;

		void create_vehicle(PhysicsSystem& physics_system) {
				
		}
	};
}