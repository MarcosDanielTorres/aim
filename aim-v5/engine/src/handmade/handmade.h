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

struct GameOffscreenBuffer {
	uint32_t width;
	uint32_t height;
	void* memory;
};

namespace Handmade {



	void game_update_and_render(GameOffscreenBuffer* buffer, float delta_time);

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
		//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		//glfwSetKeyCallback(window, keyboard_callback);
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



		void* memory = VirtualAlloc(0, width * height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = static_cast<float>(glfwGetTime());
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);


			GameOffscreenBuffer buffer{};
			buffer.width = width;
			buffer.height = height;
			buffer.memory = memory;
			game_update_and_render(&buffer, deltaTime);


			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.memory);

			// Now, during your render loop, draw the quad:
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


			glfwPollEvents();
			glfwSwapBuffers(window);
		}
	}


	int32_t round_f32_to_int32(float num) {
		return int32_t(num + 0.5);
	}

	void draw_rect(GameOffscreenBuffer* buffer, float f_x, float f_y, float f_w, float f_h, uint32_t color) {
		int32_t x = round_f32_to_int32(f_x);
		int32_t y = round_f32_to_int32(f_y);
		int32_t w = round_f32_to_int32(f_w);
		int32_t h = round_f32_to_int32(f_h);

		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (w > buffer->width) w = buffer->width;
		if (h > buffer->height) h = buffer->height;


		int32_t* buffer2 = (int32_t*)buffer->memory;
		for (int pixel_y = y; pixel_y < h + y; pixel_y++) {
			for (int pixel_x = x; pixel_x < w + x; pixel_x++) {
				buffer2[buffer->width * pixel_y + pixel_x] = color;
			}
		}
	}

	void game_update_and_render(GameOffscreenBuffer* buffer, float delta_time) {
		float x = 0;
		float y = 0;
		float w = 40;
		float h = 400;
		draw_rect(buffer, 0, 0, buffer->width, buffer->height, 0x00FF00FF);
		draw_rect(buffer, x, y, w, h, 0x0000FFFF);
	}
}

