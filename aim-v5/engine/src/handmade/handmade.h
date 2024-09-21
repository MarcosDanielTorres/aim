#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "game_types.h"
#include "Player.h"
#include "AssimpLoader.h"


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




// imgui includes
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


#include "learnopengl/shader_m.h"

#include <Windows.h>

#define aim_array_count(array) (sizeof(array)/ sizeof(array[0]))

#define aim_kilobytes(value) (value * 1024LL)
#define aim_megabytes(value) (aim_kilobytes(value) * 1024LL)
#define aim_gigabytes(value) (aim_megabytes(value) * 1024LL)
#define TILE_COUNT_X 17
#define TILE_COUNT_Y 9

enum keys {
	i = 73,
	j = 74,
	k = 75,
	l = 76,
	o = 79,
	p = 80,
	q = 81,
	d = 68,
	a = 65,
	s = 83,
	w = 87,
	num_0 = 48,
	num_1 = 49,
	num_2 = 50,
	num_3 = 51,
	num_4 = 52,
	num_5 = 53,
	num_6 = 54,
	num_7 = 55,
	num_8 = 56,
	num_9 = 57,
	enter = 257,
	arrow_right = 262,
	arrow_left = 263,
	arrow_down = 264,
	arrow_up = 265,
	space = 32,
};

struct keyboard_state {
	bool keys[500];
};

struct InputState {
	keyboard_state curr_state;
	keyboard_state prev_state;

	void process_key(keys key, bool pressed) {
		if (this->curr_state.keys[key] != pressed) {
			this->curr_state.keys[key] = pressed;
		}
	}

	bool is_key_pressed(keys key) {
		return this->curr_state.keys[key] == true;
	}

	// one-shot
	bool is_key_just_pressed(keys key) {
		return this->curr_state.keys[key] && !this->prev_state.keys[key];
	}

	bool is_key_released(keys key) {
		return this->curr_state.keys[key] == false;
	}

	void update() {
		this->prev_state = this->curr_state;
	}
};


struct GameOffscreenBuffer {
	uint32_t width;
	uint32_t height;
	void* memory;
};


struct GameMemory {
	bool is_initialized;

	uint64_t permanent_storage_size;
	void* permanent_storage;

	uint64_t transient_storage_size;
	void* transient_storage;
};


struct GameState {
	float player_x;
	float player_y;
};

InputState input_state{};

namespace Handmade {
	void game_update_and_render(GameOffscreenBuffer* buffer, GameMemory* game_memory, InputState* input_state, float delta_time);
	void draw_rect(GameOffscreenBuffer* buffer, float f_x, float f_y, float f_w, float f_h, float r, float g, float b);
	void keyboard_callback(GLFWwindow* window, int key, int scan_code, int action, int mods);

	int start() {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(1280, 720, "aim engine", NULL, NULL);


		AIM_INFO("GLFW window created successfully!\n");
		if (window == NULL)
		{
			AIM_FATAL("Failed to create GLFW window\n");
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window);
		glfwSetKeyCallback(window, keyboard_callback);
		//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		//glfwSetCursorPosCallback(window, mouse_callback);
		//glfwSetScrollCallback(window, scroll_callback);

		// tell GLFW to capture our mouse
		// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			AIM_FATAL("Failed to initialize GLAD\n");
			return -1;
		}
		AIM_INFO("OpenGL initialized successfully!\n");

		float deltaTime = 0.0f;
		float lastFrame = 0.0f;

		int width = 1280;
		int height = 720;


		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



		Shader basic_shader("handmade/basic-shader.vs", "handmade/basic-shader.fs");
		basic_shader.use();
		glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
		glBindTexture(GL_TEXTURE_2D, textureID); // Bind the texture

		// Make sure the texture uniform in the shader is set
		glUniform1i(glGetUniformLocation(basic_shader.ID, "colorBuffer"), 0);

		float vertices[] = {
			// Positions         // Texture Coords
			-1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f,  1.0f, 0.0f,  1.0f, 0.0f
		};
		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		// Create and bind VAO and VBO
		GLuint VAO, VBO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

		GameMemory game_memory{};
		game_memory.is_initialized = false;
		game_memory.permanent_storage_size = aim_megabytes(64);
		game_memory.transient_storage_size = aim_gigabytes(1);
		game_memory.permanent_storage = VirtualAlloc(
			0, (size_t)game_memory.permanent_storage + game_memory.transient_storage_size,
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		game_memory.transient_storage = ((uint8_t*)game_memory.permanent_storage + game_memory.transient_storage_size);


		void* buffer_memory = VirtualAlloc(0, width * height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		GameOffscreenBuffer buffer{};
		buffer.width = width;
		buffer.height = height;
		buffer.memory = buffer_memory;

		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = static_cast<float>(glfwGetTime());
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			input_state.update();
			game_update_and_render(&buffer, &game_memory, &input_state, deltaTime);


			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.memory);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glfwPollEvents();
			glfwSwapBuffers(window);
		}
	}


	int32_t truncate_f32_to_int32(float num) {
		return int32_t(num);
	}


	int32_t round_f32_to_int32(float num) {
		return int32_t(num + 0.5);
	}


	int32_t round_f32_to_uint32(float num) {
		return uint32_t(num + 0.5);
	}


	void game_update_and_render(GameOffscreenBuffer* buffer, GameMemory* game_memory, InputState* input_state, float delta_time) {
		if (sizeof(GameState) >= (size_t)game_memory->permanent_storage_size) {
			abort();
		}

		GameState* game_state = (GameState*)game_memory->permanent_storage;
		if (!game_memory->is_initialized) {
			game_state->player_x = 600.0f;
			game_state->player_y = 300.0f;
			game_memory->is_initialized = true;
		}

		float x = -10;
		float y = -10;
		float w = 40;
		float h = 40;


		uint32_t tile_map[TILE_COUNT_Y][TILE_COUNT_X] =
		{
			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
			{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
			{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
			{0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
			{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
			{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		};


		float upper_left_x = -30;
		float upper_left_y = 0;
		float tile_width = 60;
		float tile_height = 60;
		draw_rect(buffer, 0, 0, buffer->width, buffer->height, 1.0f, 1.0f, 1.0f);
		for (int i = 0; i < TILE_COUNT_Y; i++) {
			for (int j = 0; j < TILE_COUNT_X; j++) {
				draw_rect(buffer, upper_left_x + j * tile_width, upper_left_y + i * tile_height, tile_width, tile_height, 0.5f, 0.5f, 0.5f);
				if (tile_map[i][j] == 1) {
					draw_rect(buffer, upper_left_x + j * tile_width, upper_left_y + i * tile_height, tile_width, tile_height, 1.0f, 1.0f, 1.0f);
				}
			}
		}

		// these are the total width, from the (x, y)
		float player_width = 0.75f * tile_width;
		float player_height = 0.75 * tile_height;
		float player_x = 0;
		float player_y = 0;
		if (input_state->is_key_pressed(keys::w)) {
			player_y -= 100.0f;
		}

		if (input_state->is_key_pressed(keys::s)) {

			player_y += 100.0f;
		}

		if (input_state->is_key_pressed(keys::a)) {

			player_x -= 100.0f;
		}

		if (input_state->is_key_pressed(keys::d)) {
			player_x += 100.0f;
		}

		float new_player_x = game_state->player_x + player_x * delta_time;
		float new_player_y = game_state->player_y + player_y * delta_time;

		bool is_valid_1 = false;
		bool is_valid_2 = false;
		bool is_valid_3 = false;
		bool is_valid_4 = false;

		// checks (x + w, y)
		{
			int32_t player_tile_x = truncate_f32_to_int32((new_player_x + player_width - upper_left_x) / tile_width);
			int32_t player_tile_y = truncate_f32_to_int32((new_player_y - upper_left_y) / tile_height);
			if (player_tile_x >= 0 && player_tile_x < TILE_COUNT_X && player_tile_y >= 0 && player_tile_y < TILE_COUNT_Y) {
				is_valid_1 = tile_map[player_tile_y][player_tile_x] == 0;
			}
		}

		// checks (x, y + h)
		{
			int32_t player_tile_x = truncate_f32_to_int32((new_player_x - upper_left_x) / tile_width);
			int32_t player_tile_y = truncate_f32_to_int32((new_player_y + player_height - upper_left_y) / tile_height);
			if (player_tile_x >= 0 && player_tile_x < TILE_COUNT_X && player_tile_y >= 0 && player_tile_y < TILE_COUNT_Y) {
				is_valid_2 = tile_map[player_tile_y][player_tile_x] == 0;
			}
		}

		// checks (x + w, y + h)
		{
			int32_t player_tile_x = truncate_f32_to_int32((new_player_x + player_width - upper_left_x) / tile_width);
			int32_t player_tile_y = truncate_f32_to_int32((new_player_y + player_height - upper_left_y) / tile_height);
			if (player_tile_x >= 0 && player_tile_x < TILE_COUNT_X && player_tile_y >= 0 && player_tile_y < TILE_COUNT_Y) {
				is_valid_3 = tile_map[player_tile_y][player_tile_x] == 0;
			}
		}

		// checks (x, y)
		{
			int32_t player_tile_x = truncate_f32_to_int32((new_player_x - upper_left_x) / tile_width);
			int32_t player_tile_y = truncate_f32_to_int32((new_player_y - upper_left_y) / tile_height);
			if (player_tile_x >= 0 && player_tile_x < TILE_COUNT_X && player_tile_y >= 0 && player_tile_y < TILE_COUNT_Y) {
				is_valid_4 = tile_map[player_tile_y][player_tile_x] == 0;
			}
		}

		if ((is_valid_1 && is_valid_3) && (is_valid_4 && is_valid_2)) {
			game_state->player_x = new_player_x;
			game_state->player_y = new_player_y;
		}
		else {
			if (!is_valid_2 || !is_valid_2) {
				if (player_x < 0) {

					game_state->player_y = new_player_y;
				}
				else {
					if (player_y < 0) {

						game_state->player_x = new_player_x;
					}

				}
			}
		}


		float player_r = 1.0f;
		float player_g = 1.0f;
		float player_b = 0.0f;
		float player_left = game_state->player_x;
		float player_right = game_state->player_y;
		std::cout << "Player position: (" << player_left << ", " << player_right << ")\n";
		draw_rect(buffer,
			player_left, player_right,
			player_width, player_height,
			player_r, player_g, player_b);
	}


	void draw_rect(GameOffscreenBuffer* buffer, float f_x, float f_y, float f_w, float f_h, float r, float g, float b) {
		int32_t x = round_f32_to_int32(f_x);
		int32_t y = round_f32_to_int32(f_y);
		int32_t w = round_f32_to_int32(f_w);
		int32_t h = round_f32_to_int32(f_h);

		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (w > buffer->width) w = buffer->width;
		if (h > buffer->height) h = buffer->height;


		int32_t* buffer2 = (int32_t*)buffer->memory;

		uint32_t color = round_f32_to_uint32(r * 255.0f) << 0 |
			round_f32_to_uint32(g * 255.0f) << 8 |
			round_f32_to_uint32(b * 255.0f) << 16;

		for (int pixel_y = y; pixel_y < h + y; pixel_y++) {
			for (int pixel_x = x; pixel_x < w + x; pixel_x++) {
				buffer2[buffer->width * pixel_y + pixel_x] = color;
			}
		}
	}


	void keyboard_callback(GLFWwindow* window, int key, int scan_code, int action, int mods) {
		bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
		input_state.process_key(keys(key), pressed);
	}
}

