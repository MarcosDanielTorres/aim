// esto es importante de que venga de un include directory externo asi lo puedo leer con <> sino se rompe todo
// IMPORTANTE: Si no se mete glad arriba del todo se ROMPE es una pelotudez por dios
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "game_types.h"
//#include "application.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
// este NO anda
//#include <core/logger/logger.h>
// este SI anda
#include "core/logger/logger.h"

int armature_count = 0;
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// imgui includes
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <thread>
#include <cstdarg>

#define PART_11
#define MAX_NUM_JOINTS 320u
#define MAX_NUM_BONES_PER_VERTEX 4

void print_matrix(const glm::mat4& mat);
JPH_SUPPRESS_WARNINGS
#include "PhysicsSystem.h"
//#include "jolt_debug_renderer.h"

glm::mat4 assault_rifle_transform{};

glm::mat4 all_mag_transforms;
glm::mat4 skel_assault_rifle_transform;
glm::mat4 mag_transform;
glm::mat4 grip_transform;
glm::mat4 armature_transform;
glm::mat4 manny_armature;
glm::mat4 mag_bone_transform;
glm::mat4 ik_something;
std::vector<glm::mat4> manny_transforms;
std::vector<glm::mat4> mag_transforms;
glm::mat4 manny_world_transform{};
#include "better_camera.h"
#include "learnopengl/shader_m.h"
#include "Model.h"

// check this
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb_image.h>
//#include <stb_image_write.h>
#include "tiny_gltf.h"
// check this

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

int skinning_shader_id = -1;

static bool gui_mode = false;
static bool wireframe_mode = false;
static bool physics_mode = false;
static bool fps_mode = false;
static float gravity = 2.2;


// singleton

// Define static members
//PhysicsSystem* PhysicsSystem::instance = nullptr;
//std::once_flag PhysicsSystem::initInstanceFlag;

struct Context {
	PhysicsSystem* physics_system;
};
// singleton


struct MeshBox;
bool r_pressed_in_last_frame = false;
bool three_pressed_last_frame = false;

#pragma region helpers

namespace AssimpGLMHelpers {
	static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
	{
		glm::mat4 to{};
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
	{
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}
}

#pragma endregion helpers



int boneCount = 0;
int display_bone_index = 1;
unsigned int skel_id = -1;

std::vector<MeshBox> projectiles;

// Bounding boxes de los cubos tienen de ancho 1, desde el medio 0.5 en todas las direcciones. 
// The same as Godot
// Unreal Engine 
// Unity unknown
/*
	Collision check:
	When I press `R` cast ray.
	Cast one ray in the direction of cam.pos + t * cam.forward and increment t each time in a loop
	For every iteration go over all cubes positions at `cubePosition[i]` and see if `ro + t * rd` is inside a bounding box
	calculated on the fly:
		vec3 ray_pos = `ro + t * rd`;
		float bounding_box[6] = {
			cubePositions[i].x - 0.5 : cubePositions[i].x + 0.5
			cubePositions[i].y - 0.5 : cubePositions[i].y + 0.5
			cubePositions[i].z - 0.5 : cubePositions[i].z + 0.5
		};
		if (ray_pos.x >= bounding_box[0] && ray_pos.x <= bounding_box[1] &&
			ray_pos.x >= bounding_box[2] && ray_pos.x <= bounding_box[3] &&
			ray_pos.x >= bounding_box[4] && ray_pos.x <= bounding_box[5])

*/

/* Tickets
	functional:
	- Floor physics:
		- collision pos and cube pos move together DONE
		- collision pos and cube scale move together DONE
		- create falling cubes without rotation (they fall uniformly or whatever they call it, basically axis aligned?? ja and see how they interact with the static floor
		- add rotation to these cubes

	- Crear `CubeMesh` o `MeshCube`, ver Godot
	- Agregarle una CollisionShape a los meshes
	- Ver por que no puedo tomar los header files del proyecto e incluirlos con <>
	- Ver si se puede meter el `glad.c` de alguna forma automatica. Ya que va a ir en todos los ejecutables.

	- Usar curr_camera en todos lados. Solo esta en el `set_debug_camera()` del `physics_system`

	- Reemplazar el floor actual por un objeto que renderice y tenga fisica.
	- toggle between wireframe and physics. physics in green
	- StaticBody, DynamicBody, KinematicBody
	- Shape

	non-functional:
	- Meter im3d
	- Hacer algo con el current camera

	CMake:
		- Quiero poder agregar un puto archivo desde aca dentro y no tener que correr la mierda de CMake otra vez.
			Es tremenda pelotudez. Tengo que ver como poner la integracion.
		- Meter glad en thirdparty
		- Meter la solution `imgui` y `glfw` en un folder que diga `thirdparty`

	OpenGL:
		- Anotar NDC de Vulkan y de Opengl y capaz de WGPU tambien.

SUSPICIOUS THINGS:
	- I removed `glad.c` from the engine. It's only needed on `Sandbox`.

DONE:
	- Sacar el puto physics system de aca DONE
	- The cube is not a cube... but it may be something related to perspective because the physics engine its able to map it correctly...
		So this means the error is about perspective DONE: it was indeed a difference between the window size and the perspective size, they weren't using
		the same values.

	- Renderizar un triangulo al menos
	CMake:
		- Consumir glad desde el engine y desde sandbox.
			- Hacerlo para Debug y Release.
*/



#include "components.h"
using namespace aim::Components;

struct RayCast {
	glm::vec3 ro;
	glm::vec3 rd;
	float t;
};

std::vector<RayCast> ray_cast_list{};


enum PhysicsBodyType {
	STATIC,
	DYNAMIC,
	KINEMATIC,
	EMPTY,
};

struct PhysicsBody {
	JPH::Shape* shape;
	PhysicsBodyType physics_body_type;
	JPH::BodyID physics_body_id;

	//PhysicsBody(JPH::Ref<JPH::Shape> ashape = nullptr, PhysicsBodyType type = PhysicsBodyType::EMPTY) : physics_body_type(type) {
	//	shape = ashape;
	//}

	//PhysicsBody(PhysicsSystem& physics_system, JPH::Ref<JPH::Shape> shape, PhysicsBodyType type) : physics_body_type(type) {
	//	bool is_static = 	
	//}

};

//cube = MeshBox();
//cube.set_shape();
//cube.set_motion_type();
//cube.set_layer();
struct MeshBox {
	Transform3D transform;
	PhysicsBody body;

	MeshBox(Transform3D transform) {
		this->transform = transform;
	}

	// this is not the best api, its only used once when there is no body. Only done in creation
	// This api is more like a builder style api, its probably better to have it inside the constructor and that's it or use a builder for this shapes. which could be good.
	void set_shape(JPH::Ref<JPH::Shape> shape) {
		this->body.shape = shape;
	}

	void update_body_shape(PhysicsSystem& physics_system) {
		JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(this->transform.scale.x / 2.0, this->transform.scale.y / 2.0, this->transform.scale.z / 2.0));
		floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
		JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		JPH::Ref<JPH::Shape> floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
		this->body.shape = floor_shape;
		physics_system.get_body_interface().SetShape(this->body.physics_body_id, this->body.shape, false, JPH::EActivation::DontActivate);

		glm::vec3 my_pos = this->transform.pos;
		JPH::Vec3 new_pos = JPH::Vec3(my_pos.x, my_pos.y, my_pos.z);
		physics_system.get_body_interface().SetPosition(this->body.physics_body_id, new_pos, JPH::EActivation::DontActivate);

		physics_system.get_body_interface().SetRotation(
			this->body.physics_body_id,
			JPH::Quat(this->transform.rot.x, this->transform.rot.y, this->transform.rot.z, this->transform.rot.w),
			JPH::EActivation::DontActivate
		);
	}
};


struct PointLight {
	Transform3D transform;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	Transform3D transform;
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
};

struct DirectionalLight {
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

/*
bevy
struct PbrBundle {
	Mesh;
	Material {.base_color, ...};
};

*/



// settings
const unsigned int SRC_WIDTH = 1280;
const unsigned int SRC_HEIGHT = 720;
//const unsigned int SRC_WIDTH = 1920;
//const unsigned int SRC_HEIGHT = 1080;

// camera
Camera free_camera(FREE_CAMERA, glm::vec3(0.0f, 5.0f, 19.0f));
Camera fps_camera(FPS_CAMERA, glm::vec3(0.0f, 8.0f, 3.0f));
// TODO: hacer esto...
Camera& curr_camera = free_camera;

float lastX = SRC_WIDTH / 2.0f;
float lastY = SRC_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
glm::vec3 light_dir(-0.2f, -1.0f, -0.3f);
glm::vec3 light_scale(0.2f);


glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse(0.5f, 0.5f, 0.5f); // darken diffuse light a bit
glm::vec3 light_specular(1.0f, 1.0f, 1.0f);



// rifle
glm::vec3 rifle_pos(-2.2f, 0.0f, 0.0f);
glm::vec3 rifle_rot(0.0f, 0.0f, 0.0f);
float rifle_scale(0.5f);

// manny
glm::vec3 manny_pos(-2.2f, 0.0f, 0.0f);
glm::vec3 manny_rot(0.0f, 0.0f, 0.0f);
float manny_scale(0.5f);

// model
glm::vec3 floor_pos(0.0f, 0.0f, -2.2f);
glm::vec3 floor_scale(30.0f, 1.0f, 30.0f);
glm::vec3 model_bounding_box(floor_scale * 1.0f); // este esta "mal" aca la camara tiene cierta altura pero no es la colision posta con el piso.
//glm::vec3 model_bounding_box(model_scale / 2.0f); // esta seria la correcta pasa que ahi me queda el centro de la camara donde termina el piso entonces queda la mitad
// dentro del piso y la mitad arriba.

//glm::vec3 model_scale(0.5f, 5.0f, 1.0f);

// model materials
glm::vec3 model_material_ambient(1.0f, 0.5f, 0.31f);
glm::vec3 model_material_diffuse(1.0f, 0.5f, 0.31f);
glm::vec3 model_material_specular(0.5f, 0.5f, 0.5f);
float model_material_shininess(32.0f);




// agregar fps done
// agregar toggle between `camera` and `fps_camera` a imgui done
// agregar wireframe rapido done
// ver rapidisimo a la velocidad del diablo a donde mierda esta la cabeza de la fps camera done

extern bool create_game(game* game_inst);


unsigned int compile_shaders(const char* vertPath, const char* fragPath);

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";



void createTransformsUBO(const std::vector<glm::mat4>& transforms, unsigned int ubo) {
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glm::mat4* ptr = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * 10000, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	if (ptr) {

		for (size_t i = 0; i < 10000; i++) {
			ptr[i] = transforms[i];
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
}



struct TestingRenderer {
	const char* vert_shader_source;
	const char* frag_shader_source;
	unsigned int shader_id;

	TestingRenderer(const char* vert_shader_source, const char* frag_shader_source) : vert_shader_source(vert_shader_source), frag_shader_source(frag_shader_source) {
		compile_shaders();
	}


	void render();

	void compile_shaders() {
		// build and compile our shader program
		// ------------------------------------
		// vertex shader
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vert_shader_source, NULL);
		glCompileShader(vertexShader);
		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// fragment shader
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &frag_shader_source, NULL);
		glCompileShader(fragmentShader);
		// check for shader compile errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			FATAL("SHADER::FRAGMENT::COMPILATION_FAILED: ");
			FATAL("%s", infoLog);
			abort();
		}
		// link shaders
		shader_id = glCreateProgram();
		glAttachShader(shader_id, vertexShader);
		glAttachShader(shader_id, fragmentShader);
		glLinkProgram(shader_id);
		// check for linking errors
		glGetProgramiv(shader_id, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader_id, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			abort();
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

};

void update_physics(float delta_time) {
	if (fps_mode) {
		// check collision
		if (model_bounding_box.y >= fps_camera.position.y) {
			fps_camera.position.y = model_bounding_box.y;
			std::cout << "Colission detected at: " << model_bounding_box.y << std::endl;
		}
		else {
			fps_camera.position.y -= gravity * delta_time;
		}
	}
}

struct GLTFMesh {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	size_t indexCount;
	uint32_t first_index;
	GLuint materialId;
};

struct Mesh {
	std::vector<GLTFMesh*> primitives;
	struct UniformBlock {
		glm::mat4 matrix;
		glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
		uint32_t jointCount{ 0 };
	} uniformBlock;
};

struct GLTFSkin;

struct GLTFNode {
	GLTFNode* parent;
	std::string name;
	std::vector<GLTFNode*> children;
	uint32_t index;
	GLTFSkin* skin;
	int32_t skinIndex{ -1 };
	Mesh* mesh;
	glm::mat4 local_matrix;
	glm::vec3           translation{};
	glm::vec3           scale{ 1.0f };
	glm::quat           rotation{};

	void update();

	glm::mat4 getLocalMatrix()
	{
		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * local_matrix;
	}

	glm::mat4 getMatrix() {
		glm::mat4 node_matrix = getLocalMatrix();
		GLTFNode* curr_parent = parent;

		while (curr_parent) {
			//print_matrix(curr_parent->getLocalMatrix());
			//print_matrix(node_matrix);
			node_matrix = curr_parent->getLocalMatrix() * node_matrix;
			curr_parent = curr_parent->parent;
		}

		return node_matrix;
	}
};


struct GLTFSkin {
	std::string            name;
	GLTFNode* skeletonRoot = nullptr;
	std::vector<glm::mat4> inverseBindMatrices;
	std::vector<GLTFNode*>    joints;
	//vks::Buffer            ssbo;
	//VkDescriptorSet        descriptorSet;
};


// Animation related
struct AnimationSampler {
	enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
	InterpolationType interpolation;
	std::vector<float>     inputs;
	std::vector<glm::vec4> outputsVec4;
	std::vector<float> outputs;
	glm::vec4 cubicSplineInterpolation(size_t index, float time, uint32_t stride);
	void translate(size_t index, float time, GLTFNode* node);
	void scale(size_t index, float time, GLTFNode* node);
	void rotate(size_t index, float time, GLTFNode* node);
};

// Cube spline interpolation function used for translate/scale/rotate with cubic spline animation samples
// Details on how this works can be found in the specs https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
glm::vec4 AnimationSampler::cubicSplineInterpolation(size_t index, float time, uint32_t stride) {
	float delta = inputs[index + 1] - inputs[index];
	float t = (time - inputs[index]) / delta;
	const size_t current = index * stride * 3;
	const size_t next = (index + 1) * stride * 3;
	const size_t A = 0;
	const size_t V = stride * 1;
	const size_t B = stride * 2;

	float t2 = powf(t, 2);
	float t3 = powf(t, 3);
	glm::vec4 pt{ 0.0f };
	for (uint32_t i = 0; i < stride; i++) {
		float p0 = outputs[current + i + V];			// starting point at t = 0
		float m0 = delta * outputs[current + i + A];	// scaled starting tangent at t = 0
		float p1 = outputs[next + i + V];				// ending point at t = 1
		float m1 = delta * outputs[next + i + B];		// scaled ending tangent at t = 1
		pt[i] = ((2.f * t3 - 3.f * t2 + 1.f) * p0) + ((t3 - 2.f * t2 + t) * m0) + ((-2.f * t3 + 3.f * t2) * p1) + ((t3 - t2) * m0);
	}
	return pt;
}

// Calculates the translation of this sampler for the given node at a given time point depending on the interpolation type
	// esto parece que es t? float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
void AnimationSampler::translate(size_t index, float time, GLTFNode* node) {
	switch (interpolation) {
	case AnimationSampler::InterpolationType::LINEAR: {
		float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
		node->translation = glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
		break;
	}
	case AnimationSampler::InterpolationType::STEP: {
		node->translation = outputsVec4[index];
		break;
	}
	case AnimationSampler::InterpolationType::CUBICSPLINE: {
		node->translation = cubicSplineInterpolation(index, time, 3);
		break;
	}
	}
}

// Calculates the scale of this sampler for the given node at a given time point depending on the interpolation type
void AnimationSampler::scale(size_t index, float time, GLTFNode* node) {
	switch (interpolation) {
	case AnimationSampler::InterpolationType::LINEAR: {
		float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
		node->scale = glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
		break;
	}
	case AnimationSampler::InterpolationType::STEP: {
		node->scale = outputsVec4[index];
		break;
	}
	case AnimationSampler::InterpolationType::CUBICSPLINE: {
		node->scale = cubicSplineInterpolation(index, time, 3);
		break;
	}
	}
}

// Calculates the rotation of this sampler for the given node at a given time point depending on the interpolation type
void AnimationSampler::rotate(size_t index, float time, GLTFNode* node) {
	switch (interpolation) {
	case AnimationSampler::InterpolationType::LINEAR: {
		float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
		glm::quat q1;
		q1.x = outputsVec4[index].x;
		q1.y = outputsVec4[index].y;
		q1.z = outputsVec4[index].z;
		q1.w = outputsVec4[index].w;
		glm::quat q2;
		q2.x = outputsVec4[index + 1].x;
		q2.y = outputsVec4[index + 1].y;
		q2.z = outputsVec4[index + 1].z;
		q2.w = outputsVec4[index + 1].w;
		node->rotation = glm::normalize(glm::slerp(q1, q2, u));
		break;
	}
	case AnimationSampler::InterpolationType::STEP: {
		glm::quat q1;
		q1.x = outputsVec4[index].x;
		q1.y = outputsVec4[index].y;
		q1.z = outputsVec4[index].z;
		q1.w = outputsVec4[index].w;
		node->rotation = q1;
		break;
	}
	case AnimationSampler::InterpolationType::CUBICSPLINE: {
		glm::vec4 rot = cubicSplineInterpolation(index, time, 4);
		glm::quat q;
		q.x = rot.x;
		q.y = rot.y;
		q.z = rot.z;
		q.w = rot.w;
		node->rotation = glm::normalize(q);
		break;
	}
	}
}



struct AnimationChannel {
	enum PathType { TRANSLATION, ROTATION, SCALE };
	PathType path;
	GLTFNode* node;
	uint32_t    samplerIndex;
};

struct GLTFAnimation {
	std::string                   name;
	std::vector<AnimationSampler> samplers;
	std::vector<AnimationChannel> channels;
	float                         start = std::numeric_limits<float>::max();
	float                         end = std::numeric_limits<float>::min();
	float                         currentTime = 0.0f;
};

int32_t activeAnimationIndex = 0;
bool animationActive = true;
float animationTimer = 0.0f;

// Animation related
std::vector<GLTFNode*> nodes;
std::vector<GLTFNode*> linearNodes;
std::vector<GLTFSkin*> skins;
std::vector<GLTFAnimation> animations;


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 aTexCoords;
	glm::vec2 aTexCoords2;
	glm::uvec4 joints;
	glm::vec4 weights;
	glm::vec4 color;
};

struct Info {
	Vertex* vertexBuffer;
	uint32_t* indexBuffer;
	size_t vertex_pos;
	size_t index_pos;
};

void load_node(const tinygltf::Node& curr_node, GLTFNode* parent, uint32_t node_index, const tinygltf::Model& model, Info& info) {
	GLTFNode* new_node = new GLTFNode{};
	new_node->parent = parent;
	new_node->skinIndex = curr_node.skin;
	new_node->index = node_index;
	new_node->local_matrix = glm::mat4(1.0f);
	new_node->name = curr_node.name;

	if (curr_node.translation.size() == 3) {
		new_node->translation = glm::make_vec3(curr_node.translation.data());
	}

	if (curr_node.rotation.size() == 4) {
		glm::quat q = glm::make_quat(curr_node.rotation.data());
		new_node->rotation = glm::mat4(q);
	}

	if (curr_node.scale.size() == 3) {
		new_node->scale = glm::make_vec3(curr_node.scale.data());
	}

	if (curr_node.matrix.size() == 16) {
		new_node->local_matrix = glm::make_mat4x4(curr_node.matrix.data());
	}

	if (curr_node.children.size() > 0) {
		for (size_t i = 0; i < curr_node.children.size(); i++) {
			load_node(model.nodes[curr_node.children[i]], new_node, curr_node.children[i], model, info);
		}
	}
	//if (curr_node.mesh > -1 && curr_node.name != "SK_AssaultRifle" && curr_node.name != "SM_AssaultRifle_Magazine") {
	if (curr_node.mesh > -1) {
		const tinygltf::Mesh mesh = model.meshes[curr_node.mesh];
		Mesh* newMesh = new Mesh{};
		for (size_t i = 0; i < mesh.primitives.size(); i++) {
			const tinygltf::Primitive& primitive = mesh.primitives[i];
			bool has_skin = false;
			bool has_indices = primitive.indices > -1;
			GLuint materialId = 0;
			uint32_t vertex_count = 0;
			uint32_t index_count = 0;
			uint32_t vertex_start = static_cast<uint32_t>(info.vertex_pos);
			uint32_t index_start = static_cast<uint32_t>(info.index_pos);

			// vertices

			const float* bufferTexCoordSet1 = nullptr;
			int uv1ByteStride;

			int posByteStride;
			int normByteStride;
			int uv0ByteStride;
			int weightByteStride;

			// Position
			auto posIter = primitive.attributes.find("POSITION");
			const float* posData = nullptr;
			if (posIter != primitive.attributes.end()) {
				const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
				const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
				const auto& posBuffer = model.buffers[posBufferView.buffer];
				vertex_count = static_cast<uint32_t>(posAccessor.count);
				posData = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
				posByteStride = posAccessor.ByteStride(posBufferView) ? (posAccessor.ByteStride(posBufferView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
			}

			// Normal 
			auto normalIter = primitive.attributes.find("NORMAL");
			const float* normalData = nullptr;
			if (normalIter != primitive.attributes.end()) {
				const auto& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
				const auto& normalBufferView = model.bufferViews[normalAccessor.bufferView];
				const auto& normalBuffer = model.buffers[normalBufferView.buffer];
				normalData = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);
				normByteStride = normalAccessor.ByteStride(normalBufferView) ? (normalAccessor.ByteStride(normalBufferView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
			}

			// Textures
			auto textureIter = primitive.attributes.find("TEXCOORD_0");
			const float* textureData = nullptr;
			if (textureIter != primitive.attributes.end()) {
				const auto& textureAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
				const auto& textureBufferView = model.bufferViews[textureAccessor.bufferView];
				const auto& textureBuffer = model.buffers[textureBufferView.buffer];
				textureData = reinterpret_cast<const float*>(&textureBuffer.data[textureBufferView.byteOffset + textureAccessor.byteOffset]);
				uv0ByteStride = textureAccessor.ByteStride(textureBufferView) ? (textureAccessor.ByteStride(textureBufferView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
			}

			if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
				const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
				const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
				bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
				uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
			}

			// Joints and Weights
			auto jointIter = primitive.attributes.find("JOINTS_0");
			auto weightIter = primitive.attributes.find("WEIGHTS_0");

			const void* jointData = nullptr;
			const float* weightData = nullptr;
			int jointComponentType = -1;
			int jointType = -1;
			size_t jointStride = 0;

			if (jointIter != primitive.attributes.end() && weightIter != primitive.attributes.end()) {
				has_skin = true;

				// Joint data
				const auto& jointAccessor = model.accessors[jointIter->second];
				const auto& jointBufferView = model.bufferViews[jointAccessor.bufferView];
				const auto& jointBuffer = model.buffers[jointBufferView.buffer];
				jointData = &jointBuffer.data[jointBufferView.byteOffset + jointAccessor.byteOffset];
				jointComponentType = jointAccessor.componentType;
				jointType = jointAccessor.type;
				jointStride = jointAccessor.ByteStride(jointBufferView) ? (jointAccessor.ByteStride(jointBufferView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);

				// Weight data
				const auto& weightAccessor = model.accessors[weightIter->second];
				const auto& weightBufferView = model.bufferViews[weightAccessor.bufferView];
				const auto& weightBuffer = model.buffers[weightBufferView.buffer];
				weightData = reinterpret_cast<const float*>(&weightBuffer.data[weightBufferView.byteOffset + weightAccessor.byteOffset]);
				weightByteStride = weightAccessor.ByteStride(weightBufferView) ? (weightAccessor.ByteStride(weightBufferView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
			}

			// Append data to model's vertex buffer
			for (size_t v = 0; v < vertex_count; v++)
			{
				Vertex& vert = info.vertexBuffer[info.vertex_pos];
				//vert.position = glm::vec4(glm::make_vec3(&posData[v * posByteStride]), 1.0f);
				vert.position = glm::make_vec3(&posData[v * posByteStride]);
				vert.normal = glm::normalize(glm::vec3(normalData ? glm::make_vec3(&normalData[v * normByteStride]) : glm::vec3(0.0f)));
				//vert.aTexCoords = textureData ? glm::make_vec2(&textureData[v * uv0ByteStride]) : glm::vec3(0.0f);
				//vert.aTexCoords2 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);
				vert.aTexCoords = textureData ? glm::make_vec2(&textureData[v * uv0ByteStride]) : glm::vec2(0.0f);
				vert.aTexCoords2 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec2(0.0f);
				vert.color = glm::vec4(1.0f);
				if (has_skin) {
					switch (jointComponentType) {
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = static_cast<const uint16_t*>(jointData);
						vert.joints = glm::uvec4(glm::make_vec4(&buf[v * jointStride]));
						break;
					}
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = static_cast<const uint8_t*>(jointData);
						vert.joints = glm::vec4(glm::make_vec4(&buf[v * jointStride]));
						break;
					}
					default:
						// Not supported by spec
						std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
						break;
					}
				}
				else {
					vert.joints = glm::vec4(0.0f);
				}
				vert.weights = has_skin ? glm::make_vec4(&weightData[v * weightByteStride]) : glm::vec4(0.0f);
				if (glm::length(vert.weights) == 0.0f) {
					vert.weights = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
				}
				info.vertex_pos++;
			}

			// indices
			if (has_indices) {
				const auto& idxAccessor = model.accessors[primitive.indices];
				const auto& idxBufferView = model.bufferViews[idxAccessor.bufferView];
				const auto& idxBuffer = model.buffers[idxBufferView.buffer];

				index_count = static_cast<uint32_t>(idxAccessor.count);
				const void* dataPtr = &(idxBuffer.data[idxAccessor.byteOffset + idxBufferView.byteOffset]);

				switch (idxAccessor.componentType) {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
					for (size_t index = 0; index < idxAccessor.count; index++) {
						info.indexBuffer[info.index_pos] = buf[index];
						info.index_pos++;
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
					const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
					for (size_t index = 0; index < idxAccessor.count; index++) {
						info.indexBuffer[info.index_pos] = buf[index];
						info.index_pos++;
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
					for (size_t index = 0; index < idxAccessor.count; index++) {
						info.indexBuffer[info.index_pos] = buf[index];
						info.index_pos++;
					}
					break;
				}
				default:
					std::cerr << "Index component type " << idxAccessor.componentType << " not supported!" << std::endl;
					return;
				}
			}


			materialId = primitive.material;

			GLTFMesh* mesh = new GLTFMesh{};

			glGenVertexArrays(1, &mesh->vao);
			glBindVertexArray(mesh->vao);

			glGenBuffers(1, &mesh->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
			glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), &info.vertexBuffer[vertex_start], GL_STATIC_DRAW);

			glGenBuffers(1, &mesh->ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(uint32_t), &info.indexBuffer[index_start], GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, aTexCoords));
			glEnableVertexAttribArray(2);

			if (has_skin) {
				//glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, joints));
				glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, joints));
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
				glEnableVertexAttribArray(4);
			}

			mesh->indexCount = index_count;
			mesh->first_index = index_start;
			mesh->materialId = materialId;

			glBindVertexArray(0);
			newMesh->primitives.push_back(mesh);
		}
		new_node->mesh = newMesh;
	}

	if (parent) {
		parent->children.push_back(new_node);
	}
	else {
		// aca estan todos los nodos sin padre.
		nodes.push_back(new_node);
	}
	linearNodes.push_back(new_node);
}


struct Material {
	glm::vec4 baseColorFactor;
	GLuint baseColorTexture;
};



/*

	TODO GLTF:
			Make the model loading work even if it doesn't have textures.
				Look into padding of the textures. seems automatic but i will jsut add the padding myself
			Go over the tutorial and render it as I go.
			See default size of Blender's cube. its the opposite of what i do, they specify from center to side and i use the godot convention, a full side:
				So blender == joltphysics, godot == mine. Consider.
			See how to interact with default values for textures, see how people do this specially when you have multiple options. a fucking mess

*/

tinygltf::Model loadGLTFModel() {
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "untitled2.glb";
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "Glock.glb";
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "gusano.glb";
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, model_path); // for binary glTF(.glb)	
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "default-cube.gltf";
	//bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path); // for binary glTF(.glb)	

	// para este caso escale el cubo de blender a (0.5, 0.5, 0.5) pero asi por si solo no tiene efecto ya que esa info viene en el gltf claramente.
	// especificamente en: nodes[0].scale, o en json  nodes:[scale:[]]
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "default-cube-scaled-down-to-0.5.gltf";
	//bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path);

	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "default-cube-colored.gltf";
	//bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path); 

	// lgltf
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "CesiumMan.gltf";
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "hello.gltf";
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "assault-rifle.gltf";
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "complete.gltf";
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "RIG_AssaultRifle.gltf";
	std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "jaja.gltf"; // esta es completa con los brazos solos
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "gusano.gltf"; // esta es completa con los brazos solos
	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path);


	/*
		El shader renderiza bien todos los objetos excepto la magazine del rifle
		Si en el shader hardcodeo el jointCount a 0 igual tiene el mismo comportamiento solo que no hay animaciones

		Incluso probe con una importacion nueva que funcionaba en godot y tampoco.

		Puede ser que se este mandando mal el jointCount al shader pero lo dudo muchisimo porque igual hardcodeado me da lo mismo

		Puede ser que no tengo bien los indices de la magazine y el casing y solo estoy mostrando la primer primitiva. Para probar esto voy a mostrar el primer nodo. Si tenia razon
		Algo esta mal con los indices por ahi... Creo que el problema es que cuando guardo los valroes de los indices les sumo algo como buf[i] + vert_pos o algo por el estilo cosa de que los
		indices no se guarden relativamente. El tema es que como yo vuelvo a partir de buffers de 0 digamos, a diferencia de sascha. No deberia tener que sumar y deberia solo usar los indices relativos
		y no los absolutos
	*/

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) {
		printf("Failed to parse glTF\n");
		exit(EXIT_FAILURE);
	}


	for (const auto& skin : model.skins) {
		boneCount += skin.joints.size();
	}

	INFO("There are %d bones", boneCount);

	return model;
}


GLTFNode* findNode(GLTFNode* parent, uint32_t index) {
	GLTFNode* node_found = nullptr;

	if (parent->index == index) {
		return parent;
	}

	for (auto& node : parent->children) {
		node_found = findNode(node, index);
		if (node_found) {
			break;
		}
	}
	return node_found;
}


GLTFNode* nodeFromIndex(uint32_t index) {
	GLTFNode* node_found = nullptr;

	for (auto& node : nodes) {
		node_found = findNode(node, index);
		if (node_found) {
			break;
		}
	}
	return node_found;
}

void load_skins(tinygltf::Model& gltfModel)
{
	for (tinygltf::Skin& source : gltfModel.skins) {
		GLTFSkin* newSkin = new GLTFSkin{};
		newSkin->name = source.name;

		// Find skeleton root node
		if (source.skeleton > -1) {
			newSkin->skeletonRoot = nodeFromIndex(source.skeleton);
		}

		// Find joint nodes
		for (int jointIndex : source.joints) {
			GLTFNode* node = nodeFromIndex(jointIndex);
			if (node) {
				newSkin->joints.push_back(nodeFromIndex(jointIndex));
			}
		}

		// Get inverse bind matrices from buffer
		if (source.inverseBindMatrices > -1) {
			const tinygltf::Accessor& accessor = gltfModel.accessors[source.inverseBindMatrices];
			const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
			newSkin->inverseBindMatrices.resize(accessor.count);
			memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
		}

		skins.push_back(newSkin);
	}
}


void load_animations(tinygltf::Model& input)
{
	animations.resize(input.animations.size());

	for (size_t i = 0; i < input.animations.size(); i++)
	{
		tinygltf::Animation glTFAnimation = input.animations[i];
		animations[i].name = glTFAnimation.name;

		if (glTFAnimation.name.empty()) {
			animations[i].name = std::to_string(animations.size());
		}

		// Samplers
		animations[i].samplers.resize(glTFAnimation.samplers.size());
		for (size_t j = 0; j < glTFAnimation.samplers.size(); j++)
		{
			tinygltf::AnimationSampler glTFSampler = glTFAnimation.samplers[j];
			AnimationSampler& dstSampler = animations[i].samplers[j];
			if (glTFSampler.interpolation == "LINEAR") {
				dstSampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
			}
			if (glTFSampler.interpolation == "STEP") {
				dstSampler.interpolation = AnimationSampler::InterpolationType::STEP;
			}
			if (glTFSampler.interpolation == "CUBICSPLINE") {
				dstSampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
			}

			// Read sampler keyframe input time values
			{
				const tinygltf::Accessor& accessor = input.accessors[glTFSampler.input];
				const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const float* buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++)
				{
					dstSampler.inputs.push_back(buf[index]);
				}
				// Adjust animation's start and end times
				for (auto input : animations[i].samplers[j].inputs)
				{
					if (input < animations[i].start)
					{
						animations[i].start = input;
					};
					if (input > animations[i].end)
					{
						animations[i].end = input;
					}
				}
			}

			// Read sampler keyframe output translate/rotate/scale values
			{
				const tinygltf::Accessor& accessor = input.accessors[glTFSampler.output];
				const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3: {
					const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						dstSampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
						dstSampler.outputs.push_back(buf[index][0]);
						dstSampler.outputs.push_back(buf[index][1]);
						dstSampler.outputs.push_back(buf[index][2]);
					}
					break;
				}
				case TINYGLTF_TYPE_VEC4: {
					const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						dstSampler.outputsVec4.push_back(buf[index]);
						dstSampler.outputs.push_back(buf[index][0]);
						dstSampler.outputs.push_back(buf[index][1]);
						dstSampler.outputs.push_back(buf[index][2]);
						dstSampler.outputs.push_back(buf[index][3]);
					}
					break;
				}
				default: {
					std::cout << "unknown type" << std::endl;
					break;
				}
				}
			}
		}

		// Channels
		animations[i].channels.resize(glTFAnimation.channels.size());
		for (size_t j = 0; j < glTFAnimation.channels.size(); j++)
		{
			tinygltf::AnimationChannel glTFChannel = glTFAnimation.channels[j];
			AnimationChannel& dstChannel = animations[i].channels[j];
			if (glTFChannel.target_path == "rotation") {
				dstChannel.path = AnimationChannel::PathType::ROTATION;
			}
			if (glTFChannel.target_path == "translation") {
				dstChannel.path = AnimationChannel::PathType::TRANSLATION;
			}
			if (glTFChannel.target_path == "scale") {
				dstChannel.path = AnimationChannel::PathType::SCALE;
			}
			if (glTFChannel.target_path == "weights") {
				std::cout << "weights not yet supported, skipping channel" << std::endl;
				continue;
			}
			dstChannel.samplerIndex = glTFChannel.sampler;
			dstChannel.node = nodeFromIndex(glTFChannel.target_node);
			if (!dstChannel.node) {
				FATAL("COULD NOT LOAD ANIMATION NODE - EMPTY");
				abort();
			}
		}
	}
}


glm::mat4 getNodeMatrix(GLTFNode* node) {
	GLTFNode* curr_parent = node->parent;
	glm::mat4 node_matrix = node->getLocalMatrix();

	while (curr_parent) {
		node_matrix = curr_parent->getLocalMatrix() * node_matrix;
		curr_parent = curr_parent->parent;
	}

	return node_matrix;
}

void GLTFNode::update()
{
	if (mesh) {
		glm::mat4 m = getMatrix();
		if (skin)
		{
			mesh->uniformBlock.matrix = m;
			glm::mat4              inverseTransform = glm::inverse(m);
			/*
			`numJoints`: la cantidad de joints que tiene el mesh
			Como maximo cada vertex del mesh puede ser afectado por 4 huesos.
			De cada hueso se conoce un weight que determina cuanta influencia da cada hueso.
			Cada hueso tiene una inverseBindMatrix y tiene un transform relativo al hueso padre.
			*/
			size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);
			for (size_t i = 0; i < numJoints; i++)
			{
				GLTFNode* jointNode = skin->joints[i];
				glm::mat4 jointMat = jointNode->getMatrix() * skin->inverseBindMatrices[i];
				jointMat = inverseTransform * jointMat;
				mesh->uniformBlock.jointMatrix[i] = jointMat;
				//	mesh->uniformBlock.jointMatrix[i] = glm::mat4(1.0f);
			}
			mesh->uniformBlock.jointCount = static_cast<uint32_t>(numJoints);



			//glUseProgram(skinning_shader_id);
			//GLint jointMatricesLoc = glGetUniformLocation(skinning_shader_id, "jointMatrices");
			//glUniformMatrix4fv(jointMatricesLoc, numJoints, GL_FALSE, glm::value_ptr(mesh->uniformBlock.jointMatrix[0]));

			//glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), mesh->uniformBlock.jointCount);

			//glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &mesh->uniformBlock.matrix[0][0]);
		}
		else {
			// transform relative al parent
			mesh->uniformBlock.matrix = m;

			//glUseProgram(skinning_shader_id);
			//glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &m[0][0]);
			//glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);
		}
	}

	for (auto& child : children) {
		child->update();
	}
}

void updateAnimation(uint32_t index, float time)
{
	if (animations.empty()) {
		std::cout << ".glTF does not contain animation." << std::endl;
		return;
	}
	if (index > static_cast<uint32_t>(animations.size()) - 1) {
		std::cout << "No animation with index " << index << std::endl;
		return;
	}
	GLTFAnimation& animation = animations[index];
	std::cout << "Current animation: " << animation.name << std::endl;

	bool updated = false;
	for (auto& channel : animation.channels) {
		AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		if (sampler.inputs.size() > sampler.outputsVec4.size()) {
			std::cout << "CONTINUEEE********\n";
			continue;
		}

		for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
			if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1])) {
				float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (u <= 1.0f) {
					switch (channel.path) {
					case AnimationChannel::PathType::TRANSLATION:
						sampler.translate(i, time, channel.node);
						break;
					case AnimationChannel::PathType::SCALE:
						sampler.scale(i, time, channel.node);
						break;
					case AnimationChannel::PathType::ROTATION:
						sampler.rotate(i, time, channel.node);
						break;
					}
					updated = true;
				}
			}
		}
	}
	if (updated) {
		for (auto& node : nodes) {
			node->update();
		}
	}
}



std::vector<glm::mat4> get_inverse_bind_matrix(const tinygltf::Model& model) {
	std::vector<glm::mat4> result;
	for (int i = 0; i < model.skins.size(); i++) {
		auto inverse_bind_matrix_index = model.skins[i].inverseBindMatrices;
		auto accessor = model.accessors[inverse_bind_matrix_index];
		auto inverse_bind_matrix_bufferview = model.bufferViews[accessor.bufferView];
		auto inverse_bind_matrix_buffer = model.buffers[inverse_bind_matrix_bufferview.buffer];

		size_t offset = accessor.byteOffset + inverse_bind_matrix_bufferview.byteOffset;

		// Ensure the data is a MAT4 (4x4 matrix)
		if (accessor.type != TINYGLTF_TYPE_MAT4 || accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
			std::cerr << "Expected a MAT4 with float components" << std::endl;
			return std::vector<glm::mat4>();
		}

		result.resize(accessor.count);
		memcpy(result.data(), &inverse_bind_matrix_buffer.data[offset], sizeof(glm::mat4) * accessor.count);
	}
	return result;
}

void print_matrix(const glm::mat4& mat) {
	std::cout << "--------------------------\n";
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << "\n";
	}
}

void getNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount)
{
	if (node.children.size() > 0) {
		for (size_t i = 0; i < node.children.size(); i++) {
			getNodeProps(model.nodes[node.children[i]], model, vertexCount, indexCount);
		}
	}
	if (node.mesh > -1) {
		const tinygltf::Mesh mesh = model.meshes[node.mesh];
		for (size_t i = 0; i < mesh.primitives.size(); i++) {
			auto primitive = mesh.primitives[i];
			vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;
			if (primitive.indices > -1) {
				indexCount += model.accessors[primitive.indices].count;
			}
		}
	}
}


glm::mat4 m_globalInverseTransform;

void render_node(GLTFNode* node, Shader* skinning_shader, Shader* regular_shader) {
	if (node->mesh) {

		for (const auto& mesh : node->mesh->primitives) {
			glBindVertexArray(mesh->vao);

			//std::cout << "node name: " << node->name << " Number fo primitives: " << node->mesh->primitives.size() << std::endl;

			skinning_shader->use();
			if (node->name == "Cesium_Man") {
				glm::quat qx = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qy = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qz = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot = qz * qy * qx; // Specify order of rotations here
				glm::mat4 model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.5f, -10.0f)) *
					glm::mat4_cast(rot) *
					glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

				skinning_shader->setMat4("model", model_mat);
			}
			else {
				glm::quat qx = glm::angleAxis(glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qy = glm::angleAxis(glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qz = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot = qz * qy * qx; // Specify order of rotations here

				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
					//glm::mat4_cast(rot) *
					glm::scale(glm::mat4(1.0f), glm::vec3(0.0125f));
				skinning_shader->setMat4("model", base_model_mat);

			}
			glUseProgram(skinning_shader_id);
			GLint jointMatricesLoc = glGetUniformLocation(skinning_shader_id, "jointMatrices");
			glUniformMatrix4fv(jointMatricesLoc, node->mesh->uniformBlock.jointCount, GL_FALSE, glm::value_ptr(node->mesh->uniformBlock.jointMatrix[0]));

			glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), node->mesh->uniformBlock.jointCount);

			glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &node->mesh->uniformBlock.matrix[0][0]);

			//glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_SHORT, 0);
			glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

	for (auto& child : node->children) {
		render_node(child, skinning_shader, regular_shader);
	}
}


struct AssimpVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 aTexCoords;
	uint32_t joints[4];
	float weights[4];
	//glm::uvec4 joints;
	//glm::vec4 weights;
};

struct BoneData {
	int joints[MAX_NUM_BONES_PER_VERTEX];
	float weights[MAX_NUM_BONES_PER_VERTEX];
	int count = 0;
};


struct AssimpPrimitive {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	uint32_t index_count;
};

struct AssimpMesh {
	std::vector<AssimpPrimitive*> meshes;
	struct UniformBlock {
		glm::mat4 matrix;
		glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
		uint32_t jointCount{ 0 };
	} uniformBlock;
};

struct AssimpNode {
	AssimpMesh* mesh;
	glm::mat4 transform;
	std::string name;
	AssimpNode* parent;
	std::vector<AssimpNode*> children;
	bool skin = false;
};

struct SkinnedModel {
	std::string label;
	AssimpNode* node;
	uint8_t skeleton_index;
};


struct AssimpBoneInfo {
	uint32_t id;
	glm::mat4 offset;
};


struct Skeleton {
	std::string label;
	int8_t parent_index{ -1 };
	AssimpNode* node;
	std::unordered_map<std::string, AssimpBoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;
};

std::vector<Skeleton> skeletons;


struct SceneGraphNode {
	std::vector<AssimpNode*> assimp_nodes;
	//std::vector<AssimpAnimation> animations;
	std::string scene_name;

};

struct SceneGraph {
	std::vector<SceneGraphNode> nodes;
};

void processAssimpNode(aiNode* node, AssimpNode* parent, const aiScene* scene, SceneGraphNode& scene_graph_node);

SceneGraphNode loadAssimp(AssimpNode* assimp_model, std::string path) {
	//lass
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		FATAL("ERROR::ASSIMP::%s\n", import.GetErrorString());
		abort();
	}
	SceneGraphNode scene_graph_node;
	scene_graph_node.scene_name = scene->GetShortFilename(path.c_str());

	m_globalInverseTransform = glm::inverse(AssimpGLMHelpers::ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation));
	processAssimpNode(scene->mRootNode, nullptr, scene, scene_graph_node);
	return scene_graph_node;
}


#pragma region bones_container
/* Container for bone data */

struct KeyPosition
{
	glm::vec3 position;
	float timeStamp;
};

struct KeyRotation
{
	glm::quat orientation;
	float timeStamp;
};

struct KeyScale
{
	glm::vec3 scale;
	float timeStamp;
};

class Bone
{
public:
	Bone(const std::string& name, int ID, const aiNodeAnim* channel)
		:
		m_Name(name),
		m_ID(ID),
		m_LocalTransform(1.0f)
	{
		m_NumPositions = channel->mNumPositionKeys;

		for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
		{
			aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
			float timeStamp = channel->mPositionKeys[positionIndex].mTime;
			KeyPosition data;
			data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
			data.timeStamp = timeStamp;
			m_Positions.push_back(data);
		}

		m_NumRotations = channel->mNumRotationKeys;
		for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
		{
			aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
			float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
			KeyRotation data;
			data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
			data.timeStamp = timeStamp;
			m_Rotations.push_back(data);
		}

		m_NumScalings = channel->mNumScalingKeys;
		for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
		{
			aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
			float timeStamp = channel->mScalingKeys[keyIndex].mTime;
			KeyScale data;
			data.scale = AssimpGLMHelpers::GetGLMVec(scale);
			data.timeStamp = timeStamp;
			m_Scales.push_back(data);
		}
	}

	void Update(float animationTime)
	{
		glm::mat4 translation = InterpolatePosition(animationTime);
		glm::mat4 rotation = InterpolateRotation(animationTime);
		glm::mat4 scale = InterpolateScaling(animationTime);
		m_LocalTransform = translation * rotation * scale;
	}
	glm::mat4 GetLocalTransform() { return m_LocalTransform; }
	std::string GetBoneName() const { return m_Name; }
	int GetBoneID() { return m_ID; }



	int GetPositionIndex(float animationTime)
	{
		for (int index = 0; index < m_NumPositions - 1; ++index)
		{
			if (animationTime < m_Positions[index + 1].timeStamp)
				return index;
		}
		assert(0);
	}

	int GetRotationIndex(float animationTime)
	{
		for (int index = 0; index < m_NumRotations - 1; ++index)
		{
			if (animationTime < m_Rotations[index + 1].timeStamp)
				return index;
		}
		assert(0);
	}

	int GetScaleIndex(float animationTime)
	{
		for (int index = 0; index < m_NumScalings - 1; ++index)
		{
			if (animationTime < m_Scales[index + 1].timeStamp)
				return index;
		}
		assert(0);
	}


private:

	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	glm::mat4 InterpolatePosition(float animationTime)
	{
		if (1 == m_NumPositions)
			return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

		int p0Index = GetPositionIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
			m_Positions[p1Index].timeStamp, animationTime);
		glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position
			, scaleFactor);
		return glm::translate(glm::mat4(1.0f), finalPosition);
	}

	glm::mat4 InterpolateRotation(float animationTime)
	{
		if (1 == m_NumRotations)
		{
			auto rotation = glm::normalize(m_Rotations[0].orientation);
			return glm::toMat4(rotation);
		}

		int p0Index = GetRotationIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
			m_Rotations[p1Index].timeStamp, animationTime);
		glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation
			, scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return glm::toMat4(finalRotation);

	}

	glm::mat4 InterpolateScaling(float animationTime)
	{
		if (1 == m_NumScalings)
			return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

		int p0Index = GetScaleIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
			m_Scales[p1Index].timeStamp, animationTime);
		glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
			, scaleFactor);
		return glm::scale(glm::mat4(1.0f), finalScale);
	}

	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotations;
	std::vector<KeyScale> m_Scales;
	int m_NumPositions;
	int m_NumRotations;
	int m_NumScalings;

	glm::mat4 m_LocalTransform;
	std::string m_Name;
	int m_ID;
};
Bone* root_node_bone = nullptr;
Bone* root = nullptr;
Bone* ik_hand_root = nullptr;
Bone* ik_hand_gun_bone = nullptr;

glm::mat4 todas_las_putas_transforms{};

#pragma endregion bones_container

#pragma region assimp_animation

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation
{
public:
	Animation() = default;

	Animation(const std::string& animationPath, int skeleton_index)
	{
		Assimp::Importer importer;
		m_skeleton_index = skeleton_index;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		auto animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		//m_TicksPerSecond = 1000;

		// esto podria apuntar al nodo en la lista de nodes. Esto va a pasar por
		// RootNode -> Armature -> ...
		ReadHeirarchyData(m_RootNode, scene->mRootNode);

		//m_globalInverseTransform = glm::inverse(AssimpGLMHelpers::ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation));
		// esto se puede quedar
		ReadMissingBones(animation);

		std::cout << "Animation name is: " << scene->mAnimations[0]->mName.C_Str() << std::endl;
	}

	~Animation()
	{
	}

	// this is my version
	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	//Bone* FindBone(const std::string& name)
	//{
	//	auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
	//		[&](const Bone& Bone)
	//		{
	//			return Bone.GetBoneName() == name;
	//		}
	//	);
	//	if (iter == m_Bones.end()) return nullptr;
	//	else return &(*iter);
	//}


	inline float GetTicksPerSecond() { return m_TicksPerSecond; }

	inline float GetDuration() { return m_Duration; }

	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

	//inline const std::map<std::string, AssimpBoneInfo>& GetBoneIDMap()
	//{
	//	return m_BoneInfoMap;
	//}

private:
	void ReadMissingBones(const aiAnimation* animation)
	{
		int size = animation->mNumChannels;
		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (skeletons[m_skeleton_index].m_BoneInfoMap.find(boneName) == skeletons[m_skeleton_index].m_BoneInfoMap.end())
			{
				skeletons[m_skeleton_index].m_BoneInfoMap[boneName].id = skeletons[m_skeleton_index].m_BoneCounter;
				skeletons[m_skeleton_index].m_BoneCounter++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				skeletons[m_skeleton_index].m_BoneInfoMap[channel->mNodeName.data].id, channel));
		}
	}

	// esto lee toda la jerarquia de la animacion, no solo el skeleton
	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		//if (m_BoneInfoMap.find(dest.name) != m_BoneInfoMap.end()) {
		//	std::cout << dest.name << std::endl;
		//	std::cout << m_BoneInfoMap[dest.name].id << std::endl;
		//}
		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	float m_Duration;
	int m_TicksPerSecond;
public:
	// todo deal with this. Prolly erase it
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
public:
	int m_skeleton_index;
};
#pragma endregion assimp_animation

#pragma region assimp_animator

class Animator
{
public:
	Animator(Animation* animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(200);

		for (int i = 0; i < 200; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		// todo check this throughly
		// it could matter that node->transformation is useful if i have something like this:
		/*
			Node
			  MeshNode
			  MeshNode
				Meshnode
				  Bone
					Bone1
					Bone2
					Bone3
					  BoneChild1...
		*/

		glm::mat4 nodeTransform;
		glm::mat4 globalTransformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		// armature has a transform
		// me parece que es el que mueve la armature de lugar inicialmente. probar comentando esto si es el Bone->GetBoneName() == "Armature" 
		// entonces identity// Si era eso. 

		if (Bone) {
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
			globalTransformation = parentTransform * nodeTransform;

			int index = skeletons[m_CurrentAnimation->m_skeleton_index].m_BoneInfoMap[nodeName].id;
			glm::mat4 offset = skeletons[m_CurrentAnimation->m_skeleton_index].m_BoneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = m_globalInverseTransform * globalTransformation * offset;
		}
		else {
			nodeTransform = node->transformation;
			globalTransformation = parentTransform * nodeTransform;
		}


		//std::cout << "Parent transform: " << std::endl;
		//print_matrix(parentTransform);
		//std::cout << "Node transform: " << std::endl;
		//print_matrix(nodeTransform);
		//std::cout << "Global transformation: " << std::endl;
		//print_matrix(globalTransformation);

		//auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap(); // it looks like this takes the map as the animation have it, which means its a copy and not a reference
		// should investigate


		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

private:
	float m_CurrentTime;
	float m_DeltaTime;
public:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;

};
#pragma endregion assimp_animator

void processAssimpNode(aiNode* node, AssimpNode* parent, const aiScene* scene, SceneGraphNode& scene_graph_node) {
	AssimpNode* new_node = new AssimpNode{};
	new_node->parent = parent;
	new_node->name = std::string(node->mName.C_Str());
	new_node->transform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);

	for (int i = 0; i < node->mNumChildren; i++) {
		processAssimpNode(node->mChildren[i], new_node, scene, scene_graph_node);
	}

	std::cout << "Node name: " << new_node->name << std::endl;

	if (node->mNumMeshes > 0) {
		AssimpMesh* new_mesh = new AssimpMesh{};
		for (int i = 0; i < node->mNumMeshes; i++) {
			// Si o si tienen que estar los dos meshes.
			// El problema de esto es que se repite el skeleton por algun motivo. Como puedo saber si el skeleton ya existe?
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			AssimpPrimitive* new_primitive = new AssimpPrimitive{};
			bool has_skin = mesh->HasBones();

			std::cout << "Mesh name: " << std::string(mesh->mName.C_Str()) << std::endl;
			BoneData* vertex_id_to_bone_id = nullptr;
			if (has_skin) {
				Skeleton new_skeleton = Skeleton{};
				vertex_id_to_bone_id = new BoneData[mesh->mNumVertices]{};
				DEBUG("Processing Bones:\n");
				for (size_t i = 0; i < mesh->mNumBones; i++) {
					std::string boneName = mesh->mBones[i]->mName.C_Str();
					int boneId = -1;
					if (new_skeleton.m_BoneInfoMap.find(boneName) == new_skeleton.m_BoneInfoMap.end()) {
						AssimpBoneInfo boneInfo;
						boneInfo.id = new_skeleton.m_BoneCounter;
						boneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[i]->mOffsetMatrix);
						new_skeleton.m_BoneInfoMap[boneName] = boneInfo;
						boneId = new_skeleton.m_BoneCounter;
						new_skeleton.m_BoneCounter++;
					}
					else {
						boneId = new_skeleton.m_BoneInfoMap[boneName].id;
						//abort();
						std::cout << "hola";
					}
					assert(boneId != -1);
					for (size_t j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
						uint32_t vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
						float weight_value = mesh->mBones[i]->mWeights[j].mWeight;

						if (vertex_id_to_bone_id[vertex_id].count < MAX_NUM_BONES_PER_VERTEX) {
							vertex_id_to_bone_id[vertex_id].joints[vertex_id_to_bone_id[vertex_id].count] = boneId;
							vertex_id_to_bone_id[vertex_id].weights[vertex_id_to_bone_id[vertex_id].count] = weight_value;
							vertex_id_to_bone_id[vertex_id].count++;
						}
					}
				}
				skeletons.push_back(new_skeleton);
			}
			DEBUG("Processing Mesh: %s", mesh->mName.C_Str());

			DEBUG("Processing Vertices:");
			DEBUG("\tVertices count: % d", mesh->mNumVertices);

			std::vector<AssimpVertex> assimp_vertices;
			assimp_vertices.reserve(mesh->mNumVertices);
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				AssimpVertex vertex;
				glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
				// positions
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.position = vector;
				// normals
				if (mesh->HasNormals())
				{
					vector.x = mesh->mNormals[i].x;
					vector.y = mesh->mNormals[i].y;
					vector.z = mesh->mNormals[i].z;
					vertex.normal = vector;
				}
				vertex.aTexCoords = glm::vec2(0.0f, 0.0f);
				if (has_skin) {
					//vertex.joints = glm::make_vec4(vertex_id_to_bone_id[i].joints.data());
					//vertex.weights = glm::make_vec4(vertex_id_to_bone_id[i].weights.data());
					vertex.joints[0] = vertex_id_to_bone_id[i].joints[0];
					vertex.joints[1] = vertex_id_to_bone_id[i].joints[1];
					vertex.joints[2] = vertex_id_to_bone_id[i].joints[2];
					vertex.joints[3] = vertex_id_to_bone_id[i].joints[3];

					vertex.weights[0] = vertex_id_to_bone_id[i].weights[0];
					vertex.weights[1] = vertex_id_to_bone_id[i].weights[1];
					vertex.weights[2] = vertex_id_to_bone_id[i].weights[2];
					vertex.weights[3] = vertex_id_to_bone_id[i].weights[3];
				}
				else {
					//vertex.joints = glm::vec4(0.0f);

					vertex.joints[0] = 0;
					vertex.joints[1] = 0;
					vertex.joints[2] = 0;
					vertex.joints[3] = 0;

					//vertex.weights = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
					vertex.weights[0] = 1.0f;
					vertex.weights[1] = 0.0f;
					vertex.weights[2] = 0.0f;
					vertex.weights[3] = 0.0f;

				}
				// this is probably not needed

				assimp_vertices.push_back(vertex);
			}

			DEBUG("Processing Indices...\n");
			std::vector<uint32_t> assimp_indices;
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					assimp_indices.push_back(face.mIndices[j]);
			}

			new_primitive->index_count = assimp_indices.size();
			DEBUG("There are %d indices\n", new_primitive->index_count);


			glGenVertexArrays(1, &new_primitive->vao);
			glBindVertexArray(new_primitive->vao);

			glGenBuffers(1, &new_primitive->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, new_primitive->vbo);
			glBufferData(GL_ARRAY_BUFFER, assimp_vertices.size() * sizeof(AssimpVertex), assimp_vertices.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &new_primitive->ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_primitive->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, assimp_indices.size() * sizeof(uint32_t), assimp_indices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, normal));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, aTexCoords));
			glEnableVertexAttribArray(2);

			if (has_skin) {
				glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, joints));
				//glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, joints));
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(AssimpVertex), (void*)offsetof(AssimpVertex, weights));
				glEnableVertexAttribArray(4);

				delete[] vertex_id_to_bone_id;
			}
			glBindVertexArray(0);

			//for (size_t i = 0; i < assimp_vertices.size(); i++) {
				//std::cout << "Vertex: " << i << std::endl;
				//std::cout << "\t joints: [" << assimp_vertices[i].joints[0] << ", " << assimp_vertices[i].joints[1] << ", " << assimp_vertices[i].joints[2] << ", " << assimp_vertices[i].joints[3] << "]";
				//std::cout << "\t weights: [" << assimp_vertices[i].weights[0] << ", " << assimp_vertices[i].weights[1] << ", " << assimp_vertices[i].weights[2] << ", " << assimp_vertices[i].weights[3] << "]";
			//}

			new_mesh->meshes.push_back(new_primitive);
		}
		new_node->mesh = new_mesh;
	}
	if (parent) {
		parent->children.push_back(new_node);
		std::string parent_name = new_node->parent ? new_node->parent->name : std::string("NULL");
		std::cout << "NODE NAME: " << new_node->name << "  NODE PARENT: " << parent_name << std::endl;
	}
	else {
		scene_graph_node.assimp_nodes.push_back(new_node);
	}
	if (new_node->name == "ROOT") {
		std::cout << "IM ROOT" << std::endl;
		for (size_t i = 0; i < new_node->children.size(); i++) {
			std::cout << "CHILDREN " << i << "  CHILDREN NAME: " << new_node->children[i]->name << std::endl;

		}
	}
}

void load_assimp_anim(std::string path) {
	//lass
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!scene || !scene->mRootNode)
	{
		FATAL("ERROR::ASSIMP::%s file%s\n", import.GetErrorString(), scene->GetShortFilename(path.c_str()));
		abort();
	}
	if (!scene->HasAnimations()) {
		FATAL("Model %s has no animations\n", scene->GetShortFilename(path.c_str()));
		abort();
	}

	for (size_t i = 0; i < scene->mNumAnimations; i++) {
		std::cout << "Animation name: " << scene->mAnimations[i]->mName.C_Str() << std::endl;
	}
}
void render_assimp_node2(AssimpNode* node, Shader* skinning_shader, Shader* regular_shader) {
	if (node->mesh) {
		for (const auto& mesh : node->mesh->meshes) {
			glBindVertexArray(mesh->vao);

			std::cout << "node name: " << node->name << std::endl;

			skinning_shader->use();
			if (node->name == "SK_Manny_Arms") {

				for (int i = 0; i < manny_transforms.size(); ++i)
				{
					skinning_shader->setMat4("jointMatrices[" + std::to_string(i) + "]", manny_transforms[i]);
				}
				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
					glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
				skinning_shader->setMat4("model", base_model_mat);

				glUseProgram(skinning_shader_id);
				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &node->transform[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);
				if (node->name == "SK_Manny_Arms")
					glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 1);
				else
					glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);

				glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
				glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);
				glBindVertexArray(0);

			}
			else {
				for (int i = 0; i < manny_transforms.size(); ++i)
				{
					skinning_shader->setMat4("jointMatrices[" + std::to_string(i) + "]", glm::mat4(1.0f));
				}

			}

		}
	}

	for (auto& child : node->children) {
		render_assimp_node2(child, skinning_shader, regular_shader);
	}
}



void render_assimp_node(AssimpNode* node, Shader* skinning_shader, Shader* regular_shader) {
	// IMPORTANT los huesos que se cargan en las animaciones no estan en el scene graph. eso es importante


	// Armature == SKEL_AssaultRifle == SK_AssaultRifle (Mesh)
	// tengo que ver como es que se updatea el hueso con el mesh. esto funciona en las animaciones si
	// Si muevo el grip todo se tiene que mover, entonces si aplico... TODO: mover el mesh y el grip lo mismo, el grip moverlo con un glm::translate etc
	if (node->name == "Grip") {
		grip_transform = node->transform;
		std::cout << "GRIP BONE" << std::endl;
		print_matrix(node->transform);
		std::cout << "" << std::endl;
	}
	if (node->name == "SKEL_AssaultRifle") {
		skel_assault_rifle_transform = node->transform;
		//(esto deberia ser como Armature en manny pero no esta testeado, o sea que deberia ser un hueso)
		std::cout << "SKEL_AssaultRifle BONE " << std::endl;
		print_matrix(node->transform);
		std::cout << "" << std::endl;
	}
	if (node->name == "SKEL_AssaultRifle") {
		//node->transform = glm::translate(node->transform, glm::vec3(2.0f, 2.0f, 2.0f));
		std::cout << "SKEL_AssaultRifle (armature)" << std::endl;
		print_matrix(node->transform);
		// esta y la de abajo son iguales y concinden con la de blender
	}
	if (node->name == "Armature") {
		std::cout << "Armature (same as SKEL_AssaultRifle)" << std::endl;
		print_matrix(node->transform);
		// esto esta mal, armature es del manny, no tiene nada que ver con esto
		// TODO ver esto porque deberia haber 2 armatures nodes, uno en el fbx del rifle y otro en el del manny. wtf?
		// IMPORTANT el node Armature solo esta en manny jajaja

		glm::quat qx = glm::angleAxis(glm::radians(rifle_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat qy = glm::angleAxis(glm::radians(rifle_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat qz = glm::angleAxis(glm::radians(rifle_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::quat rot = qz * qy * qx; // Specify order of rotations here

		armature_transform = glm::translate(glm::mat4(1.0f), rifle_pos) * glm::mat4_cast(rot) * node->transform;
		std::cout << "TEMPPPPPPP" << std::endl;
		armature_count++;
		print_matrix(armature_transform);
		std::cout << "" << std::endl;
	}
	if (node->name == "Magazine") {
		// esto nunca llega, tnego que poder tener un mapa de huesos por ahi
		std::cout << "MAGAZINE BONE" << std::endl;
		//node->transform = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
		mag_transform = node->transform;
		print_matrix(node->transform);
		std::cout << "" << std::endl;
	}
	if (node->name == "Bolt") {
		// esto nunca llega, tnego que poder tener un mapa de huesos por ahi
		std::cout << "BOLT BONE" << std::endl;
		print_matrix(node->transform);
		std::cout << "" << std::endl;
	}
	if (node->name == "Trigger") {
		// esto nunca llega, tnego que poder tener un mapa de huesos por ahi
		std::cout << "TRIGGER BONE" << std::endl;
		print_matrix(node->transform);
		std::cout << "" << std::endl;
	}
	if (node->name == "ik_hand_gun") {
		std::cout << "IK_HAND_GUN BONE" << std::endl;
		print_matrix(node->transform);
		std::cout << "" << std::endl;
	}

	if (node->mesh) {
		for (const auto& mesh : node->mesh->meshes) {
			glBindVertexArray(mesh->vao);

			std::cout << "node name: " << node->name << std::endl;

			skinning_shader->use();
			glm::quat qx = glm::angleAxis(glm::radians(rifle_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat qy = glm::angleAxis(glm::radians(rifle_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat qz = glm::angleAxis(glm::radians(rifle_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::quat rot = qz * qy * qx; // Specify order of rotations here

			glm::mat4 base_model_mat =
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
				//glm::mat4_cast(rot) *
				glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
			skinning_shader->setMat4("model", base_model_mat);
			skinning_shader->setMat4("model", glm::mat4(1.0f));



			if (node->name == "SM_AssaultRifle_Magazine") {
				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
					glm::scale(glm::mat4(1.0f), glm::vec3(rifle_scale));
				//glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

				skinning_shader->setMat4("model", base_model_mat);
				skinning_shader->setMat4("model", armature_transform * base_model_mat);
				skinning_shader->setMat4("model", todas_las_putas_transforms);
				skinning_shader->setMat4("model", glm::mat4(1.0f));

				for (int i = 0; i < mag_transforms.size(); ++i)
				{
					skinning_shader->setMat4("jointMatrices[" + std::to_string(i) + "]", mag_transforms[i]);
				}

			}

			if (node->name == "SM_AssaultRifle_Casing") {
				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 3.0f, 0.0f)) *
					glm::scale(glm::mat4(1.0f), glm::vec3(0.0125f));

				skinning_shader->setMat4("model", base_model_mat);
			}

			if (node->name == "SK_AssaultRifle") {
				std::cout << "SK_AssaultRifle transform (MESH):" << std::endl;
				print_matrix(node->transform);
				std::cout << "" << std::endl;

				glUseProgram(skinning_shader_id);
				for (int i = 0; i < 200; ++i) {
					std::string name = std::string("jointMatrices[" + std::to_string(i) + "]");
					glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, name.c_str()), 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);
				}

				//node->transform = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), rifle_pos) *
					//glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f)) *
					glm::mat4_cast(rot) *
					glm::scale(glm::mat4(1.0f), glm::vec3(rifle_scale));

				skinning_shader->setMat4("model", base_model_mat);
				skinning_shader->setMat4("model", glm::mat4(1.0f));

				//glm::mat4 new_mat = ik_hand_gun_bone->GetLocalTransform();

				//skinning_shader->setMat4("model", new_mat);
			}
			glUseProgram(skinning_shader_id);
			//GLint jointMatricesLoc = glGetUniformLocation(skinning_shader_id, "jointMatrices");
			//glUniformMatrix4fv(jointMatricesLoc, node->mesh->uniformBlock.jointCount, GL_FALSE, glm::value_ptr(node->mesh->uniformBlock.jointMatrix[0]));

			//glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), node->mesh->uniformBlock.jointCount);

			if (node->name == "Vampire" || node->name == "Circle"  || node->name == "SK_Manny_Arms" || node->name == "Magazine")
				glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 1);
			else
				glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);

			if (node->name == "SK_Manny_Arms") {
				glm::quat qx = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qy = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qz = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot = qy * qx * qz; // Specify order of rotations here

				glm::mat4 model = glm::translate(glm::mat4(1.0f), manny_pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), glm::vec3(manny_scale));
				skinning_shader->setMat4("model", model);
				manny_world_transform = model * node->transform;
				for (int i = 0; i < manny_transforms.size(); ++i)
				{
					skinning_shader->setMat4("jointMatrices[" + std::to_string(i) + "]", manny_transforms[i]);
				}
			}

			glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &node->transform[0][0]);
			if (node->name == "SK_AssaultRifle") {
				//glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &node->transform[0][0]);

				//todo sacar la trans the `ik_hand_gun_bone->GetLocalTransform()` y ver que tan lejos esta la gun de la mano
				// ver esto porque el manny esta al reves, capaz es esa la cagada, tienen distintas coordenadas
				glm::mat4 jaja = ik_hand_gun_bone->GetLocalTransform();
				jaja[3][1] *= -1.0;    // Negate the Y component
				jaja[3][2] *= -1.0;
				//glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, -.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
				//jaja = rotationMatrix * jaja;
				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &jaja[0][0]);
				//glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &ik_something[0][0]);


				print_matrix(glm::inverse(skel_assault_rifle_transform * grip_transform) * todas_las_putas_transforms);
				AssimpBoneInfo grip = skeletons[2].m_BoneInfoMap["Grip"];
				print_matrix(grip.offset);
				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &(glm::inverse(skel_assault_rifle_transform * grip_transform) * todas_las_putas_transforms * grip.offset)[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &todas_las_putas_transforms[0][0]);
				print_matrix(manny_world_transform);
				print_matrix(manny_world_transform * todas_las_putas_transforms);
				print_matrix(manny_world_transform * (glm::inverse(skel_assault_rifle_transform * grip_transform) * todas_las_putas_transforms * grip.offset));
				// no son iguales pero coincide su traslacion
				//assert(manny_world_transform * (glm::inverse(skel_assault_rifle_transform * grip_transform) * todas_las_putas_transforms * grip.offset) == todas_las_putas_transforms);

				// TODO ahora yo estoy modificando el transform del nodo SK_AssaultRifle pero lo que deberia modificar es el root bone si es que es skinned. En este caso
				// el grip bone. Esto necesita mas planning.  Unreal engine parece que lo mappea asi no mas, no al grip pero al nodo. aunque seguro se pueda las dos
				assault_rifle_transform = manny_world_transform * todas_las_putas_transforms;
				//grip_transform = assault_rifle_transform;
				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &assault_rifle_transform[0][0]);

				//glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &grip_transform[0][0]);
			}
			else {
				if (node->name == "SM_AssaultRifle_Magazine")
					// esto es asi porque el transform es igual al del hueso.
					//glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &(manny_world_transform * mag_rifle_transform)[0][0]);

					//glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &( assault_rifle_transform * skel_assault_rifle_transform * grip_transform * mag_transform)[0][0]);

					// skel_assault_rifle_transform este creo que no hace falta en el calculo porque representaba la posicion en el mundo y eso ya esta dado por `assault_rifle_transform`
					glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, &(assault_rifle_transform * mag_bone_transform)[0][0]);
			}

			glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
			glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);
			glBindVertexArray(0);
		}
	}
	for (auto& child : node->children) {
		render_assimp_node(child, skinning_shader, regular_shader);
	}
}

int main() {
	SceneGraph scene_graph{};
	tinygltf::Model model = loadGLTFModel();

	/*
	A_FP_AssaultRifle_Fire
	Aug 10 2024 18:20:35 [INFO] \src\main.cpp:1452: A_FP_AssaultRifle_Idle_Loop
	Aug 10 2024 18:20:35 [INFO] \src\main.cpp:1452: A_FP_AssaultRifle_Idle_Pose
	Aug 10 2024 18:20:35 [INFO] \src\main.cpp:1452: A_FP_AssaultRifle_Walk_F_Loop
	Aug 10 2024 18:20:35 [INFO] \src\main.cpp:1452: A_Reference
	Aug 10 2024 18:20:35 [INFO] \src\main.cpp:1452: A_FP_WEP_AssaultRifle_Reload
	Aug 10 2024 18:20:35 [INFO] \src\main.cpp:1452: A_WEP_Reference
	*/


	projectiles.reserve(100);
	//game game_inst;
	//create_game(&game_inst);

	//game_inst.init(&game_inst);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "aim engine", NULL, NULL);
	//GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "aim", glfwGetPrimaryMonitor(), NULL);



	//const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	//glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	//glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	//glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	//glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	//GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "My Title", glfwGetPrimaryMonitor(), NULL);



	INFO("GLFW window created successfully!");
	if (window == NULL)
	{
		FATAL("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		FATAL("Failed to initialize GLAD");
		return -1;
	}
	INFO("OpenGL initialized successfully!");

	/*
	Explicacion:
	La magazine queda en la posicion del arma pero no tiene nada que ver con los bones. La razon por la cual queda ahi es porque
	tiene una transformacion y cuando se le aplica funciona. Ya que en este proyecto cuando exporte GLTF deje un offset en la magazine y ademas segun el dibujo que
	hice en la tablet los calculos que suceden en update() no afectan a esta visualizacion del arma correcta (obvio todo esto sin animaciones)

	Ahora si importo cada archivo por separado con su propio .FBX la magazine queda mal porque el tipo antes de exportar setteo la magazine en 0,0,0 y la dejo offseteada
	cuando hacia las animaciones.

	Conclusion: Si comento el update(), just para este .blend porque los parents tienen la identidad como transform, la magazine va a quedar bien si cargo el .gltf (porque no esta offseteado)
	y mal si meto el .fbx (porque se centro en 0,0,0 antes de exportar)
	Nota: si se comenta el update() hay que pasar node->GetLocalMatrix a "nodeMatrix" ya que no se setteo en el update() y sino es undefined
	*/
	INFO("Loading assimp models...\n");
	AssimpNode assault_rifle, assault_rifle_magazine, assault_rifle_casing;
	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/RIG_AssaultRifle.gltf"));
	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SK_AssaultRifle.fbx"));
	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle_magazine, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SM_AssaultRifle_Magazine.fbx"));
	//loadAssimp(&assault_rifle_casing, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SM_AssaultRifle_Casing.fbx");
	//load_assimp_anim(std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/Animations/A_FP_AssaultRifle_Idle_Pose.fbx");
	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/dancing_vampire.dae"));
	//Animation danceAnimation(std::string(AIM_ENGINE_ASSETS_PATH) + "models/dancing_vampire.dae");
	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/gusano2.glb"));
	//Animation danceAnimation(std::string(AIM_ENGINE_ASSETS_PATH) + "models/gusano2.glb");

	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SK_AssaultRifle.fbx"));
	//Animation danceAnimation(std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/Animations/A_FP_WEP_AssaultRifle_Fire.fbx");

	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SK_FP_Manny_Simple.fbx"));
	//Animation danceAnimation(std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/Animations/A_FP_AssaultRifle_Fire.fbx");

	scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SK_FP_Manny_Simple.fbx"));
	//scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SK_FP_Manny_Simple_Y_UP.fbx"));
	std::cout << "Finished loading Manny" << std::endl;
	scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SK_AssaultRifle.fbx"));
	scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SM_AssaultRifle_Magazine.fbx"));
	scene_graph.nodes.push_back(loadAssimp(&assault_rifle, std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/SM_AssaultRifle_Casing.fbx"));
	Animation danceAnimation(std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/Animations/A_FP_AssaultRifle_Reload.fbx", 0);
	Animation mag_anim(std::string(AIM_ENGINE_ASSETS_PATH) + "models/Unreal/Animations/A_FP_WEP_AssaultRifle_Reload.fbx", 2);


	Animator animator(&danceAnimation);
	Animator mag_animator(&mag_anim);
	//Animator animator(&danceAnimation);



	size_t vertexCount = 0, indexCount = 0;
	const tinygltf::Scene& scene = model.scenes[0];
	for (size_t i = 0; i < scene.nodes.size(); i++) {
		getNodeProps(model.nodes[scene.nodes[i]], model, vertexCount, indexCount);
	}
	std::vector<GLTFMesh> meshes;
	Info info{};
	info.vertexBuffer = new Vertex[vertexCount];
	info.indexBuffer = new uint32_t[indexCount];
	info.vertex_pos = 0;
	info.index_pos = 0;

#ifndef PART_1

	for (size_t i = 0; i < scene.nodes.size(); i++) {
		const tinygltf::Node& node = model.nodes[scene.nodes[i]];
		load_node(node, nullptr, scene.nodes[i], model, info);
	}
	if (model.animations.size() > 0) {
		load_animations(model);
	}
	load_skins(model);
	for (auto node : linearNodes) {
		if (node->skinIndex > -1) {
			node->skin = skins[node->skinIndex];
		}
		if (node->mesh) {
			node->update();
		}
	}
#endif

	for (const auto& gltfMesh : model.meshes) {
		// En los ejemplos que he visto solo hay un mesh en el gltf
		// meshes.push_back(createMesh(model, gltfMesh));
	}

#pragma region imgui
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
#pragma endregion imgui

#pragma region p2_physics_engine

	// this is a must
	JPH::RegisterDefaultAllocator();
	PhysicsSystem physics_system{};
	// Set up the context
	Context context;
	context.physics_system = &physics_system;

	// Set the user pointer
	glfwSetWindowUserPointer(window, &context);
	//PhysicsSystem& physics_system = PhysicsSystem::getInstance();



	//JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(10.0f, 1.0f, 10.0f));
	//JPH::BodyID my_floor = physics_system.create_body(Transform3D(glm::vec3(0.0, -1.0, 0.0)), floor_shape, true);
	//JPH::BodyID my_floor = physics_system.create_body(Transform3D(glm::vec3(0.0, 8.0, 0.0)), new JPH::BoxShape(JPH::Vec3(10.0f, 1.0f, 10.0f)), true);

	//MeshBox floor_meshbox = MeshBox(Transform3D(glm::vec3(0.0, -1.0, 0.0)));

	// SEE what happens if I move the static body (the floor) and see if the ball follows it
	// todo: JPH::BodyID my_floor = physics_system.create_body(floor_meshbox.transform.pos, floor_meshbox.physics_body->shape, true);


	//JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(1.5f), JPH::RVec3(0.0_r, 15.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
	//sphere_settings.mMassPropertiesOverride = JPH::MassProperties{ .mMass = 40.0f };
	//Transform3D sphere_transform = Transform3D(glm::vec3(0.0, 15.0, 0.0));
	//JPH::BodyID my_sphere = physics_system.create_body(&sphere_transform, new JPH::SphereShape(1.5f), false);
	//physics_system.get_body_interface().GetShape(my_sphere);
	//physics_system.get_body_interface().SetLinearVelocity(my_sphere, JPH::Vec3(0.0f, -2.0f, 0.0f));
	//physics_system.get_body_interface().SetRestitution(my_sphere, 0.5f);

	physics_system.inner_physics_system.OptimizeBroadPhase();

#pragma endregion p2_physics_engine


#pragma region renderer

	// build and compile our shader zprogram
	 // ------------------------------------
	//Shader lightingShader("5.2.light_casters.vs", "5.2.light_casters.fs");
	//Shader lightingShader("5.1.light_casters.vs", "5.1.light_casters.fs");
	//Shader lightingShader("1.colors.vs", "1.colors.fs");
	Shader lightingShaderGouraud("gouraud.vs", "gouraud.fs");
	Shader lightCubeShader("1.light_cube.vs", "1.light_cube.fs");
	Shader skinning_shader("skel_shader-2.vs.glsl", "6.multiple_lights.fs.glsl");
#ifdef PART_1
	Shader skel_shader("skel_shader-anton-part-1.vs.glsl", "skel_shader-anton-part-1.fs.glsl");
#else
	Shader skel_shader("skel_shader.vs.glsl", "skel_shader.fs.glsl");
#endif
	skel_id = skel_shader.ID;
	skinning_shader_id = skinning_shader.ID;



	Shader Raycast("line_shader.vs", "line_shader.fs");


	float vertices1[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	float vertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,	0.0f,  0.0f, -1.0f,			0.0f, 0.0f, // Bottom-left
		 0.5f,  0.5f, -0.5f,	0.0f,  0.0f, -1.0f,			1.0f, 1.0f, // top-right
		 0.5f, -0.5f, -0.5f,	0.0f,  0.0f, -1.0f,			1.0f, 0.0f, // bottom-right         
		 0.5f,  0.5f, -0.5f,	0.0f,  0.0f, -1.0f,			1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,	0.0f,  0.0f, -1.0f,			0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,	0.0f,  0.0f, -1.0f,			0.0f, 1.0f, // top-left
		// Front face        
		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,			0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,			1.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f,  1.0f,			1.0f, 1.0f, // top-right
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f,  1.0f,			1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,	0.0f,  0.0f,  1.0f,			0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,			0.0f, 0.0f, // bottom-left
		// Left face         
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,			1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,			0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,			0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,			1.0f, 0.0f, // top-right
		// Right face        
		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,			1.0f, 0.0f, // top-left
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,			0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,	1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // top-right         
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,			0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,			1.0f, 0.0f, // top-left
		 0.5f, -0.5f,  0.5f,	1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // bottom-left     
		 // Bottom face       
		 -0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,			0.0f, 1.0f, // top-right
		  0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,			1.0f, 1.0f, // top-left
		  0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,			1.0f, 0.0f, // bottom-left
		  0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,			1.0f, 0.0f, // bottom-left
		 -0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,			0.0f, 0.0f, // bottom-right
		 -0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,			0.0f, 1.0f, // top-right
		 // Top face          
		 -0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,			0.0f, 1.0f, // top-left
		  0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,			1.0f, 0.0f, // bottom-right
		  0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,			1.0f, 1.0f, // top-right     
		  0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,			1.0f, 0.0f, // bottom-right
		 -0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,			0.0f, 1.0f, // top-left
		 -0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,			0.0f, 0.0f  // bottom-left        
	};



	std::vector<MeshBox> boxes = {

		///////
		MeshBox(Transform3D(glm::vec3(-3.0f,  1.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(-2.0f,  1.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  1.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  1.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f, 1.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(2.0f, 1.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(3.0f, 1.0f, -5.0f))),

		MeshBox(Transform3D(glm::vec3(-3.0f,  2.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(-2.0f,  2.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  2.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  2.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f, 2.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(2.0f, 2.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(3.0f, 2.0f, -5.0f))),

		MeshBox(Transform3D(glm::vec3(-3.0f,  3.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(-2.0f,  3.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  3.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  3.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f, 3.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(2.0f, 3.0f, -5.0f))),
		MeshBox(Transform3D(glm::vec3(3.0f, 3.0f, -5.0f))),

		///////


		MeshBox(Transform3D(glm::vec3(0.0f,  13.0f,  0.0f))),
		MeshBox(Transform3D(glm::vec3(5.0f,  6.0f, -10.0f))),
		MeshBox(Transform3D(glm::vec3(-1.5f, 5.2f, -2.5f))),
		MeshBox(Transform3D(glm::vec3(-9.8f, 5.0f, -12.3f))),
		MeshBox(Transform3D(glm::vec3(2.4f, 3.4f, -3.5f))),
		MeshBox(Transform3D(glm::vec3(-8.7f,  6.0f, -7.5f))),
		MeshBox(Transform3D(glm::vec3(3.3f, 5.0f, -2.5f))),
		MeshBox(Transform3D(glm::vec3(1.5f,  5.0f, -2.5f))),
		MeshBox(Transform3D(glm::vec3(8.5f,  8.2f, -1.5f))),
		MeshBox(Transform3D(glm::vec3(-8.3f,  4.0f, -1.5f))),


		// a pile of boxes
		MeshBox(Transform3D(glm::vec3(0.0f,  20.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  22.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  24.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f,  26.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  27.0f, 0.0f))),


		MeshBox(Transform3D(glm::vec3(0.0f,  30.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  31.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  32.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f,  33.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  34.0f, 0.0f))),

		MeshBox(Transform3D(glm::vec3(0.0f,  30.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  31.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  32.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f,  33.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  34.0f, 0.0f))),

		MeshBox(Transform3D(glm::vec3(0.0f,  30.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  31.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  32.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f,  33.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  34.0f, 0.0f))),

		MeshBox(Transform3D(glm::vec3(0.0f,  30.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  31.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  32.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f,  33.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  34.0f, 0.0f))),

		MeshBox(Transform3D(glm::vec3(0.0f,  30.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  31.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(-1.0f,  32.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(1.0f,  33.0f, 0.0f))),
		MeshBox(Transform3D(glm::vec3(0.0f,  34.0f, 0.0f))),

	};

	// Blender and JoltPhysics specify the scale of a cube from the middle to sides. So a scale of (1, 1, 1) means from the center to side is 1 unit.
	// In Godot is different, a scale of (1, 1, 1) means for side to side the length is 1.

	// Grid in Blender and Godot are the same size, 1 unit in length
	for (int i = 0; i < boxes.size(); i++) {
		JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(boxes[i].transform.scale.x / 2.0, boxes[i].transform.scale.y / 2.0, boxes[i].transform.scale.z / 2.0));
		floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
		JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		JPH::Ref<JPH::Shape> shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
		boxes[i].set_shape(shape);
		boxes[i].body.physics_body_id = physics_system.create_body(&boxes[i].transform, boxes[i].body.shape, false);
	}


	// floor
	JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(floor_scale.x / 2.0, floor_scale.y / 2.0, floor_scale.z / 2.0));
	floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
	JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	JPH::Ref<JPH::Shape> floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

	MeshBox floor_meshbox = MeshBox(Transform3D(glm::vec3(floor_pos.x, floor_pos.y, floor_pos.z), glm::vec3(floor_scale.x, floor_scale.y, floor_scale.z)));

	floor_meshbox.set_shape(floor_shape); // the problem of storing the shape is that this can change, in theory... but not for now at least
	floor_meshbox.body.physics_body_id = physics_system.create_body(&floor_meshbox.transform, floor_meshbox.body.shape, true);


#pragma region l2_LIGHT_DEFINITION
	PointLight point_lights[] = {
		PointLight{
			.transform = Transform3D(glm::vec3(0.7f,  0.2f,  2.0f)),
			.ambient = glm::vec3(0.05f, 0.05f, 0.05f),
			.diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
			.specular = glm::vec3(1.0f, 1.0f, 1.0f),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
		},
		PointLight{
			.transform = Transform3D(glm::vec3(2.3f,  -3.3f,  -4.0f)),
			.ambient = glm::vec3(0.05f, 0.05f, 0.05f),
			.diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
			.specular = glm::vec3(1.0f, 1.0f, 1.0f),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
		},
		PointLight{
			.transform = Transform3D(glm::vec3(-4.0f,  2.0f,  -12.0f)),
			.ambient = glm::vec3(0.05f, 0.05f, 0.05f),
			.diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
			.specular = glm::vec3(1.0f, 1.0f, 1.0f),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
		},
		PointLight{
			.transform = Transform3D(glm::vec3(0.0f,  0.0f,  -3.0f)),
			.ambient = glm::vec3(0.05f, 0.05f, 0.05f),
			.diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
			.specular = glm::vec3(1.0f, 1.0f, 1.0f),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
		},
	};

	DirectionalLight directional_light{
		.direction = glm::vec3(-0.2f, -1.0f, -0.3f),
		.ambient = glm::vec3(0.05f, 0.05f, 0.05f),
		.diffuse = glm::vec3(0.4f, 0.4f, 0.4f),
		.specular = glm::vec3(0.5f, 0.5f, 0.5f)
	};

	SpotLight spot_light{
		.ambient = glm::vec3(0.0f, 0.0f, 0.0f),
		.diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
		.specular = glm::vec3(1.0f, 1.0f, 1.0f),
		.constant = 1.0f,
		.linear = 0.09f,
		.quadratic = 0.032f,
		.cutOff = glm::cos(glm::radians(12.5f)),
		.outerCutOff = glm::cos(glm::radians(15.0f)),
	};


#pragma endregion l2_LIGHT_DEFINITION

	unsigned int vbo_ray, vao_ray;
	glGenVertexArrays(1, &vao_ray);
	glGenBuffers(1, &vbo_ray);
	//glBindVertexArray(vao_ray);


	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);


	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int diffuseMap = loadTexture("container2.png");
	unsigned int specularMap = loadTexture("container2_specular.png");
	// render loop
	// -----------
	glm::vec3 model_color = glm::vec3(1.0f, 0.5f, 0.31f);

	float theta = 0.0f;
	float rot_speed = 1.0f;
	int bone_matrices_locations[32];
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);


#pragma region render
		//glClearColor(1.1f, 0.1f, 0.1f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		//glFrontFace(GL_CCW);

		glLineWidth(2.0f);



		// view/projection transformations
		glm::mat4 projection;
		if (!fps_mode) {
			projection = glm::perspective(glm::radians(free_camera.zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 10000.0f);
		}
		else {
			projection = glm::perspective(glm::radians(fps_camera.zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 10000.0f);

		}
		glm::mat4  view;
		if (!fps_mode) {
			view = free_camera.GetViewMatrix();
		}
		else {
			view = fps_camera.GetViewMatrix();
		}
#if 1



#else

		lightingShaderGouraud.use();
		lightingShaderGouraud.setVec3("objectColor", model_color.r, model_color.g, model_color.b);
		lightingShaderGouraud.setVec3("lightColor", light_color.r, light_color.g, light_color.b);
		lightingShaderGouraud.setVec3("lightPos", light_pos.r, light_pos.g, light_pos.b);
		lightingShaderGouraud.setVec3("viewPos", camera.Position); lightPos

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShaderGouraud.setMat4("projection", projection);
		lightingShaderGouraud.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, model_pos);
		model = glm::scale(model, model_scale); // a smaller cube
		lightingShaderGouraud.setMat4("model", model);

#endif


		// skere
#if 0
		skel_shader.use();

		float identity[] = {
			1.0f, 0.0f, 0.0f, 0.0f, // first column
			0.0f, 1.0f, 0.0f, 0.0f, // second column
			0.0f, 0.0f, 1.0f, 0.0f, // third column
			0.0f, 0.0f, 0.0f, 1.0f //
		};

		char name[64];

		for (int i = 0; i < 32; i++) {
			sprintf(name, "bone_matrices[%i]", i);
			bone_matrices_locations[i] = glGetUniformLocation(skel_shader.ID, name);
			glUniformMatrix4fv(bone_matrices_locations[i], 1, GL_FALSE, identity);
		}

		glm::mat4 left_ear_mat = glm::mat4(1.0f);

		if (glfwGetKey(window, 'Z') == GLFW_PRESS) {
			std::cout << "zzzz";
			theta += rot_speed * deltaTime;
			left_ear_mat = glm::inverse(mats[1]) * glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0, 1.0, 0.0)) * mats[1];
			glUniformMatrix4fv(bone_matrices_locations[1], 1, GL_FALSE, &left_ear_mat[0][0]);

			left_ear_mat = glm::inverse(mats[2]) * glm::rotate(glm::mat4(1.0f), -theta, glm::vec3(0.0, 1.0, 0.0)) * mats[2];
			glUniformMatrix4fv(bone_matrices_locations[2], 1, GL_FALSE, &left_ear_mat[0][0]);
		}

		if (glfwGetKey(window, 'X') == GLFW_PRESS) {
			std::cout << "xxxx";
			theta -= rot_speed * deltaTime;
			left_ear_mat = glm::inverse(mats[1]) * glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0, 1.0, 0.0)) * mats[1];
			glUniformMatrix4fv(bone_matrices_locations[1], 1, GL_FALSE, &left_ear_mat[0][0]);
			left_ear_mat = glm::inverse(mats[2]) * glm::rotate(glm::mat4(1.0f), -theta, glm::vec3(0.0, 1.0, 0.0)) * mats[2];
			glUniformMatrix4fv(bone_matrices_locations[2], 1, GL_FALSE, &left_ear_mat[0][0]);
		}

		if (glfwGetKey(window, 'X') == GLFW_RELEASE && glfwGetKey(window, 'Z') == GLFW_RELEASE) {
			theta = 0.0f;
		}

		skel_shader.setMat4("projection", projection);
		skel_shader.setMat4("view", view);

		for (const auto& mesh : meshes) {
			glBindVertexArray(mesh.vao);

			if (mesh.materialId != -1) {
				const Material& material = materials[mesh.materialId];
			}

			glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

			skel_shader.setMat4("model", model_mat);

			glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
	}
#endif

#if 1
		skinning_shader.use();
#pragma region LIGHT_RENDERING

		skinning_shader.setMat4("nodeMatrix", glm::mat4(1.0f));

		skinning_shader.setVec3("objectColor", model_color.r, model_color.g, model_color.b);
		skinning_shader.setInt("material.diffuse", 0);
		skinning_shader.setInt("material.specular", 1);
		skinning_shader.setFloat("material.shininess", model_material_shininess);

		if (!fps_mode) {
			skinning_shader.setVec3("viewPos", free_camera.position);
			skinning_shader.setVec3("spotLight.position", free_camera.position);
			skinning_shader.setVec3("spotLight.direction", free_camera.forward);
		}
		else {
			skinning_shader.setVec3("viewPos", fps_camera.position);
			skinning_shader.setVec3("spotLight.position", fps_camera.position);
			skinning_shader.setVec3("spotLight.direction", fps_camera.forward);
		}

		// directional_light
		skinning_shader.setVec3("dirLight.direction", directional_light.direction);
		skinning_shader.setVec3("dirLight.ambient", directional_light.ambient);
		skinning_shader.setVec3("dirLight.diffuse", directional_light.diffuse);
		skinning_shader.setVec3("dirLight.specular", directional_light.specular);

		// point_lights
		int n = sizeof(point_lights) / sizeof(point_lights[0]);
		for (int i = 0; i < n; i++) {
			std::string prefix = "pointLights[" + std::to_string(i) + "]";

			skinning_shader.setVec3(prefix + ".position", point_lights[i].transform.pos);
			skinning_shader.setVec3(prefix + ".ambient", point_lights[i].ambient);
			skinning_shader.setVec3(prefix + ".diffuse", point_lights[i].diffuse);
			skinning_shader.setVec3(prefix + ".specular", point_lights[i].specular);
			skinning_shader.setFloat(prefix + ".constant", point_lights[i].constant);
			skinning_shader.setFloat(prefix + ".linear", point_lights[i].linear);
			skinning_shader.setFloat(prefix + ".quadratic", point_lights[i].quadratic);
		}

		// spot_light
		skinning_shader.setVec3("spotLight.ambient", spot_light.ambient);
		skinning_shader.setVec3("spotLight.diffuse", spot_light.diffuse);
		skinning_shader.setVec3("spotLight.specular", spot_light.specular);
		skinning_shader.setFloat("spotLight.constant", spot_light.constant);
		skinning_shader.setFloat("spotLight.linear", spot_light.linear);
		skinning_shader.setFloat("spotLight.quadratic", spot_light.quadratic);
		skinning_shader.setFloat("spotLight.cutOff", spot_light.cutOff);
		skinning_shader.setFloat("spotLight.outerCutOff", spot_light.outerCutOff);
		skinning_shader.setMat4("projection", projection);
		skinning_shader.setMat4("view", view);
		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
#pragma endregion LIGHT_RENDERING
#pragma region CUBE_OBJECT
		// render the cube
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < boxes.size(); i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			//glm::mat4 model = glm::mat4(1.0f);
			//model = glm::translate(model, boxes[i].transform.pos);
			//float angle = 20.0f * i;
			//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f)); // take rotation out for testing

			// with rotations
			glm::mat4 model = glm::translate(glm::mat4(1.0f), boxes[i].transform.pos) *
				glm::mat4_cast(boxes[i].transform.rot) *
				glm::scale(glm::mat4(1.0f), boxes[i].transform.scale);
			skinning_shader.setMat4("model", model);

			//skinning_shader.setMat4("nodeMatrix", glm::mat4(1.0f));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// projectiles
		for (unsigned int i = 0; i < projectiles.size(); i++)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), projectiles[i].transform.pos) *
				glm::mat4_cast(projectiles[i].transform.rot) *
				glm::scale(glm::mat4(1.0f), projectiles[i].transform.scale);
			skinning_shader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		// render floor, is just a plane

		// position * scale
		//glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), floor_meshbox.transform.pos), floor_meshbox.transform.scale);
		// positoin * rotation * scale
		//glm::mat4 model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), floor_meshbox.transform.pos), glm::radians(floor_meshbox.transform.rot.x), glm::vec3(1.0f, 0.0f, 0.0f)), floor_meshbox.transform.scale);
		//glm::mat4 model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), floor_meshbox.transform.pos), glm::radians(floor_meshbox.transform.rot.x), glm::vec3(1.0f, 0.0f, 0.0f)), floor_meshbox.transform.scale);
		glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), floor_meshbox.transform.pos) *
			glm::mat4_cast(floor_meshbox.transform.rot) *
			glm::scale(glm::mat4(1.0f), floor_meshbox.transform.scale);


		skinning_shader.setMat4("model", model_mat);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// render floor, is just a plane


#pragma endregion CUBE_OBJECT

#pragma region NODE_RENDERING
		if (animationActive && animations.size() > 0) {
			animationTimer += deltaTime;
			if (animationTimer > animations[activeAnimationIndex].end) {
				animationTimer -= animations[activeAnimationIndex].end;
			}
			updateAnimation(activeAnimationIndex, animationTimer);
		}
		//for (size_t i = 0; i < animations.size(); i++) {
		//	GLTFAnimation& anim = animations[i];
		//	std::cout << "Animation name: " << anim.name << ", index: " << i << std::endl;
		//}
		/*
			Animation name: A_FP_AssaultRifle_Fire, index: 0
			Animation name: A_FP_AssaultRifle_Idle_Loop, index: 1
			Animation name: A_FP_AssaultRifle_Idle_Pose, index: 2
			Animation name: A_FP_AssaultRifle_Walk_F_Loop, index: 3
			Animation name: A_Reference, index: 4
			Animation name: A_FP_WEP_AssaultRifle_Reload, index: 5
			Animation name: A_WEP_Reference, index: 6
		*/

		skinning_shader.use();
		//skinning_shader.setMat4("nodeMatrix", glm::mat4(1.0f));
		//glUseProgram(skinning_shader_id);
		//glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);
		for (auto& node : nodes) {
			//render_node(node, &skinning_shader, &skel_shader);
		}
		//skinning_shader.setMat4("nodeMatrix", glm::mat4(1.0f));
		//glUseProgram(skinning_shader_id);
		//glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);

		if (animationActive) {
			animator.UpdateAnimation(deltaTime);
			mag_animator.UpdateAnimation(deltaTime);
			std::cout << "MANNY ANIM BONE NAMES: " << std::endl;
			for (auto& bone : animator.m_CurrentAnimation->m_Bones) {
				std::cout << bone.GetBoneName() << std::endl;
				if (bone.GetBoneName() == "Armature") {
					manny_armature = bone.GetLocalTransform();
					print_matrix(manny_armature);
				}
				if (bone.GetBoneName() == "root") {
					root = &bone;
				}
				if (bone.GetBoneName() == "ik_hand_root") {
					ik_hand_root = &bone;
				}
				if (bone.GetBoneName() == "ik_hand_gun") {
					ik_hand_gun_bone = &bone;
				}
			}
			std::cout << "END BONE NAMES: \n" << std::endl;
			std::cout << "MAG ANIM BONE NAMES: " << std::endl;
			for (auto& bone : mag_animator.m_CurrentAnimation->m_Bones) {
				std::cout << bone.GetBoneName() << std::endl;
				if (bone.GetBoneName() == "Grip") {
					print_matrix(bone.GetLocalTransform());
				}
				if (bone.GetBoneName() == "Magazine") {
					mag_bone_transform = bone.GetLocalTransform();
					print_matrix(mag_bone_transform);
				}
				//if (bone.GetBoneName() == "root") {
				//	root = &bone;
				//}
				//if (bone.GetBoneName() == "ik_hand_root") {
				//	ik_hand_root = &bone;
				//}
				//if (bone.GetBoneName() == "ik_hand_gun") {
				//	ik_hand_gun_bone = &bone;
				//}
			}
			std::cout << "END BONE NAMES: " << std::endl;
		}
		// RootNode (este no va, no es un hueso) - Armature -> root -> ik_hand_root -> ik_hand_gun
		// RootNode (este no va, no es un hueso) - SKEL_AssaultRifle -> Grip -> Magazine
		int index = skeletons[animator.m_CurrentAnimation->m_skeleton_index].m_BoneInfoMap["ik_hand_gun"].id;
		ik_something = animator.m_FinalBoneMatrices[index];
		std::cout << "IK_HAND_GUN transform" << std::endl;
		print_matrix(ik_hand_gun_bone->GetLocalTransform());
		print_matrix(ik_something);
		todas_las_putas_transforms = manny_armature * root->GetLocalTransform() * ik_hand_root->GetLocalTransform() * ik_hand_gun_bone->GetLocalTransform();
		print_matrix(
			manny_armature *
			root->GetLocalTransform() *
			ik_hand_root->GetLocalTransform() *
			ik_hand_gun_bone->GetLocalTransform()
		);
		//assert(ik_hand_gun_bone->GetLocalTransform() == root->GetLocalTransform() * ik_hand_root->GetLocalTransform() * ik_hand_gun_bone->GetLocalTransform() );

		/* En UNREAL queda:
			root
				ik_hand_root
					ik_hand_gun
						SK_AssaultRifle (este es el skeleton)
						ik_hand_l
						ik_hand_r


		*/

		skinning_shader.use();

		// esto tiene que ser por modelo porque todos comparten el mismo shader
		// si dejo la primer linea rompo lo que tiene jointMatrices pero no esta bajo la animacion
		manny_transforms = animator.GetFinalBoneMatrices();

		mag_transforms = mag_animator.GetFinalBoneMatrices();

		for (auto& scene : scene_graph.nodes) {
			for (auto& node : scene.assimp_nodes) {
				render_assimp_node(node, &skinning_shader, &skel_shader);
			}
		}
		std::cout << "Armature count: " << armature_count << std::endl;
		manny_transforms.clear();
		armature_count = 0;

		// assimp
		/*
		if (assault_rifle.mesh) {
			for (auto mesh : assault_rifle.mesh->meshes) {
				glBindVertexArray(mesh->vao);

				skinning_shader.use();
				glm::quat qx = glm::angleAxis(glm::radians(00.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qy = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qz = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot = qz * qy * qx; // Specify order of rotations here

				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -30.0f, 0.0f)) *
					glm::mat4_cast(rot);
				//glm::scale(glm::mat4(1.0f), glm::vec3(0.0125f));
				skinning_shader.setMat4("model", base_model_mat);
				skinning_shader.setMat4("model", glm::mat4(1.0f));

				glUseProgram(skinning_shader_id);
				GLint jointMatricesLoc = glGetUniformLocation(skinning_shader_id, "jointMatrices");
				glUniformMatrix4fv(jointMatricesLoc, 0, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)[0]));

				glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);

				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

				glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

			}

		}

		if (assault_rifle_magazine.mesh) {
			for (auto mesh : assault_rifle_magazine.mesh->meshes) {
				glBindVertexArray(mesh->vao);

				skinning_shader.use();
				glm::quat qx = glm::angleAxis(glm::radians(00.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qy = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qz = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot = qz * qy * qx; // Specify order of rotations here

				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -40.0f, 0.0f)) *
					glm::mat4_cast(rot);
				//glm::scale(glm::mat4(1.0f), glm::vec3(0.0125f));
				//skinning_shader.setMat4("model", base_model_mat);
				skinning_shader.setMat4("model", finalMagazineTransform);

				glUseProgram(skinning_shader_id);
				GLint jointMatricesLoc = glGetUniformLocation(skinning_shader_id, "jointMatrices");
				glUniformMatrix4fv(jointMatricesLoc, 0, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)[0]));

				glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);

				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

				glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

			}
		}


		if (assault_rifle_casing.mesh) {
			for (auto mesh : assault_rifle_casing.mesh->meshes) {
				glBindVertexArray(mesh->vao);

				skinning_shader.use();
				glm::quat qx = glm::angleAxis(glm::radians(00.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qy = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat qz = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				glm::quat rot = qz * qy * qx; // Specify order of rotations here

				glm::mat4 base_model_mat =
					glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, -50.0f, 0.0f)) *
					glm::mat4_cast(rot);
				//glm::scale(glm::mat4(1.0f), glm::vec3(0.0125f));
				skinning_shader.setMat4("model", base_model_mat);

				glUseProgram(skinning_shader_id);
				GLint jointMatricesLoc = glGetUniformLocation(skinning_shader_id, "jointMatrices");
				glUniformMatrix4fv(jointMatricesLoc, 0, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)[0]));

				glUniform1i(glGetUniformLocation(skinning_shader_id, "jointCount"), 0);

				glUniformMatrix4fv(glGetUniformLocation(skinning_shader_id, "nodeMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

				glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}
		*/
		// assimp



#pragma endregion NODE_RENDERING
#endif





#pragma region LAMP_OBJECT
		// also draw the LAMP object
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		glm::mat4 light_model = glm::mat4(1.0f);

		glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), light_scale); // a smaller cube
		glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), float(glfwGetTime()), glm::vec3(1.0, 0.0, 0.0));
		glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), light_pos);

		glm::mat4 some_rot = glm::rotate(glm::mat4(1.0f), 0 * float(deltaTime), glm::vec3(0.0, 1.0, 0.0));
		light_model = some_rot * translation_matrix * rotation_matrix * scale_matrix;

		lightCubeShader.setMat4("model", light_model);

		// Just a visual representation, could be hardcoded in the fs
		light_pos = glm::vec3(light_model[3]);
		lightCubeShader.setVec3("lightColor", glm::vec3(1.0f));
		glBindVertexArray(lightCubeVAO);
		for (unsigned int i = 0; i < 4; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, point_lights[i].transform.pos);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			lightCubeShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//glDrawArrays(GL_TRIANGLES, 0, 36);




#pragma endregion LAMP_OBJECT


#pragma region r22_RAYCAST
		Raycast.use();

		Raycast.setMat4("projection", projection);
		Raycast.setMat4("view", view);
		Raycast.setMat4("model", glm::mat4(1.0f));
		JPH::Color line_color = physics_system.debugRenderer.lines_color;
		Raycast.setVec4("line_color", line_color.r, line_color.g, line_color.b, line_color.a);



		std::vector<float> raycast_to_render{};
		for (int i = 0; i < physics_system.debugRenderer.ray_cast_list.size(); i++) {
			glm::vec3 ro = physics_system.debugRenderer.ray_cast_list[i].ro;
			glm::vec3 rd = physics_system.debugRenderer.ray_cast_list[i].rd;

			raycast_to_render.push_back(ro.x);
			raycast_to_render.push_back(ro.y);
			raycast_to_render.push_back(ro.z);
			raycast_to_render.push_back(rd.x);
			raycast_to_render.push_back(rd.y);
			raycast_to_render.push_back(rd.z);

		}

		//debugRenderer.mCameraPos = JPH::Vec3(free_camera.position.x, free_camera.position.y, free_camera.position.z);
		physics_system.set_debug_camera_pos(curr_camera.position);

		// This are my manual rays when I press R
		// this has been disabled for now.
		//for (int i = 0; i < ray_cast_list.size(); i++) {
		//	glm::vec3 ro = ray_cast_list[i].ro;
		//	glm::vec3 rd = ray_cast_list[i].rd;

		//	raycast_to_render.push_back(ro.x);
		//	raycast_to_render.push_back(ro.y);
		//	raycast_to_render.push_back(ro.z);
		//	raycast_to_render.push_back(rd.x);
		//	raycast_to_render.push_back(rd.y);
		//	raycast_to_render.push_back(rd.z);
		//}

		if (!raycast_to_render.empty()) {

			glBindVertexArray(vao_ray);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_ray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * raycast_to_render.size(), &raycast_to_render[0], GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glDrawArrays(GL_LINES, 0, raycast_to_render.size() / 6 * 2);
		}
		physics_system.debugRenderer.ray_cast_list.clear();


#pragma endregion r22_RAYCAST


		update_physics(deltaTime);

		physics_system.update_physics(deltaTime);

		//physics_system.DrawBodies(
		//	JPH::BodyManager::DrawSettings{
		//		.mDrawShape = true,
		//		.mDrawShapeWireframe = true,
		//		//.mDrawBoundingBox = true,
		//	}, &debugRenderer
		//	);



		glfwPollEvents();

#pragma region IMGUI_RENDERING
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Demo Window useful for inspecting its implementation
		//ImGui::ShowDemoWindow();

		// Start the parent window
		//ImGui::Begin("Parent Window");

		//// Number of objects or child windows
		//int numberOfObjects = 5;

		// Iterate through each object and create a child window for it
		//for (int i = 0; i < numberOfObjects; i++) {
		//    // Define a unique ID for each child based on the object index
		//    std::string child_id = "Object " + std::to_string(i);

		//    // Start the child window
		//    if (ImGui::BeginChild(child_id.c_str(), ImVec2(0, 100), true)) {
		//        ImGui::Text("Content of Object %d", i);

		//		ImGui::ColorPicker3("Something", reinterpret_cast<float*>(&light_color));
		//		ImGui::CollapsingHeader("Expanded Header", ImGuiTreeNodeFlags_DefaultOpen);
		//		ImGui::Text("This header is expanded by default.");
		//        // Add more UI elements related to the object here
		//        // For example, sliders, buttons, etc.
		//    }
		//    ImGui::EndChild(); // End the child window

		//    // Optionally, add some space between child windows
		//    ImGui::Spacing();
		//}
		//ImGui::End(); // End the parent window

		ImGuiWindowFlags global_flags = ImGuiWindowFlags_None;
		if (!gui_mode) {
			global_flags = ImGuiWindowFlags_NoInputs;
		}


		ImGui::Begin("Physics", nullptr, global_flags);
		ImGui::Checkbox("Enable physics", &physics_system.is_running);
		ImGui::Checkbox("Enable collision shape", &physics_system.debug_bodies);
		ImGui::End();

		// this could be: Mesh.render_gui()
		ImGui::Begin("Model", nullptr, global_flags);
		ImGui::Checkbox("Animation mode", &animationActive);
		ImGui::InputInt("active anim index", &activeAnimationIndex);
		ImGui::Text("Animation name: %s", animations[activeAnimationIndex].name.c_str());
		ImGui::Text("Number of animations: %d", animations.size());

		if (ImGui::CollapsingHeader("model transform", ImGuiTreeNodeFlags_DefaultOpen)) {
			//ImGui::DragFloat3("model pos", glm::value_ptr(floor_pos), 0.1f);
			//ImGui::DragFloat3("model scale", glm::value_ptr(floor_scale), 0.1f);
			if (ImGui::DragFloat3("model pos", glm::value_ptr(floor_meshbox.transform.pos), 0.1f)) {
				floor_meshbox.update_body_shape(physics_system);
			}

			if (ImGui::DragFloat3("model scale", glm::value_ptr(floor_meshbox.transform.scale), 0.05f, 0.1001f, 100.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
				floor_meshbox.update_body_shape(physics_system);

			}

			if (ImGui::DragFloat3("model rotation", glm::value_ptr(floor_meshbox.transform.eulerAngles), 0.25f, -FLT_MAX, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
				floor_meshbox.transform.update_rotation();
				floor_meshbox.update_body_shape(physics_system);
			}
		}

		if (ImGui::CollapsingHeader("model material", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float;
			ImGui::ColorEdit3("model color", reinterpret_cast<float*>(&model_color), flags);
			ImGui::DragFloat3("material ambient", glm::value_ptr(model_material_ambient), 0.1f);
			ImGui::DragFloat3("material diffuse", glm::value_ptr(model_material_diffuse), 0.1f);
			ImGui::DragFloat3("material specular", glm::value_ptr(model_material_specular), 0.1f);
			ImGui::DragFloat("material shininess", &model_material_shininess, 0.1f);
		}
		ImGui::End();
		// this could be: Mesh.render_gui()



		// this could be: Light.render_gui()
		ImGui::Begin("light", nullptr, global_flags);
		if (ImGui::CollapsingHeader("light transform", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat3("light pos", glm::value_ptr(light_pos), 0.1f);
			ImGui::DragFloat3("light scale", glm::value_ptr(light_scale), 0.1f);
			// TODO: Add rotation as well...
		}

		if (ImGui::CollapsingHeader("light model matrix", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::InputFloat4("", glm::value_ptr(light_model[0]));
			ImGui::InputFloat4("", glm::value_ptr(light_model[1]));
			ImGui::InputFloat4("", glm::value_ptr(light_model[2]));
			ImGui::InputFloat4("", glm::value_ptr(light_model[3]));
		}

		if (ImGui::CollapsingHeader("light material", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float;
			ImGui::ColorEdit3("light ambient", reinterpret_cast<float*>(&light_ambient), flags);
			ImGui::ColorEdit3("light diffuse", reinterpret_cast<float*>(&light_diffuse), flags);
			ImGui::ColorEdit3("light specular", reinterpret_cast<float*>(&light_specular), flags);
		}
		ImGui::End();
		// this could be: Light.render_gui()



		// this could be: Camera.render_gui()
		ImGui::Begin("Camera", nullptr, global_flags);

		ImGui::Checkbox("Wireframe", &wireframe_mode);

		ImGui::DragFloat3("rifle pos", glm::value_ptr(rifle_pos), 0.1f);
		ImGui::DragFloat3("rifle rot", glm::value_ptr(rifle_rot), 0.1f);
		ImGui::DragFloat("rifle scale", &rifle_scale, 0.1f);

		ImGui::DragFloat3("manny pos", glm::value_ptr(manny_pos), 0.1f);
		ImGui::DragFloat3("manny rot", glm::value_ptr(manny_rot), 0.1f);
		ImGui::DragFloat("manny scale", &manny_scale, 0.1f);
		if (wireframe_mode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}



		ImGui::Checkbox("FPS Camera", &fps_mode);

		//game_inst.on_gui_render(&game_inst);
		if (fps_mode) {
			fps_camera.render_gui(ImGui::GetCurrentContext());
		}
		else {
			free_camera.render_gui(ImGui::GetCurrentContext());
		}

		ImGui::SliderFloat("movement speed", &curr_camera.movement_speed, 0.1f, 100.0f);


		if (ImGui::CollapsingHeader("proj matrix", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::InputFloat4("", glm::value_ptr(projection[0]));
			ImGui::InputFloat4("", glm::value_ptr(projection[1]));
			ImGui::InputFloat4("", glm::value_ptr(projection[2]));
			ImGui::InputFloat4("", glm::value_ptr(projection[3]));
		}

		if (ImGui::CollapsingHeader("cam view matrix", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::InputFloat4("", glm::value_ptr(view[0]));
			ImGui::InputFloat4("", glm::value_ptr(view[1]));
			ImGui::InputFloat4("", glm::value_ptr(view[2]));
			ImGui::InputFloat4("", glm::value_ptr(view[3]));
		}

		ImGui::End();
		// this could be: Camera.render_gui()


		ImGui::Begin("perf", nullptr, global_flags);
		float converted_delta_time = deltaTime * 1000.0f;
		float fps = 1000.0 / converted_delta_time;
		ImGui::InputFloat("delta time (in sec)", &converted_delta_time);
		ImGui::InputFloat("FPS", &fps);
		ImGui::End();


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#pragma endregion IMGUI_RENDERING
#pragma endregion render

		glfwSwapBuffers(window);
}


	// Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
	//physics_system.get_body_interface().RemoveBody(my_sphere);
	// Destroy the sphere. After this the sphere ID is no longer valid.
	//physics_system.get_body_interface().DestroyBody(my_sphere);

	// Remove and destroy the floor
	//physics_system.get_body_interface().RemoveBody(my_floor);
	//physics_system.get_body_interface().DestroyBody(my_floor);

	// Unregisters all types with the factory and cleans up the default material
	JPH::UnregisterTypes();

	// Destroy the factory
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}

// weird behaviour with chars int uints int8 etc
unsigned int bitflag_base = 0x00034000;
unsigned int bitflag = 0x03;

void cast_ray() {
	if (!r_pressed_in_last_frame) {
		RayCast ray{ .ro = free_camera.position, .rd = free_camera.position + 100.0f * free_camera.forward };
		ray_cast_list.push_back(ray);
	}
}

void processInput(GLFWwindow* window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !three_pressed_last_frame) {
		display_bone_index++;
		display_bone_index = display_bone_index % boneCount;
		glUseProgram(skel_id);
		glUniform1i(glGetUniformLocation(skel_id, "gDisplayBoneIndex"), display_bone_index);
		INFO("display bone index: %d", display_bone_index);

	}


	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !r_pressed_in_last_frame) {
		MeshBox projectile = MeshBox(Transform3D(curr_camera.position));

		// Get the context from the user pointer
		Context* context = static_cast<Context*>(glfwGetWindowUserPointer(window));

		JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(projectile.transform.scale.x / 2.0, projectile.transform.scale.y / 2.0, projectile.transform.scale.z / 2.0));
		floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
		JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		JPH::Ref<JPH::Shape> shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
		projectile.set_shape(shape);

		projectiles.push_back(projectile);
		INFO("projectiles: %d", projectiles.size());

		//physics_system.get_body_interface().GetShape(my_sphere);
		//physics_system.get_body_interface().SetLinearVelocity(my_sphere, JPH::Vec3(0.0f, -2.0f, 0.0f));
		//physics_system.get_body_interface().SetRestitution(my_sphere, 0.5f);
		MeshBox& stored_projectile = projectiles.back();

		stored_projectile.body.physics_body_id = context->physics_system->create_body(&stored_projectile.transform, stored_projectile.body.shape, false);
		JPH::Vec3 direction = JPH::Vec3(curr_camera.forward.x, curr_camera.forward.y, curr_camera.forward.z) * 70.0f;
		context->physics_system->get_body_interface().SetLinearVelocity(stored_projectile.body.physics_body_id, direction);
	}


	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {

		bitflag = bitflag ^ 07;
		uint32_t value = bitflag_base | bitflag;
		glfwSetInputMode(window, GLFW_CURSOR, value);
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		gui_mode = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
	}

	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
		gui_mode = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !r_pressed_in_last_frame) {

		JPH::RVec3Arg from = JPH::RVec3Arg(12.0f, 0.0f, 1.0f);
		JPH::RVec3Arg to = JPH::RVec3Arg(50.0f, 0.0f, 0.0f);
		JPH::ColorArg color = JPH::ColorArg(0, 0, 255, 255);
		DEBUG("jaa %f %f %f", from.GetX(), from.GetY(), from.GetZ());

		//debugRenderer.DrawLine(from, to, color);
		cast_ray();
	}

	r_pressed_in_last_frame = glfwGetKey(window, GLFW_KEY_1) == (GLFW_PRESS || GLFW_REPEAT);
	three_pressed_last_frame = glfwGetKey(window, GLFW_KEY_3) == (GLFW_PRESS || GLFW_REPEAT);


	/*
	* TODO:
	* When I press G it will switch between GUI mode and game mode.
	* - In GUI mode the camera will NOT move and the cursor will be captured
	* - In game mode the camera will move and the cursor will NOT be captured
	Basically switch between DISABLED and CAPTURED

	03 o 04 xoreados con 07 y listo

	int bitflag_base = 0x00034000

	current_bitflag = current_bitflag ^ 07

	// maybe the shifting is not needed if i use a in32. Should try...
	// uint32_t or int32_t?

	current_bitflag =  (bitflag_base << 4)  | current_bitflag

	*/
	//#define GLFW_CURSOR_DISABLED        0x00034003
	//#define GLFW_CURSOR_CAPTURED        0x00034004


	/*
		Camera free_camera(FPS_CAMERA);
		Camera fps_camera(FREE_CAMERA);

		free_camera.process_keyboard();


		camera_process_keyboard(&camera);

	*/
	if (!gui_mode) {
		if (fps_mode) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				fps_camera.process_keyboard(FORWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				fps_camera.process_keyboard(BACKWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				fps_camera.process_keyboard(LEFT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				fps_camera.process_keyboard(RIGHT, deltaTime);
		}
		else {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				free_camera.process_keyboard(FORWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				free_camera.process_keyboard(BACKWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				free_camera.process_keyboard(LEFT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				free_camera.process_keyboard(RIGHT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				free_camera.process_keyboard(UP, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				free_camera.process_keyboard(DOWN, deltaTime);
		}
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	DEBUG("Window resized!");
	// Note: width and height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);


	// im not updating the perspective camera correctly using the new properties of the window. It only works because the aspect ratio remains the same in my
	// normal usecase
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;


	if (!gui_mode) {
		if (!fps_mode) {
			free_camera.process_mouse_movement(xoffset, yoffset);
		}
		else {

			fps_camera.process_mouse_movement(xoffset, yoffset);
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!fps_mode) {
		free_camera.process_mouse_scroll(static_cast<float>(yoffset));
	}
	else {
		fps_camera.process_mouse_scroll(static_cast<float>(yoffset));

	}
}


unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	std::string textureFullPath = std::string(AIM_ENGINE_ASSETS_PATH) + "textures/" + path;
	path = textureFullPath.c_str();
	DEBUG("path: %s", path);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		ERROR("Texture failed to load at path: %s", path);
		stbi_image_free(data);
	}

	return textureID;
}


unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource) {
	// vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		FATAL("SHADER::FRAGMENT::COMPILATION_FAILED: ");
		FATAL("%s", infoLog);
		abort();
	}
	// link shaders
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		abort();
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return shaderProgram;
}
