#pragma once
// esto es importante de que venga de un include directory externo asi lo puedo leer con <> sino se rompe todo
// IMPORTANTE: Si no se mete glad arriba del todo se ROMPE es una pelotudez por dios
#include <glad/glad.h>
#include <iostream>
#include "game_types.h"
#include "application.h"
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


/*
TODO:
	- Ver por que no puedo tomar los header files del proyecto e incluirlos con <>
	- Ver si se puede meter el `glad.c` de alguna forma automatica. Ya que va a ir en todos los ejecutables.

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
	- Renderizar un triangulo al menos
	CMake:
		- Consumir glad desde el engine y desde sandbox.
			- Hacerlo para Debug y Release.
*/



#include "better_camera.h"
#include "learnopengl/shader_m.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

static bool gui_mode = false;
static bool wireframe_mode = false;
static bool fps_mode = false;
static float gravity = 2.2;

struct Transform3D {
	glm::vec3 pos;
	glm::vec3 scale;
};

struct Light {
	Transform3D light_transform;
	glm::vec3 color;
};

struct Model {
	Transform3D model_transform;
	glm::vec3 unlit_color;
};


// settings
//const unsigned int SCR_WIDTH = 1700;
//const unsigned int SCR_HEIGHT = 900;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 640;

// camera
Camera free_camera(FREE_CAMERA, glm::vec3(5.0f, 4.0f, 8.0f));
Camera fps_camera(FPS_CAMERA, glm::vec3(0.0f, 8.0f, 3.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
glm::vec3 light_scale(0.2f);

glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse(0.5f, 0.5f, 0.5f); // darken diffuse light a bit
glm::vec3 light_specular(1.0f, 1.0f, 1.0f);



// model
glm::vec3 model_pos(0.0f, 0.0f, 0.0f);
glm::vec3 model_scale(10.0f, 1.0f, -10.0f);
glm::vec3 model_bounding_box(model_scale * 1.0f); // este esta "mal" aca la camara tiene cierta altura pero no es la colision posta con el piso.
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


int main() {
	game game_inst;
	create_game(&game_inst);

	game_inst.init(&game_inst);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(game_inst.app_config.width, game_inst.app_config.height, game_inst.app_config.name, NULL, NULL);
	//GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", glfwGetPrimaryMonitor(), NULL);



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



#pragma region renderer
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	 // ------------------------------------
	Shader lightingShader("1.colors.vs", "1.colors.fs");
	Shader lightingShaderGouraud("gouraud.vs", "gouraud.fs");
	Shader lightCubeShader("1.light_cube.vs", "1.light_cube.fs");


	float vertices[] = {
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
	// first, configure the cube's VAO (and VBO)
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

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);


#pragma region render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
#if 1
		lightingShader.use();
		lightingShader.setVec3("objectColor", model_color.r, model_color.g, model_color.b);
		//lightingShader.setVec3("material.ambient", model_material_ambient);
		//lightingShader.setVec3("material.diffuse", model_material_diffuse);
		lightingShader.setInt("material.diffuse", 0);
		lightingShader.setInt("material.specular", 1);
		lightingShader.setFloat("material.shininess", model_material_shininess);
		lightingShader.setVec3("light.position", light_pos);
		lightingShader.setVec3("light.ambient", light_ambient);
		lightingShader.setVec3("light.diffuse", light_diffuse); // darken diffuse light a bit
		lightingShader.setVec3("light.specular", light_specular);

		if (!fps_mode) {
			lightingShader.setVec3("viewPos", free_camera.position);
		}
		else {

			lightingShader.setVec3("viewPos", fps_camera.position);
		}

		// view/projection transformations
		glm::mat4 projection;
		if (!fps_mode) {
			projection = glm::perspective(glm::radians(free_camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		}
		else {
			projection = glm::perspective(glm::radians(fps_camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		}
		glm::mat4  view;
		if (!fps_mode) {
			view = free_camera.GetViewMatrix();
		}
		else {
			view = fps_camera.GetViewMatrix();
		}

		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, model_pos);
		model = glm::scale(model, model_scale); // a smaller cube
		lightingShader.setMat4("model", model);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
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
		// render the cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


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
		glDrawArrays(GL_TRIANGLES, 0, 36);
#pragma endregion LAMP_OBJECT



		update_physics(deltaTime);


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

			//static ImVec4 col{};

		ImGuiWindowFlags global_flags = ImGuiWindowFlags_None;
		if (!gui_mode) {
			global_flags = ImGuiWindowFlags_NoInputs;
		}


		ImGui::Begin("Model", nullptr, global_flags);
		if (ImGui::CollapsingHeader("model transform", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat3("model pos", glm::value_ptr(model_pos), 0.1f);
			ImGui::DragFloat3("model scale", glm::value_ptr(model_scale), 0.1f);
			// TODO: Add rotation as well...
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



		ImGui::Begin("camera", nullptr, global_flags);

		ImGui::Checkbox("Wireframe", &wireframe_mode);
		if (wireframe_mode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		ImGui::Checkbox("FPS Camera", &fps_mode);

		game_inst.on_gui_render(&game_inst);
		if (fps_mode) {
		//	fps_camera.render_gui();
		}
		else {

		//	free_camera.render_gui();
		}


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


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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
