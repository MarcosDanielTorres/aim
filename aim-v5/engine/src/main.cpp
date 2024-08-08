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
// este NO anda
//#include <core/logger/logger.h>
// este SI anda
#include "core/logger/logger.h"

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

void print_matrix(const glm::mat4& mat);
JPH_SUPPRESS_WARNINGS
#include "PhysicsSystem.h"
//#include "jolt_debug_renderer.h"


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
	std::vector<GLTFMesh> primitives;
};

struct GLTFNode {
	GLTFNode* parent;
	std::string name;
	std::vector<GLTFNode*> children;
	uint32_t index;
	int skin;
	Mesh mesh;
	glm::mat4 local_matrix;
	glm::vec3           translation{};
	glm::vec3           scale{ 1.0f };
	glm::quat           rotation{};

	void update() {

	}

	glm::mat4 getLocalMatrix()
	{
		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * local_matrix;
	}
};

struct Primitive {

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
	std::string            interpolation;
	std::vector<float>     inputs;
	std::vector<glm::vec4> outputsVec4;
};

struct AnimationChannel {
	std::string path;
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

uint32_t activeAnimation = 0;

// Animation related
std::vector<GLTFNode*> nodes;
std::vector<GLTFSkin> skins;
std::vector<GLTFAnimation> animations;


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 aTexCoords;
	glm::vec3 color;
	glm::vec4 joints;
	glm::vec4 weights;
};


void load_node(const tinygltf::Node& curr_node, GLTFNode* parent, uint32_t node_index, const tinygltf::Model& model,
	std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer)
{
	GLTFNode* new_node = new GLTFNode{};
	new_node->parent = parent;
	new_node->skin = curr_node.skin;
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
			load_node(model.nodes[curr_node.children[i]], new_node, curr_node.children[i], model, indexBuffer, vertexBuffer);
		}
	}

	if (curr_node.mesh > -1) {
		const tinygltf::Mesh mesh = model.meshes[curr_node.mesh];
		for (size_t i = 0; i < mesh.primitives.size(); i++) {
			const tinygltf::Primitive& primitive = mesh.primitives[i];
			uint32_t first_index = static_cast<uint32_t>(indexBuffer.size());
			uint32_t vertex_start = static_cast<uint32_t>(vertexBuffer.size());
			uint32_t index_count = 0;
			bool has_skin = false;
			bool has_indices = primitive.indices > -1;
			GLuint materialId = 0;
			size_t vertex_count = 0;


			// vertices

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
				vertex_count = posAccessor.count;
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

				if (jointComponentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
					abort();
					FATAL("ja");
				}

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
				Vertex vert{};
				vert.position = glm::vec4(glm::make_vec3(&posData[v * posByteStride]), 1.0f);
				vert.normal = glm::normalize(glm::vec3(normalData ? glm::make_vec3(&normalData[v * normByteStride]) : glm::vec3(0.0f)));
				vert.aTexCoords = textureData ? glm::make_vec2(&textureData[v * uv0ByteStride]) : glm::vec3(0.0f);
				vert.color = glm::vec3(1.0f);
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
				vertexBuffer.push_back(vert);
			}
			// vertices

		//for (size_t i = 0; i < vertex_count; ++i) {
		//	Vertex vertex{};
		//	vertex.position = glm::vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
		//	if (normalData) {
		//		vertex.normal = glm::vec3(normalData[i * 3], normalData[i * 3 + 1], normalData[i * 3 + 2]);
		//	}
		//	else {
		//		vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
		//	}

		//	if (textureData) {
		//		vertex.aTexCoords = glm::vec2(textureData[i * 2], textureData[i * 2 + 1]);
		//	}
		//	else {
		//		vertex.aTexCoords = glm::vec2(0.0f, 0.0f);
		//	}

		//	if (has_skin) {
		//		for (int j = 0; j < 4; ++j) {
		//			size_t offset = i * jointStride + j * tinygltf::GetComponentSizeInBytes(jointComponentType);
		//			switch (jointComponentType) {
		//			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		//				//vertex.joints[j] = reinterpret_cast<const uint8_t*>(jointData)[i * 4 + j];
		//				vertex.joints[j] = *reinterpret_cast<const uint8_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
		//				break;
		//			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		//				vertex.joints[j] = *reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
		//				//vertex.joints[j] = reinterpret_cast<const uint16_t*>(jointData)[i * 4 + j];
		//				break;
		//			default:
		//				std::cerr << "Error: Unsupported component type in JOINTS_0 attribute." << std::endl;
		//				break;
		//			}
		//			vertex.weights[j] = weightData[i * 4 + j];
		//		}

		//		for (int j = 0; j < 4; ++j) {
		//			if (weightData[i * 4 + j] >= 0.5f) {
		//				size_t offset = i * jointStride + j * tinygltf::GetComponentSizeInBytes(jointComponentType);
		//				switch (jointComponentType) {
		//				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		//					bone_ids[i] = *reinterpret_cast<const uint8_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
		//					//bone_ids[i] = reinterpret_cast<const uint8_t*>(jointData)[i * 4 + j];
		//					break;
		//				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		//					bone_ids[i] = *reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
		//					//bone_ids[i] = reinterpret_cast<const uint16_t*>(jointData)[i * 4 + j];
		//					break;
		//				default:
		//					std::cerr << "Error: Unsupported component type in JOINTS_0 attribute." << std::endl;
		//					break;
		//				}
		//				break;
		//			}
		//		}
		//	}
		//	else {
		//		vertex.joints = glm::vec4(0.0f);
		//		vertex.weights = glm::vec4(0.0f);
		//	}

		//	// Print joints and weights for debugging
		//	std::cout << "Vertex " << i << " Joints: "
		//		<< vertex.joints[0] << ", " << vertex.joints[1] << ", " << vertex.joints[2] << ", " << vertex.joints[3] << " Weights: "
		//		<< vertex.weights[0] << ", " << vertex.weights[1] << ", " << vertex.weights[2] << ", " << vertex.weights[3] << std::endl;

		//	vertexBuffer.push_back(vertex);
		//}


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
						indexBuffer.push_back(buf[index] + vertex_start);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
					const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
					for (size_t index = 0; index < idxAccessor.count; index++) {
						indexBuffer.push_back(buf[index] + vertex_start);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
					for (size_t index = 0; index < idxAccessor.count; index++) {
						indexBuffer.push_back(buf[index] + vertex_start);
					}
					break;
				}
				default:
					std::cerr << "Index component type " << idxAccessor.componentType << " not supported!" << std::endl;
					return;
				}
			}


			materialId = primitive.material;

			GLTFMesh mesh{};

			glGenVertexArrays(1, &mesh.vao);
			glBindVertexArray(mesh.vao);

			glGenBuffers(1, &mesh.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
			glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), &vertexBuffer[vertex_start], GL_STATIC_DRAW);

			glGenBuffers(1, &mesh.ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(uint16_t), &indexBuffer[first_index], GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, aTexCoords));
			glEnableVertexAttribArray(2);

			if (has_skin) {
				glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, joints));
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
				glEnableVertexAttribArray(4);
			}

			mesh.indexCount = index_count;
			mesh.first_index = first_index;
			mesh.materialId = materialId;

			glBindVertexArray(0);
			new_node->mesh.primitives.push_back(mesh);
		}
	}

	if (parent) {
		parent->children.push_back(new_node);
	}
	else {
		// aca estan todos los nodos sin padre.
		nodes.push_back(new_node);
	}
}




GLTFMesh createMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) {
	GLint* bone_ids = NULL; // array of bone IDs
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 aTexCoords;
#ifdef PART_1
		glm::vec4 joints;
		glm::vec4 weights;
#else
		int joints[4];
		float weights[4];
#endif
	};

	struct VertexSas {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 jointIndices;
		glm::vec4 jointWeights;
	};

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	GLuint materialId = 0;
	bool hasJointsAndWeights = false;
	int vertex_count = 0;

	// por ahora solo cicla una vez porque no hay mas de una `primitive`
	for (const auto& primitive : mesh.primitives) {
		// Indices
		//{
		//	const tinygltf::Accessor &  accessor   = input.accessors[glTFPrimitive.indices];
		//	const tinygltf::BufferView &bufferView = input.bufferViews[accessor.bufferView];
		//	const tinygltf::Buffer &    buffer     = input.buffers[bufferView.buffer];

		//	indexCount += static_cast<uint32_t>(accessor.count);

		//	// glTF supports different component types of indices
		//	switch (accessor.componentType)
		//	{
		//		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
		//			const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
		//			for (size_t index = 0; index < accessor.count; index++)
		//			{
		//				indexBuffer.push_back(buf[index] + vertexStart);
		//			}
		//			break;
		//		}
		//		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
		//			const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
		//			for (size_t index = 0; index < accessor.count; index++)
		//			{
		//				indexBuffer.push_back(buf[index] + vertexStart);
		//			}
		//			break;
		//		}
		//		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
		//			const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
		//			for (size_t index = 0; index < accessor.count; index++)
		//			{
		//				indexBuffer.push_back(buf[index] + vertexStart);
		//			}
		//			break;
		//		}
		//		default:
		//			std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
		//			return;
		//	}
		//}







	// 
	// 
	/////////////////////////////////////////////////






	// Position
		const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
		const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
		const auto& posBuffer = model.buffers[posBufferView.buffer];
		vertex_count = posAccessor.count;

		bone_ids = (int*)malloc(posAccessor.count * sizeof(int));

		// Normal TEXCOORD_0
		auto normalIter = primitive.attributes.find("NORMAL");
		const float* normalData = nullptr;
		if (normalIter != primitive.attributes.end()) {
			const auto& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
			const auto& normalBufferView = model.bufferViews[normalAccessor.bufferView];
			const auto& normalBuffer = model.buffers[normalBufferView.buffer];
			normalData = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);
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
			hasJointsAndWeights = true;

			// Joint data
			const auto& jointAccessor = model.accessors[jointIter->second];
			const auto& jointBufferView = model.bufferViews[jointAccessor.bufferView];
			const auto& jointBuffer = model.buffers[jointBufferView.buffer];
			jointData = &jointBuffer.data[jointBufferView.byteOffset + jointAccessor.byteOffset];
			jointComponentType = jointAccessor.componentType;
			jointType = jointAccessor.type;
			jointStride = jointBufferView.byteStride ? jointBufferView.byteStride : (jointAccessor.type == TINYGLTF_TYPE_VEC4 ? 4 * tinygltf::GetComponentSizeInBytes(jointComponentType) : 0);
			// Weight data
			const auto& weightAccessor = model.accessors[weightIter->second];
			const auto& weightBufferView = model.bufferViews[weightAccessor.bufferView];
			const auto& weightBuffer = model.buffers[weightBufferView.buffer];
			weightData = reinterpret_cast<const float*>(&weightBuffer.data[weightBufferView.byteOffset + weightAccessor.byteOffset]);
		}

		const float* posData = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

		for (size_t i = 0; i < posAccessor.count; ++i) {
			Vertex vertex;
			vertex.position = glm::vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
			if (normalData) {
				vertex.normal = glm::vec3(normalData[i * 3], normalData[i * 3 + 1], normalData[i * 3 + 2]);
			}
			else {
				vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
			}
			vertex.aTexCoords = glm::vec2(0.0f, 0.0f); // Placeholder, should be set if texcoords are available

			if (hasJointsAndWeights) {
				for (int j = 0; j < 4; ++j) {
					size_t offset = i * jointStride + j * tinygltf::GetComponentSizeInBytes(jointComponentType);
					switch (jointComponentType) {
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
						//vertex.joints[j] = reinterpret_cast<const uint8_t*>(jointData)[i * 4 + j];
						vertex.joints[j] = *reinterpret_cast<const uint8_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
						break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						vertex.joints[j] = *reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
						//vertex.joints[j] = reinterpret_cast<const uint16_t*>(jointData)[i * 4 + j];
						break;
					default:
						std::cerr << "Error: Unsupported component type in JOINTS_0 attribute." << std::endl;
						break;
					}
					vertex.weights[j] = weightData[i * 4 + j];
				}

				for (int j = 0; j < 4; ++j) {
					if (weightData[i * 4 + j] >= 0.5f) {
						size_t offset = i * jointStride + j * tinygltf::GetComponentSizeInBytes(jointComponentType);
						switch (jointComponentType) {
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
							bone_ids[i] = *reinterpret_cast<const uint8_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
							//bone_ids[i] = reinterpret_cast<const uint8_t*>(jointData)[i * 4 + j];
							break;
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
							bone_ids[i] = *reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(jointData) + offset);
							//bone_ids[i] = reinterpret_cast<const uint16_t*>(jointData)[i * 4 + j];
							break;
						default:
							std::cerr << "Error: Unsupported component type in JOINTS_0 attribute." << std::endl;
							break;
						}
						break;
					}
				}
			}
			else {
#ifdef PART_1
				vertex.joints = glm::vec4(0.0f);
				vertex.weights = glm::vec4(0.0f);
#else
				std::fill(std::begin(vertex.joints), std::end(vertex.joints), 0);
				vertex.weights[0] = 1.0f;
				std::fill(std::begin(vertex.weights) + 1, std::end(vertex.weights), 0.0f);
#endif
			}

			// Print joints and weights for debugging
			std::cout << "Vertex " << i << " Joints: "
				<< vertex.joints[0] << ", " << vertex.joints[1] << ", " << vertex.joints[2] << ", " << vertex.joints[3] << " Weights: "
				<< vertex.weights[0] << ", " << vertex.weights[1] << ", " << vertex.weights[2] << ", " << vertex.weights[3] << std::endl;

			vertices.push_back(vertex);
		}

		std::cout << "BONES\n" << std::endl;
		for (int i = 0; i < vertex_count; i++) {
			std::cout << "Vertex" << i << ": " << bone_ids[i] << std::endl;
		}

		const auto& idxAccessor = model.accessors[primitive.indices];
		const auto& idxBufferView = model.bufferViews[idxAccessor.bufferView];
		const auto& idxBuffer = model.buffers[idxBufferView.buffer];



		TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE;
		if (idxAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			std::cerr << "Error: Unsupported component type in indices." << std::endl;
			continue;
		}

		const uint16_t* idxData = reinterpret_cast<const uint16_t*>(&idxBuffer.data[idxBufferView.byteOffset + idxAccessor.byteOffset]);
		indices.insert(indices.end(), idxData, idxData + idxAccessor.count);

		materialId = primitive.material;
	}

	GLTFMesh result;
	glGenVertexArrays(1, &result.vao);
	glBindVertexArray(result.vao);

	glGenBuffers(1, &result.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, result.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &result.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, aTexCoords));
	glEnableVertexAttribArray(2);

	if (hasJointsAndWeights) {
#ifdef PART_1
		if (bone_ids) {
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(GLint), bone_ids, GL_STATIC_DRAW);
			glVertexAttribIPointer(3, 1, GL_INT, 0, NULL);
			glEnableVertexAttribArray(3);
			free(bone_ids);
		}

#else
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, joints));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
		glEnableVertexAttribArray(4);
#endif
		}

	result.indexCount = indices.size();
	result.materialId = materialId;

	glBindVertexArray(0);

	return result;
	}



struct Material {
	glm::vec4 baseColorFactor;
	GLuint baseColorTexture;
};

std::vector<Material> loadMaterials(const tinygltf::Model& model) {
	std::vector<Material> materials;

	for (const auto& gltfMaterial : model.materials) {
		Material material;
		material.baseColorFactor = glm::vec4(1.0f); // Default color factor

		if (gltfMaterial.values.find("baseColorFactor") != gltfMaterial.values.end()) {
			material.baseColorFactor = glm::vec4(
				gltfMaterial.values.at("baseColorFactor").ColorFactor()[0],
				gltfMaterial.values.at("baseColorFactor").ColorFactor()[1],
				gltfMaterial.values.at("baseColorFactor").ColorFactor()[2],
				gltfMaterial.values.at("baseColorFactor").ColorFactor()[3]
			);
		}

		if (gltfMaterial.values.find("baseColorTexture") != gltfMaterial.values.end()) {
			const auto& textureIndex = gltfMaterial.values.at("baseColorTexture").TextureIndex();
			const auto& texture = model.textures[textureIndex];
			const auto& image = model.images[texture.source];
			GLuint textureId;

			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.image.data());
			glGenerateMipmap(GL_TEXTURE_2D);

			material.baseColorTexture = textureId;
		}
		else {
			material.baseColorTexture = 0; // No texture
		}

		materials.push_back(material);
	}

	return materials;
}

//std::vector<glm::mat4> extractJointMatrices(const tinygltf::Model& model) { std::vector<glm::mat4> jointMatrices;
//
//	for (const auto& skin : model.skins) {
//		for (size_t i = 0; i < skin.joints.size(); ++i) {
//			int jointNodeIndex = skin.joints[i];
//			const auto& jointNode = model.nodes[jointNodeIndex];
//
//			glm::mat4 jointMatrix(1.0f);
//			if (!jointNode.matrix.empty()) {
//				jointMatrix = glm::make_mat4x4(jointNode.matrix.data());
//			}
//			else {
//				if (!jointNode.translation.empty()) {
//					glm::vec3 translation(jointNode.translation[0], jointNode.translation[1], jointNode.translation[2]);
//					jointMatrix = glm::translate(jointMatrix, translation);
//				}
//				if (!jointNode.rotation.empty()) {
//					glm::quat rotation(jointNode.rotation[3], jointNode.rotation[0], jointNode.rotation[1], jointNode.rotation[2]);
//					jointMatrix *= glm::mat4_cast(rotation);
//				}
//				if (!jointNode.scale.empty()) {
//					glm::vec3 scale(jointNode.scale[0], jointNode.scale[1], jointNode.scale[2]);
//					jointMatrix = glm::scale(jointMatrix, scale);
//				}
//			}
//
//			jointMatrices.push_back(jointMatrix);
//		}
//	}
//
//	return jointMatrices;
//}

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

	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "assault-rifle-yup.glb";
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, model_path); // for binary glTF(.glb)	
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "default-cube.gltf";
	//bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path); // for binary glTF(.glb)	

	// para este caso escale el cubo de blender a (0.5, 0.5, 0.5) pero asi por si solo no tiene efecto ya que esa info viene en el gltf claramente.
	// especificamente en: nodes[0].scale, o en json  nodes:[scale:[]]
	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "default-cube-scaled-down-to-0.5.gltf";
	//bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path); 

	//std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "default-cube-colored.gltf";
	//bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path); 

	std::string model_path = std::string(AIM_ENGINE_ASSETS_PATH) + "models/" + "hello.gltf";
	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path);

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

struct Keyframe {
	float time;
	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale;
};

struct Animation {
	std::vector<Keyframe> keyframes;
};

Animation createSimpleAnimation() {
	Animation animation;

	// Define keyframes
	Keyframe keyframe1;
	keyframe1.time = 0.0f;
	keyframe1.translation = glm::vec3(0.0f, 0.0f, 0.0f);
	keyframe1.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	keyframe1.scale = glm::vec3(1.0f, 1.0f, 1.0f);

	Keyframe keyframe2;
	keyframe2.time = 1.0f;
	keyframe2.translation = glm::vec3(0.0f, 1.0f, 0.0f);
	keyframe2.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	keyframe2.scale = glm::vec3(1.0f, 1.0f, 1.0f);

	Keyframe keyframe3;
	keyframe3.time = 2.0f;
	keyframe3.translation = glm::vec3(1.0f, 1.0f, 0.0f);
	keyframe3.rotation = glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));
	keyframe3.scale = glm::vec3(2.0f, 2.0f, 2.0f);

	animation.keyframes.push_back(keyframe1);
	animation.keyframes.push_back(keyframe2);
	animation.keyframes.push_back(keyframe3);

	return animation;
}


struct Joint {
	int parentIndex;
	glm::mat4 inverseBindMatrix;
	glm::mat4 currentTransform;
};

std::vector<Joint> parseSkeleton(const tinygltf::Model& model) {
	std::vector<Joint> joints;
	if (model.skins.empty()) {
		std::cerr << "Error: No skins found in the model." << std::endl;
		return joints;
	}

	const tinygltf::Skin& skin = model.skins[0]; // Assuming the first skin is used

	// Extract inverse bind matrices
	const tinygltf::Accessor& accessor = model.accessors[skin.inverseBindMatrices];
	const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
	const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
	const float* inverseBindMatrices = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

	for (size_t i = 0; i < skin.joints.size(); ++i) {
		Joint joint;
		joint.parentIndex = -1; // Initialize to -1 (no parent)

		// Load inverse bind matrix
		glm::mat4 invBindMatrix = glm::make_mat4(&inverseBindMatrices[i * 16]);
		joint.inverseBindMatrix = invBindMatrix;

		joints.push_back(joint);
	}

	// Set parent indices based on node hierarchy
	for (size_t i = 0; i < model.nodes.size(); ++i) {
		const tinygltf::Node& node = model.nodes[i];
		if (node.skin != -1) { // This node is part of a skeleton
			for (size_t j = 0; j < node.children.size(); ++j) {
				int childIndex = node.children[j];
				for (size_t k = 0; k < skin.joints.size(); ++k) {
					if (skin.joints[k] == childIndex) {
						joints[k].parentIndex = i;
					}
				}
			}
		}
	}

	return joints;
}


glm::mat4 interpolateTransform(const Keyframe& kf1, const Keyframe& kf2, float time) {
	float alpha = (time - kf1.time) / (kf2.time - kf1.time);

	glm::vec3 translation = glm::mix(kf1.translation, kf2.translation, alpha);
	glm::quat rotation = glm::slerp(kf1.rotation, kf2.rotation, alpha);
	glm::vec3 scale = glm::mix(kf1.scale, kf2.scale, alpha);

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

	return translationMatrix * rotationMatrix * scaleMatrix;
}

void updateJointTransforms(const tinygltf::Model& model, std::vector<Joint>& joints, const Animation& animation, float time) {
	if (model.skins.empty()) return;
	const tinygltf::Skin& skin = model.skins[0];

	for (size_t i = 0; i < skin.joints.size(); ++i) {
		int jointIndex = skin.joints[i];
		if (jointIndex >= model.nodes.size()) {
			std::cerr << "Error: Joint index out of range." << std::endl;
			continue;
		}

		const tinygltf::Node& node = model.nodes[jointIndex];

		// Find the two keyframes to interpolate between
		Keyframe kf1, kf2;
		for (size_t j = 0; j < animation.keyframes.size() - 1; ++j) {
			if (time >= animation.keyframes[j].time && time <= animation.keyframes[j + 1].time) {
				kf1 = animation.keyframes[j];
				kf2 = animation.keyframes[j + 1];
				break;
			}
		}

		joints[i].currentTransform = interpolateTransform(kf1, kf2, time);
	}
}


// Update joint transforms
void updateJointTransforms2(const tinygltf::Model& model, std::vector<Joint>& joints, float time) {
	if (model.skins.empty()) return;
	const tinygltf::Skin& skin = model.skins[0];

	for (size_t i = 0; i < skin.joints.size(); ++i) {
		int jointIndex = skin.joints[i];
		if (jointIndex >= model.nodes.size()) {
			std::cerr << "Error: Joint index out of range." << std::endl;
			continue;
		}

		const tinygltf::Node& node = model.nodes[jointIndex];


		glm::vec3 translation(0.0f, 0.0f, 0.0f);
		if (!node.translation.empty()) {
			translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
		}

		glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
		if (!node.rotation.empty()) {
			rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
		}

		glm::vec3 scale(1.0f, 1.0f, 1.0f);
		if (!node.scale.empty()) {
			scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
		}

		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

		joints[i].currentTransform = translationMatrix * rotationMatrix * scaleMatrix;
	}
}

void calculateBoneTransforms(const std::vector<Joint>& joints, std::vector<glm::mat4>& boneTransforms) {
	boneTransforms.resize(joints.size());

	for (size_t i = 0; i < joints.size(); ++i) {
		glm::mat4 transform = joints[i].currentTransform;

		int parentIndex = joints[i].parentIndex;
		while (parentIndex >= 0) {
			transform = joints[parentIndex].currentTransform * transform;
			parentIndex = joints[parentIndex].parentIndex;
		}

		boneTransforms[i] = transform * joints[i].inverseBindMatrix;
	}
}

void uploadBoneTransforms(GLuint shaderProgram, const std::vector<glm::mat4>& boneTransforms) {
	for (size_t i = 0; i < boneTransforms.size(); ++i) {
		std::string uniformName = "boneTransforms[" + std::to_string(i) + "]";
		GLuint uniformLocation = glGetUniformLocation(shaderProgram, uniformName.c_str());
		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(boneTransforms[i]));
	}
}


#define MAX_NUM_BONES_PER_VERTEX 4

struct VertexBoneData
{
	unsigned int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
	float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

	VertexBoneData()
	{
	}

	void AddBoneData(unsigned int BoneID, float Weight)
	{
		for (unsigned int i = 0; i < sizeof(BoneIDs) / sizeof(BoneIDs[0]); i++) {
			if (Weights[i] == 0.0) {
				BoneIDs[i] = BoneID;
				Weights[i] = Weight;
				printf("bone %d weight %f index %i\n", BoneID, Weight, i);
				return;
			}
		}

		// should never get here - more bones than we have space for
		assert(0);
	}
};



std::vector<VertexBoneData> vertex_to_bones;
std::vector<int> mesh_base_vertex;
std::map<std::string, unsigned int> bone_name_to_index_map;


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

void load_skins(const tinygltf::Model& model) {
	skins.resize(model.skins.size());

	for (size_t i = 0; i < model.skins.size(); i++) {
		tinygltf::Skin gltfSkin = model.skins[i];
		skins[i].name = gltfSkin.name;
		skins[i].skeletonRoot = nodeFromIndex(gltfSkin.skeleton);

		for (int jointIndex : gltfSkin.joints) {
			GLTFNode* node = nodeFromIndex(jointIndex);
			if (node) {
				skins[i].joints.push_back(node);
			}
		}

		if (gltfSkin.inverseBindMatrices > -1) {
			auto accessor = model.accessors[gltfSkin.inverseBindMatrices];
			auto inverse_bind_matrix_bufferview = model.bufferViews[accessor.bufferView];
			auto inverse_bind_matrix_buffer = model.buffers[inverse_bind_matrix_bufferview.buffer];

			size_t offset = accessor.byteOffset + inverse_bind_matrix_bufferview.byteOffset;

			skins[i].inverseBindMatrices.resize(accessor.count);
			memcpy(skins[i].inverseBindMatrices.data(), &inverse_bind_matrix_buffer.data[offset], sizeof(glm::mat4) * accessor.count);
		}
	}
}

void load_animations(tinygltf::Model& input)
{
	animations.resize(input.animations.size());

	for (size_t i = 0; i < input.animations.size(); i++)
	{
		tinygltf::Animation glTFAnimation = input.animations[i];
		animations[i].name = glTFAnimation.name;

		// Samplers
		animations[i].samplers.resize(glTFAnimation.samplers.size());
		for (size_t j = 0; j < glTFAnimation.samplers.size(); j++)
		{
			tinygltf::AnimationSampler glTFSampler = glTFAnimation.samplers[j];
			AnimationSampler& dstSampler = animations[i].samplers[j];
			dstSampler.interpolation = glTFSampler.interpolation;

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
					}
					break;
				}
				case TINYGLTF_TYPE_VEC4: {
					const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						dstSampler.outputsVec4.push_back(buf[index]);
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
			dstChannel.path = glTFChannel.target_path;
			dstChannel.samplerIndex = glTFChannel.sampler;
			dstChannel.node = nodeFromIndex(glTFChannel.target_node);
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

void update_joints(GLTFNode* node)
{
	if (node->skin > -1)
	{
		// Update the joint matrices
		glm::mat4              inverseTransform = glm::inverse(getNodeMatrix(node));
		GLTFSkin               skin = skins[node->skin];
		size_t                 numJoints = (uint32_t)skin.joints.size();
		assert(numJoints > 0);
		std::vector<glm::mat4> jointMatrices(numJoints); //outside
		for (size_t i = 0; i < numJoints; i++)
		{
			jointMatrices[i] = getNodeMatrix(skin.joints[i]) * skin.inverseBindMatrices[i];
			jointMatrices[i] = inverseTransform * jointMatrices[i];
		}

		glUseProgram(skinning_shader_id);
		GLint jointMatricesLoc = glGetUniformLocation(skinning_shader_id, "jointMatrices");
		glUniformMatrix4fv(jointMatricesLoc, jointMatrices.size(), GL_FALSE, glm::value_ptr(jointMatrices[0]));
	}

	for (auto& child : node->children)
	{
		update_joints(child);
	}
}

void update_animation(float deltaTime)
{
	if (activeAnimation > static_cast<uint32_t>(animations.size()) - 1)
	{
		std::cout << "No animation with index " << activeAnimation << std::endl;
		return;
	}
	GLTFAnimation& animation = animations[activeAnimation];
	animation.currentTime += deltaTime;
	if (animation.currentTime > animation.end)
	{
		animation.currentTime -= animation.end;
	}

	for (auto& channel : animation.channels)
	{
		AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
		{
			if (sampler.interpolation != "LINEAR")
			{
				std::cout << "This sample only supports linear interpolations\n";
				continue;
			}

			// Get the input keyframe values for the current time stamp
			if ((animation.currentTime >= sampler.inputs[i]) && (animation.currentTime <= sampler.inputs[i + 1]))
			{
				float a = (animation.currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (channel.path == "translation")
				{
					channel.node->translation = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
				}
				if (channel.path == "rotation")
				{
					glm::quat q1;
					q1.x = sampler.outputsVec4[i].x;
					q1.y = sampler.outputsVec4[i].y;
					q1.z = sampler.outputsVec4[i].z;
					q1.w = sampler.outputsVec4[i].w;

					glm::quat q2;
					q2.x = sampler.outputsVec4[i + 1].x;
					q2.y = sampler.outputsVec4[i + 1].y;
					q2.z = sampler.outputsVec4[i + 1].z;
					q2.w = sampler.outputsVec4[i + 1].w;

					channel.node->rotation = glm::normalize(glm::slerp(q1, q2, a));
				}
				if (channel.path == "scale")
				{
					channel.node->scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
				}
			}
		}
	}
	for (auto& node : nodes)
	{
		update_joints(node);
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


int main() {
	tinygltf::Model model = loadGLTFModel();
	std::vector<Material> materials = loadMaterials(model);
	std::vector<Joint> joints = parseSkeleton(model);
	auto mats = get_inverse_bind_matrix(model);
	INFO("Amount of mats: %d", mats.size());
	for (int i = 0; i < mats.size(); i++) {
		print_matrix(mats[i]);
	}

	INFO("SCENES");
	for (int i = 0; i < model.scenes.size(); i++) {
		INFO("%s ", model.scenes[i].name.c_str());
	}

	INFO("NODES");
	for (int i = 0; i < model.nodes.size(); i++) {
		INFO("%s ", model.nodes[i].name.c_str());
	}
	INFO("MESHES");
	for (int i = 0; i < model.meshes.size(); i++) {
		INFO("%s ", model.meshes[i].name.c_str());

		for (int j = 0; j < model.meshes[i].primitives.size(); j++) {
			INFO("INDICES %d: ", model.accessors[model.meshes[i].primitives[j].indices]);
			INFO("-- ATTRIBUTES: ");
			for (const auto& [key, value] : model.meshes[i].primitives[j].attributes) {
				INFO("key: %s   value: %d   model.accessors[value] = %s", key.c_str(), value, model.accessors[value].name.c_str());
			}
		}
	}
	INFO("SKINS");
	for (int i = 0; i < model.skins.size(); i++) {
		INFO("%s ", model.skins[i].name.c_str());
		for (int j = 0; j < model.skins[i].joints.size(); j++) {
			INFO("%d  == meshes: %s", model.skins[i].joints[j], model.nodes[model.skins[i].joints[j]].name.c_str());
		}
	}
	INFO("ANIMATIONS");
	for (int i = 0; i < model.animations.size(); i++) {
		INFO("%s ", model.animations[i].name.c_str());
	}


	projectiles.reserve(100);
	//game game_inst;
	//create_game(&game_inst);

	//game_inst.init(&game_inst);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "JDsa", NULL, NULL);
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

	std::vector<GLTFMesh> meshes;
	std::vector<uint32_t> indexBuffer;
	std::vector<Vertex> vertexBuffer;
#ifndef PART_1

	const tinygltf::Scene& scene = model.scenes[0];
	for (size_t i = 0; i < scene.nodes.size(); i++) {
		const tinygltf::Node& node = model.nodes[scene.nodes[i]];
		load_node(node, nullptr, scene.nodes[i], model, indexBuffer, vertexBuffer);
	}
	load_skins(model);
	load_animations(model);
	for (auto node : nodes) {
		update_joints(node);
	}
#endif

#ifdef PART_1
	for (const auto& gltfMesh : model.meshes) {
		// En los ejemplos que he visto solo hay un mesh en el gltf
		meshes.push_back(createMesh(model, gltfMesh));
	}
#endif

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
	Shader lightingShader("6.multiple_lights.vs.glsl", "6.multiple_lights.fs.glsl");
	//Shader lightingShader("5.2.light_casters.vs", "5.2.light_casters.fs");
	//Shader lightingShader("5.1.light_casters.vs", "5.1.light_casters.fs");
	//Shader lightingShader("1.colors.vs", "1.colors.fs");
	Shader lightingShaderGouraud("gouraud.vs", "gouraud.fs");
	Shader lightCubeShader("1.light_cube.vs", "1.light_cube.fs");
	Shader skinning_shader("skel_shader-2.vs.glsl", "skel_shader-2.fs.glsl");
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
	Animation animation = createSimpleAnimation();
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
			projection = glm::perspective(glm::radians(free_camera.zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
		}
		else {
			projection = glm::perspective(glm::radians(fps_camera.zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);

		}
		glm::mat4  view;
		if (!fps_mode) {
			view = free_camera.GetViewMatrix();
		}
		else {
			view = fps_camera.GetViewMatrix();
		}
#if 1



		// lightingShader.set_directional_light(directional_light);

		// lightingShader.set_point_light(pointLights[0], 0);
		// lightingShader.set_point_light(pointLights[1], 1);
		// lightingShader.set_point_light(pointLights[2], 2);
		// lightingShader.set_point_light(pointLights[3], 3);
		// lightingShader.set_point_lights(pointLights);

		// lightingShader.set_spot_light(spot_light);

#pragma region LIGHT_RENDERING

		float time = fmod(glfwGetTime(), animation.keyframes.back().time); // Loop animation



		lightingShader.use();





		lightingShader.setVec3("objectColor", model_color.r, model_color.g, model_color.b);
		lightingShader.setInt("material.diffuse", 0);
		lightingShader.setInt("material.specular", 1);
		lightingShader.setFloat("material.shininess", model_material_shininess);

		if (!fps_mode) {
			lightingShader.setVec3("viewPos", free_camera.position);
			lightingShader.setVec3("spotLight.position", free_camera.position);
			lightingShader.setVec3("spotLight.direction", free_camera.forward);
		}
		else {
			lightingShader.setVec3("viewPos", fps_camera.position);
			lightingShader.setVec3("spotLight.position", fps_camera.position);
			lightingShader.setVec3("spotLight.direction", fps_camera.forward);
		}

		// directional_light
		lightingShader.setVec3("dirLight.direction", directional_light.direction);
		lightingShader.setVec3("dirLight.ambient", directional_light.ambient);
		lightingShader.setVec3("dirLight.diffuse", directional_light.diffuse);
		lightingShader.setVec3("dirLight.specular", directional_light.specular);

		// point_lights
		int n = sizeof(point_lights) / sizeof(point_lights[0]);
		for (int i = 0; i < n; i++) {
			std::string prefix = "pointLights[" + std::to_string(i) + "]";

			lightingShader.setVec3(prefix + ".position", point_lights[i].transform.pos);
			lightingShader.setVec3(prefix + ".ambient", point_lights[i].ambient);
			lightingShader.setVec3(prefix + ".diffuse", point_lights[i].diffuse);
			lightingShader.setVec3(prefix + ".specular", point_lights[i].specular);
			lightingShader.setFloat(prefix + ".constant", point_lights[i].constant);
			lightingShader.setFloat(prefix + ".linear", point_lights[i].linear);
			lightingShader.setFloat(prefix + ".quadratic", point_lights[i].quadratic);
		}

		// spot_light
		lightingShader.setVec3("spotLight.ambient", spot_light.ambient);
		lightingShader.setVec3("spotLight.diffuse", spot_light.diffuse);
		lightingShader.setVec3("spotLight.specular", spot_light.specular);
		lightingShader.setFloat("spotLight.constant", spot_light.constant);
		lightingShader.setFloat("spotLight.linear", spot_light.linear);
		lightingShader.setFloat("spotLight.quadratic", spot_light.quadratic);
		lightingShader.setFloat("spotLight.cutOff", spot_light.cutOff);
		lightingShader.setFloat("spotLight.outerCutOff", spot_light.outerCutOff);

		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);


#pragma endregion LIGHT_RENDERING




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
			lightingShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// projectiles
		for (unsigned int i = 0; i < projectiles.size(); i++)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), projectiles[i].transform.pos) *
				glm::mat4_cast(projectiles[i].transform.rot) *
				glm::scale(glm::mat4(1.0f), projectiles[i].transform.scale);
			lightingShader.setMat4("model", model);

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


		lightingShader.setMat4("model", model_mat);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// render floor, is just a plane




		/*
		// skere
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

			glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 10.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

			skel_shader.setMat4("model", model_mat);

			glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
		}
		*/

		skinning_shader.use();
		skinning_shader.setMat4("projection", projection);
		skinning_shader.setMat4("view", view);
		for (auto& node : nodes) {
			if (node->mesh.primitives.size() > 0) {
				for (const auto& mesh : node->mesh.primitives) {
					glBindVertexArray(mesh.vao);

					if (mesh.materialId != -1) {
						const Material& material = materials[mesh.materialId];
					}

					glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 10.0f)) *
						glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

					skinning_shader.setMat4("model", model_mat);

					glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, 0);
					glBindVertexArray(0);
				}

			}

			// do children
		}





#pragma endregion CUBE_OBJECT

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
