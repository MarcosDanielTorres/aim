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

struct WorldPosition {
	// first 24 bits used to identify the chunk
	// the other 8 bits used to identify in which tile inside the chunk
	// so at most 256 x 256 tiles in a given chunk
	uint32_t tile_x;
	uint32_t tile_y;
	// Cords of pixels (x, y) in meters, relative to current tile inside current tilemap.
	// These are in meters but transformed to pixels when redering
	float at_x_in_tile, at_y_in_tile;
};

struct GameState {
	WorldPosition player_pos;
};

struct TileMap {
	uint32_t* tiles;
};

struct World {
	float tile_side_in_meters{ 1.4f };
	float tile_side_in_pixels{ 60.0f };
	float meters_to_pixels{ 60.0f / 1.4f };
	uint32_t tile_count_x{ 17 };
	uint32_t tile_count_y{ 9 };
	uint32_t tile_map_count_x{ 2 };
	uint32_t tile_map_count_y{ 2 };
	float upper_left_x = 1280 / 9;
	float upper_left_y = 720 / 9;
	float tile_width = 60;
	float tile_height = 60;

	// TODO(Marcos): add chunk dim, and shifting and mask

	TileMap* tile_maps;
	TileMap* curr_tile_map;
};


struct RawPosition {
	float tile_x, tile_y;
	float x, y;
};


namespace Handmade {


	WorldPosition compute_canonical_position(World* world, float tile_map_x, float tile_map_y, float test_x, float test_y);
	WorldPosition compute_canonical_position_2(World* world, WorldPosition position);
	void recanonicalize_coord(World* world, int32_t tile_count, int32_t* tile, float* tile_rel);
	bool world_is_point_empty(World* world, float tile_map_x, float tile_map_y, float x, float y, WorldPosition position);
	bool world_is_point_empty_2(World* world, WorldPosition position);
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
		// TODO(marcos): a fucking mess 
		float vertices2[] = {
			// Positions         // Texture Coords
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,  1.0f, 1.0f
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

			//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
			game_state->player_pos = WorldPosition{
				.tile_x = 3,
				.tile_y = 3,
				// cords of pixels (x, y) relative to current tile inside current tilemap
				 .at_x_in_tile = 0.0f, .at_y_in_tile = 0.0f,
			};
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
		world.curr_tile_map = world_get_tilemap(&world, game_state->player_pos.tile_x, game_state->player_pos.tile_y);
		if (!world.curr_tile_map) {
			abort();
		}

		//std::cout << "printing tilemap" << std::endl;
		for (int i = 0; i < TILE_COUNT_Y; i++) {
			for (int j = 0; j < TILE_COUNT_X; j++) {
				//std::cout << world.curr_tile_map->tiles[i * TILE_COUNT_X + j] << " ";
			}
			//std::cout << "\n";
		}

		//uint32_t test_bit = uint32_t(0xABCDEF03);
		//uint32_t some_tile = test_bit & (255);
		//uint32_t some_tilemap = test_bit >> unsigned char(8);
		draw_rect(buffer, 0, 0, buffer->width, buffer->height, 0.3f, 0.0f, 0.0f);
		for (int i = 0; i < TILE_COUNT_Y; i++) {
			for (int j = 0; j < TILE_COUNT_X; j++) {
				draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 0.5f, 0.5f, 0.5f);
				if (tilemap_get_tile_data_unchecked(&world, world.curr_tile_map, j, i) == 1) {
					draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 1.0f, 1.0f, 1.0f);
				}
				if (i == (game_state->player_pos.tile_y & uint32_t(255)) && j == (game_state->player_pos.tile_x & uint32_t(255))) {
					draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 0.0f, 1.0f, 1.0f);
				}
			}
		}

		// these are the total width, from the (x, y)
		float player_height = 1.4f;
		float player_width = 0.75f * player_height;
		float player_x = 0;
		float player_y = 0;
		if (input_state->is_key_pressed(keys::w)) {
			player_y += 2.0f;
		}

		if (input_state->is_key_pressed(keys::s)) {

			player_y -= 2.0f;
		}
		if (input_state->is_key_pressed(keys::a)) {

			player_x -= 2.0f;
		}

		if (input_state->is_key_pressed(keys::d)) {
			player_x += 2.0f;
		}

		// Collision handling :
		// (0, 0) is at the top left corner of the tile. But because the drawing starts at (x, y) and extends down to (x + xoffset, y + yoffset)
		// because of the way the textures coords are set, I use the top row of the tile to check collisions and I offset it in Y when drawing.
		WorldPosition top_center_pos = game_state->player_pos;
		top_center_pos.at_x_in_tile += player_x * delta_time;
		top_center_pos.at_y_in_tile += player_y * delta_time;
		top_center_pos = compute_canonical_position_2(&world, top_center_pos);


		WorldPosition top_left_pos = top_center_pos;
		top_left_pos.at_x_in_tile -= 0.5f * player_width;
		top_left_pos = compute_canonical_position_2(&world, top_left_pos);

		WorldPosition top_right_pos = top_center_pos;
		top_right_pos.at_x_in_tile += 0.5f * player_width;
		top_right_pos = compute_canonical_position_2(&world, top_right_pos);


		if (world_is_point_empty_2(&world, top_left_pos) &&
			world_is_point_empty_2(&world, top_center_pos) &&
			world_is_point_empty_2(&world, top_right_pos))
		{
			game_state->player_pos = top_center_pos;
		}


		float player_r = 1.0f;
		float player_g = 1.0f;
		float player_b = 0.0f;
		float player_left = world.upper_left_x +
			(world.meters_to_pixels * game_state->player_pos.at_x_in_tile) + (game_state->player_pos.tile_x & uint32_t(255)) * world.tile_width -
			0.5f * world.meters_to_pixels * player_width;
		float player_right = world.upper_left_y +
			(world.meters_to_pixels * game_state->player_pos.at_y_in_tile) + (game_state->player_pos.tile_y & uint32_t(255)) * world.tile_height;


		std::cout << "Player position: (" << player_left << ", " << player_right << ")\n";
		draw_rect(buffer,
			player_left, player_right,
			world.meters_to_pixels * player_width, world.meters_to_pixels * player_height,
			player_r, player_g, player_b);
	}

	// x: relative to tilemap
	// y: relative to tilemap
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
		if (w > buffer->width)	w = buffer->width;
		if (h > buffer->height) h = buffer->height;


		int32_t* buffer2 = (int32_t*)buffer->memory;

		uint32_t color = round_f32_to_uint32(r * 255.0f) << 0 |
			round_f32_to_uint32(g * 255.0f) << 8 |
			round_f32_to_uint32(b * 255.0f) << 16;

		for (int pixel_y = 719 - y; pixel_y > 719 - y - h; pixel_y--) {
			for (int pixel_x = x; pixel_x < w + x; pixel_x++) {
				buffer2[buffer->width * pixel_y + pixel_x] = color;
			}
		}
	}

	TileMap* world_get_tilemap(World* world, int32_t tile_x, int32_t tile_y) {
		// TODO(Marcos): separate this into a different struct. Could be private but probably not
		int32_t tile_map_x = tile_x >> unsigned char(8);
		int32_t tile_map_y = tile_y >> unsigned char(8);
		TileMap* tile_map = nullptr;
		if (tile_map_x >= 0 && tile_map_x < world->tile_map_count_x && tile_map_y >= 0 && tile_map_y < world->tile_map_count_y) {
			tile_map = &world->tile_maps[tile_map_y * world->tile_map_count_x + tile_map_x];
		}
		return tile_map;
	}

	void recanonicalize_coord(World* world, uint32_t tile_count, uint32_t* tile, float* tile_rel) {
		int32_t tile_offset = floor_f32_to_int32(*tile_rel / world->tile_side_in_meters);
		int32_t temp_tile = *tile & uint32_t(255);
		int32_t temp_tilemap = *tile >> unsigned char(8);
		temp_tile += tile_offset;
		*tile_rel -= tile_offset * world->tile_side_in_meters;
		if (*tile_rel < 0 || *tile_rel >= world->tile_side_in_meters) {
			abort();
		}
		//*tilemap += floor_f32_to_int32(*tile / tile_count );
		if (temp_tile >= tile_count) {
			temp_tilemap += 1;
			temp_tile = tile_count - *tile;
		}
		else if (*tile < 0) {
			temp_tilemap -= 1;
			temp_tile = tile_count + *tile;
		}
		if (temp_tilemap < 0 || temp_tilemap >= 2) {
			abort();
		}
		*tile = (temp_tilemap << unsigned char(8)) | temp_tile;
	}

	WorldPosition compute_canonical_position_2(World* world, WorldPosition pos) {
		WorldPosition result = pos;

		recanonicalize_coord(world, world->tile_count_x, &result.tile_x, &result.at_x_in_tile);
		recanonicalize_coord(world, world->tile_count_y, &result.tile_y, &result.at_y_in_tile);

		return result;
	}

	bool world_is_point_empty_2(World* world, WorldPosition position) {
		bool is_valid = false;

		TileMap* tile_map = world_get_tilemap(world, position.tile_x, position.tile_y);
		if (!tile_map) {
			return false;
		}

		is_valid = tilemap_is_point_empty(world, tile_map, position.tile_x, position.tile_y);
		return is_valid;
	}

	void keyboard_callback(GLFWwindow* window, int key, int scan_code, int action, int mods) {
		bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
		input_state.process_key(keys(key), pressed);
	}

}

