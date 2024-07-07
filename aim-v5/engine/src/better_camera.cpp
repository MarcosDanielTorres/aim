#include "better_camera.h"

Camera::Camera(CameraType camera_type, glm::vec3 position, float yaw, float pitch)
	: forward(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM)
{
	this->position = position;
	this->pitch = pitch;
	this->yaw = yaw;
	this->camera_type = camera_type;
	update_camera_vectors();
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(position, position + forward, up);
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::process_keyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = movement_speed * deltaTime;
	switch (camera_type) {
	case CameraType::FREE_CAMERA:
		if (direction == FORWARD)
			position += forward * velocity;
		if (direction == BACKWARD)
			position -= forward * velocity;
		if (direction == LEFT)
			position -= right * velocity;
		if (direction == RIGHT)
			position += right * velocity;
		if (direction == UP)
			position += up * velocity;
		if (direction == DOWN)
			position -= up * velocity;
		break;

	case CameraType::FPS_CAMERA:
		if (direction == Camera_Movement::FORWARD)
			position += glm::vec3(forward.x, 0.0, forward.z) * velocity;
		if (direction == Camera_Movement::BACKWARD)
			position -= glm::vec3(forward.x, 0.0, forward.z) * velocity;
		if (direction == LEFT)
			position -= right * velocity;
		if (direction == RIGHT)
			position += right * velocity;
		if (direction == UP)
			position += up * velocity;
		if (direction == DOWN)
			position -= up * velocity;
		break;
	default:
		break;
	}
}

void Camera::render_gui(ImGuiContext* context) {
	ImGui::SetCurrentContext(context);
	if(ImGui::GetCurrentContext() == nullptr) {
		FATAL("DJSADJLASD");
		return;
	}
	if (ImGui::CollapsingHeader("camera transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat3("cam pos", glm::value_ptr(this->position), 0.1f);
		ImGui::DragFloat3("cam forward", glm::value_ptr(this->forward), 0.1f);
		ImGui::DragFloat3("cam up", glm::value_ptr(this->up), 0.1f);
		ImGui::DragFloat3("cam right", glm::value_ptr(this->right), 0.1f);
		ImGui::Spacing();
		ImGui::DragFloat("cam yaw", &this->yaw, 0.1f);
		ImGui::DragFloat("cam pitch", &this->pitch, 0.1f);
		// TODO: Add rotation as well...
	}
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::process_mouse_movement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= mouse_sensitivity;
	yoffset *= mouse_sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	// update Front, Right and Up Vectors using the updated Euler angles
	update_camera_vectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::process_mouse_scroll(float yoffset)
{
	zoom -= (float)yoffset;
	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;
}

// calculates the front vector from the Camera's (updated) Euler Angles
void Camera::update_camera_vectors()
{
	// calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(front);
	// also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(forward, glm::vec3(0.0, 1.0, 0.0)));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, forward));
}
