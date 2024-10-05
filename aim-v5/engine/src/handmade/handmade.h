#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

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

#define CHUNK_DIM 256

// TODO(Marcos): Move these to a 'screen' view
// buffer size == screen view == screen resolution
// screen resolution will define tile_count

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

struct ChunkInfo {
	uint32_t chunk_x;
	uint32_t chunk_y;

	uint32_t tile_x;
	uint32_t tile_y;
};

struct WorldPosition {
	// first 24 bits used to identify the chunk
	// the other 8 bits used to identify in which tile inside the chunk
	// so at most 256 x 256 tiles in a given chunk
	uint32_t packed_chunk_x;
	uint32_t packed_chunk_y;
	// Cords of pixels (x, y) in meters, relative to current tile inside current tilemap.
	// These are in meters but transformed to pixels when redering
	float at_x_in_tile, at_y_in_tile;
};

struct GameState {
	WorldPosition player_pos;
};

struct WorldChunk {
	uint32_t* tiles;
};

struct World {
	float tile_side_in_meters{ 1.4f };
	float tile_side_in_pixels{ 60.0f };
	float meters_to_pixels{ 60.0f / 1.4f };
	float upper_left_x = 1280 / 9;
	float upper_left_y = 720 / 9;
	float tile_width = 60;
	float tile_height = 60;

	uint32_t chunk_dim{ 256 };
	// TODO(Marcos): crear funciones porque no es intuitivo que mierda hacen 
	uint32_t chunk_shift{ 8 };
	uint32_t chunk_mask { (uint32_t(1) << chunk_shift) - uint32_t(1)};
	WorldChunk* chunks;
};



namespace Handmade {
	// TODO(Marcos): y si saco toda esta mierda de aca y la meto en la struct y listo? como member functions
	// Al menos me ahorraria todos estos fordecls que encima rompen la poronga de plugin de vim cuando hago goto

	// fordecls
	WorldPosition compute_canonical_position(World* world, WorldPosition position);
	ChunkInfo get_chunk_info(World* world, int32_t chunk_info_x, int32_t chunk_info_y);
	bool world_is_point_empty(World* world, WorldPosition position);
	WorldChunk get_chunk(World* world, uint32_t chunk_x, uint32_t chunk_y);
	uint32_t get_tile_data_unchecked(World* world, WorldChunk chunk, int32_t tile_x, int32_t tile_y);
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
				// chunk 0, tile 3
				.packed_chunk_x = 3,
				.packed_chunk_y = 3,
				// cords of pixels (x, y) relative to current tile inside current chunk
				 .at_x_in_tile = 0.0f, .at_y_in_tile = 0.0f,
			};
			game_memory->is_initialized = true;
		}


		World world{};

		uint32_t tiles[CHUNK_DIM][CHUNK_DIM] =
		{
			{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
			{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0,    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,    1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},

			{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,    1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0,    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
			{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
		};



		WorldChunk world_chunk;
		world_chunk.tiles = (uint32_t*)tiles;
		world.chunks = &world_chunk;

		// el problema de tener este ChunkInfo es que al pedo estoy comprimiendo todo en un 32bits si despues lo voy a tener aca. Eso es lo que no entiendo
		// porque para usar los valores si o si tengo que hacer el computo, y normalmente lo necesito en varios lados a la data. No lo uso en un solo lugar.
		// Supongo que el sentido de comprimir las cosas es porque si lo descomprimis no lo haces en todos lados... para eso ni lo descomprimis. No entiendo!

		// Conclusion: No tiene sentido por ahora, probablemnte mas tarde cuando se integre en la pipeline general

		ChunkInfo curr_chunk_info = get_chunk_info(&world, game_state->player_pos.packed_chunk_x, game_state->player_pos.packed_chunk_y);
		WorldChunk curr_chunk = get_chunk(&world, curr_chunk_info.chunk_x, curr_chunk_info.chunk_y);


		draw_rect(buffer, 0, 0, buffer->width, buffer->height, 0.3f, 0.0f, 0.0f);
		for (int i = 0; i < TILE_COUNT_Y + 1; i++) {
			for (int j = 0; j < TILE_COUNT_X + 3; j++) {
				draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 0.5f, 0.5f, 0.5f);
				if (get_tile_data_unchecked(&world, curr_chunk, j, i) == 1) {
					draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 1.0f, 1.0f, 1.0f);
				}
				//if (i == (game_state->player_pos.packed_chunk_y & world.chunk_mask) && j == (game_state->player_pos.packed_chunk_x & world.chunk_mask)) {
				//	draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 0.0f, 1.0f, 1.0f);
				//}
				if (i == curr_chunk_info.tile_y && j == curr_chunk_info.tile_x) {
					draw_rect(buffer, world.upper_left_x + j * world.tile_width, world.upper_left_y + i * world.tile_height, world.tile_width, world.tile_height, 0.0f, 1.0f, 1.0f);
				}
			}
		}

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
		// (0, 0) is at the bottom left corner of the tile. 
		// because the coordinate system is set to be Y+ up.
		// TODO(Marcos): recheck this `WorldPosition` variables
		WorldPosition bottom_center_pos = game_state->player_pos;
		bottom_center_pos.at_x_in_tile += player_x * delta_time;
		bottom_center_pos.at_y_in_tile += player_y * delta_time;
		bottom_center_pos = compute_canonical_position(&world, bottom_center_pos);


		WorldPosition bottom_left_pos = bottom_center_pos;
		bottom_left_pos.at_x_in_tile -= 0.5f * player_width;
		bottom_left_pos = compute_canonical_position(&world, bottom_left_pos);

		WorldPosition bottom_right_pos = bottom_center_pos;
		bottom_right_pos.at_x_in_tile += 0.5f * player_width;
		bottom_right_pos = compute_canonical_position(&world, bottom_right_pos);


		if (world_is_point_empty(&world, bottom_left_pos) &&
			world_is_point_empty(&world, bottom_center_pos) &&
			world_is_point_empty(&world, bottom_right_pos))
		{
			game_state->player_pos = bottom_center_pos;
		}


		float player_r = 1.0f;
		float player_g = 1.0f;
		float player_b = 0.0f;
		float player_left = world.upper_left_x +
			(world.meters_to_pixels * game_state->player_pos.at_x_in_tile) + (game_state->player_pos.packed_chunk_x & uint32_t(255)) * world.tile_width -
			0.5f * world.meters_to_pixels * player_width;
		float player_right = world.upper_left_y +
			(world.meters_to_pixels * game_state->player_pos.at_y_in_tile) + (game_state->player_pos.packed_chunk_y & uint32_t(255)) * world.tile_height;


		std::cout << "Player position: (" << player_left << ", " << player_right << ")\n";
		draw_rect(buffer,
			player_left, player_right,
			world.meters_to_pixels * player_width, world.meters_to_pixels * player_height,
			player_r, player_g, player_b);
	}


	WorldChunk get_chunk(World* world, uint32_t chunk_x, uint32_t chunk_y) {
		WorldChunk result = world->chunks[chunk_y * world->chunk_dim + chunk_x];
		return result;
	}

	uint32_t get_tile_data_unchecked(World* world, WorldChunk chunk, int32_t tile_x, int32_t tile_y) {
		uint32_t result = chunk.tiles[tile_y * world->chunk_dim + tile_x];
		return result;
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

	ChunkInfo get_chunk_info(World* world, int32_t chunk_info_x, int32_t chunk_info_y) {
		// TODO(Marcos): See differences between the datatypes of the shifts and masks
		//uint32_t chunk_x = chunk_info_x >> unsigned char(8);
		//uint32_t chunk_y = chunk_info_y >> unsigned char(8);

		//uint32_t tile_x = chunk_info_x & 0xFF;
		//uint32_t tile_y = chunk_info_y & 0xFF;

		uint32_t chunk_x = chunk_info_x >> world->chunk_shift;
		uint32_t chunk_y = chunk_info_y >> world->chunk_shift;

		uint32_t tile_x = chunk_info_x & world->chunk_mask;
		uint32_t tile_y = chunk_info_y & world->chunk_mask;

		// Note(Marcos): It's just WorldPosition decompressed. (seems useless!)
		// I don't understand why have chunk and tile inside 32 bits if i need to compute the chunk and tile everytime.

		return ChunkInfo{
			.chunk_x = chunk_x,
			.chunk_y = chunk_y,

			.tile_x = tile_x,
			.tile_y = tile_y,
		};
	}

	void recanonicalize_coord(World* world, uint32_t* tile, float* tile_rel) {
		int32_t tile_offset = floor_f32_to_int32(*tile_rel / world->tile_side_in_meters);
		*tile_rel -= tile_offset * world->tile_side_in_meters;
		if (*tile_rel < 0 || *tile_rel >= world->tile_side_in_meters) {
			abort();
		}
		*tile += tile_offset;

		// Note(Marcos): he said: all of these shouldn't go
		// Why? I think if matters but not now as not anytime soon i will be going out of the chunk.
		/*
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
		*/
		// Note(Marcos): this is not needed as `world->tile_side_in_meters` fits in 8 bits.
		// *tile = (temp_tilemap << unsigned char(8)) | temp_tile;
	}

	WorldPosition compute_canonical_position(World* world, WorldPosition pos) {
		WorldPosition result = pos;

		recanonicalize_coord(world, &result.packed_chunk_x, &result.at_x_in_tile);
		recanonicalize_coord(world, &result.packed_chunk_y, &result.at_y_in_tile);

		return result;
	}

	bool world_is_point_empty(World* world, WorldPosition position) {
		bool is_valid = false;

		ChunkInfo chunk_info = get_chunk_info(world, position.packed_chunk_x, position.packed_chunk_y);
		WorldChunk chunk = get_chunk(world, chunk_info.chunk_x, chunk_info.chunk_y);
		is_valid = get_tile_data_unchecked(world, chunk, chunk_info.tile_x, chunk_info.tile_y) == 0;
		return is_valid;
	}

	void keyboard_callback(GLFWwindow* window, int key, int scan_code, int action, int mods) {
		bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
		input_state.process_key(keys(key), pressed);
	}

}

