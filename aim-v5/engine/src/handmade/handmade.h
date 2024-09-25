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
#include <math.h>

#define aim_array_count(array) (sizeof(array)/ sizeof(array[0]))

#define aim_kilobytes(value) (value * 1024LL)
#define aim_megabytes(value) (aim_kilobytes(value) * 1024LL)
#define aim_gigabytes(value) (aim_megabytes(value) * 1024LL)

#define TILE_COUNT_Y 9
#define TILE_COUNT_X 17

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

InputState input_state{};

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
	float player_tile_x{ 0 };
	float player_tile_y{ 0 };
	float player_x;
	float player_y;
};

struct TileMap {
	uint32_t* tiles;
};

struct World {
	uint32_t tile_count_x{ 17 };
	uint32_t tile_count_y{ 9 };
	uint32_t tile_map_count_x{ 2 };
	uint32_t tile_map_count_y{ 2 };
	float upper_left_x = 30;
	float upper_left_y = 0;
	float tile_width = 60;
	float tile_height = 60;

	TileMap* tile_maps;
	TileMap* curr_tile_map;
};

struct CanonicalPosition {
	// cords of the tilemap
	float tile_map_x, tile_map_y;
	// cords of tiles relative to current tilemap
	float at_tile_x_in_tilemap, at_tile_y_in_tilemap;
	// cords of pixels (x, y) relative to current tilemap
	float at_x_in_tilemap, at_y_in_tilemap;
	// cords of pixels (x, y) relative to current tile inside current tilemap
	float at_x_in_tile, at_y_in_tile;
};

struct RawPosition {
	float tile_x, tile_y;
	float x, y;
};


namespace Handmade {


	CanonicalPosition get_canonical_position(World* world, float tile_map_x, float tile_map_y, float test_x, float test_y);
	bool world_is_point_empty(World* world, float tile_map_x, float tile_map_y, float x, float y);
	TileMap* world_get_tilemap(World* world, int32_t tile_map_x, int32_t tile_map_y);
	bool tilemap_is_point_empty(World* world, TileMap* tile_map, float x, float y);
	uint32_t tilemap_get_tile_data_unchecked(World* world, TileMap* tile_map, int32_t x, int32_t y);
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

	int32_t floor_f32_to_int32(float num) {
		return int32_t(floorf(num));
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
			game_state->player_tile_x = 0.0f;
			game_state->player_tile_y = 0.0f;
			game_memory->is_initialized = true;
		}



		uint32_t tile_map00[TILE_COUNT_Y][TILE_COUNT_X] =
		{
			{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
			{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
			{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
			{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
			{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		};
		uint32_t tile_map01[TILE_COUNT_Y][TILE_COUNT_X] =
		{
			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
		};
		uint32_t tile_map10[TILE_COUNT_Y][TILE_COUNT_X] =
		{
			{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		};
		uint32_t tile_map11[TILE_COUNT_Y][TILE_COUNT_X] =
		{
			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
		};

		TileMap tile_maps[2][2];
		tile_maps[0][0].tiles = (uint32_t*)tile_map00;
		tile_maps[0][1].tiles = (uint32_t*)tile_map10;
		tile_maps[1][0].tiles = (uint32_t*)tile_map01;
		tile_maps[1][1].tiles = (uint32_t*)tile_map11;

		World world{};
		world.tile_maps = (TileMap*)tile_maps;
		world.curr_tile_map = world_get_tilemap(&world, game_state->player_tile_x, game_state->player_tile_y);
		if (!world.curr_tile_map) {
			abort();
		}

		std::cout << "printing tilemap" << std::endl;
		for (int i = 0; i < TILE_COUNT_Y; i++) {
			for (int j = 0; j < TILE_COUNT_X; j++) {
				std::cout << world.curr_tile_map->tiles[i * TILE_COUNT_X + j] << " ";
			}
			std::cout << "\n";
		}



		draw_rect(buffer, 0, 0, buffer->width, buffer->height, 1.0f, 0.0f, 0.0f);
		for (int i = 0; i < TILE_COUNT_Y; i++) {
			for (int j = 0; j < TILE_COUNT_X; j++) {
				draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 0.5f, 0.5f, 0.5f);
				if (tilemap_get_tile_data_unchecked(&world, world.curr_tile_map, j, i) == 1) {
					draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 1.0f, 1.0f, 1.0f);
				}
			}
		}

		// these are the total width, from the (x, y)
		float player_width = 0.75f * world.tile_width;
		float player_height = 0.75 * world.tile_height;
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

		if (
			// checks (x + w, y)
			world_is_point_empty(&world, game_state->player_tile_x, game_state->player_tile_y, new_player_x + player_width, new_player_y) &&
			// checks (x, y + h)
			world_is_point_empty(&world, game_state->player_tile_x, game_state->player_tile_y, new_player_x, new_player_y + player_height) &&
			// checks (x + w, y + h)
			world_is_point_empty(&world, game_state->player_tile_x, game_state->player_tile_y, new_player_x + player_width, new_player_y + player_height) &&
			// checks (x, y)
			world_is_point_empty(&world, game_state->player_tile_x, game_state->player_tile_y, new_player_x, new_player_y)
			) {

			CanonicalPosition can_pos = get_canonical_position(
				&world,
				game_state->player_tile_x, game_state->player_tile_y,
				new_player_x, new_player_y
			);

			game_state->player_tile_x = can_pos.tile_map_x;
			game_state->player_tile_y = can_pos.tile_map_y;
			// These down below are the same thing, different coordinate systems that accomplish the same.
#if 0
			game_state->player_x = can_pos.at_x_in_tilemap;
			game_state->player_y = can_pos.at_y_in_tilemap;
#else
			game_state->player_x = world.upper_left_x + can_pos.at_x_in_tile + world.tile_width * can_pos.at_tile_x_in_tilemap;
			game_state->player_y = world.upper_left_y + can_pos.at_y_in_tile + world.tile_height * can_pos.at_tile_y_in_tilemap;
#endif
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

	bool tilemap_is_point_empty(World* world, TileMap* tile_map, float x, float y) {
		bool is_valid = false;
		if (tile_map) {
			if (x >= 0 && x < TILE_COUNT_X && y >= 0 && y < TILE_COUNT_Y) {
				is_valid = tilemap_get_tile_data_unchecked(world, tile_map, x, y) == 0;
			}
		}
		return is_valid;
	}

	uint32_t tilemap_get_tile_data_unchecked(World* world, TileMap* tile_map, int32_t x, int32_t y) {
		return tile_map->tiles[y * world->tile_count_x + x];
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

	TileMap* world_get_tilemap(World* world, int32_t tile_map_x, int32_t tile_map_y) {
		TileMap* tile_map = nullptr;
		if (tile_map_x >= 0 && tile_map_x < world->tile_map_count_x && tile_map_y >= 0 && tile_map_y < world->tile_map_count_y) {
			tile_map = &world->tile_maps[tile_map_y * world->tile_map_count_x + tile_map_x];
		}
		return tile_map;
	}

	CanonicalPosition get_canonical_position(World* world, float tile_map_x, float tile_map_y, float test_x, float test_y) {
		float aux_x = test_x - world->upper_left_x;
		float aux_y = test_y - world->upper_left_y;
		float at_tile_map_x = aux_x;
		float at_tile_map_y = aux_y;

		float tile_test_x = floor_f32_to_int32(aux_x / world->tile_width);
		float tile_test_y = floor_f32_to_int32(aux_y / world->tile_height);

		float at_x_in_tile = aux_x - tile_test_x * world->tile_width;
		float at_y_in_tile = aux_y - tile_test_y * world->tile_height;

		// Note: en vez de hacer `at_tile_map_` en todos los ifs. Como se que `tile_test_` siempre es entre 0 y tile_width
		// puedo hacer: `at_tile_map_ - world->tile_width * tile_test_` y es lo mismo
		if (tile_test_x < 0) {
			at_tile_map_x = world->tile_count_x * world->tile_width + test_x;
			tile_test_x = world->tile_count_x + tile_test_x;
			tile_map_x = tile_map_x - 1;
		}

		if (tile_test_x >= world->tile_count_x) {
			at_tile_map_x = test_x - world->tile_count_x * world->tile_width;
			tile_test_x = tile_test_x - world->tile_count_x;
			tile_map_x = tile_map_x + 1;
		}


		if (tile_test_y < 0) {
			at_tile_map_y = world->tile_count_y * world->tile_height + test_y;
			tile_test_y = world->tile_count_y + tile_test_y;
			tile_map_y = tile_map_y - 1;
		}

		if (tile_test_y >= world->tile_count_y) {
			at_tile_map_y = test_y - world->tile_count_y * world->tile_height;
			tile_test_y = tile_test_y - world->tile_count_y;
			tile_map_y = tile_map_y + 1;
		}

		return CanonicalPosition{
			// cords of the tilemap
			.tile_map_x = tile_map_x, .tile_map_y = tile_map_y,
			// cords of tiles relative to current tilemap
			.at_tile_x_in_tilemap = tile_test_x, .at_tile_y_in_tilemap = tile_test_y,
			// cords of pixels (x, y) relative to current tilemap
			.at_x_in_tilemap = at_tile_map_x, .at_y_in_tilemap = at_tile_map_y,
			// cords of pixels (x, y) relative to current tile inside current tilemap
			 .at_x_in_tile = at_x_in_tile, .at_y_in_tile = at_y_in_tile,
		};
	}

	bool world_is_point_empty(World* world, float tile_map_x, float tile_map_y, float test_x, float test_y) {
		bool is_valid = false;
		CanonicalPosition can_pos = get_canonical_position(world, tile_map_x, tile_map_y, test_x, test_y);

		TileMap* tile_map = world_get_tilemap(world, can_pos.tile_map_x, can_pos.tile_map_y);
		if (!tile_map) {
			return false;
		}

		is_valid = tilemap_is_point_empty(world, tile_map, can_pos.at_tile_x_in_tilemap, can_pos.at_tile_y_in_tilemap);
		return is_valid;
	}

	void keyboard_callback(GLFWwindow* window, int key, int scan_code, int action, int mods) {
		bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
		input_state.process_key(keys(key), pressed);
	}

}

