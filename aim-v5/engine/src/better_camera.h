#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "defines.h"
#include "core/logger/logger.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

enum CameraType {
	FREE_CAMERA,
	FPS_CAMERA,
};

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
};

struct Camera {
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;

	CameraType camera_type;

	float pitch, yaw;
	float movement_speed;
	float mouse_sensitivity;
	float zoom;

	// constructor with vectors
	AIM_API Camera(CameraType camera_type, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 AIM_API GetViewMatrix();
	void AIM_API render_gui(ImGuiContext* context);

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void AIM_API process_keyboard(Camera_Movement direction, float deltaTime);

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void AIM_API process_mouse_movement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void AIM_API process_mouse_scroll(float yoffset);

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void update_camera_vectors();
};