#pragma once
// esto es importante de que venga de un include directory externo asi lo puedo leer con <> sino se rompe todo
// IMPORTANTE: Si no se mete glad arriba del todo se ROMPE es una pelotudez por dios
#include <glad/glad.h>
#include <iostream>
#include "game_types.h"
#include "application.h"

// este NO anda
//#include <core/logger/logger.h>
// este SI anda
#include "core/logger/logger.h"


/*
TODO:
	- Renderizar un triangulo al menos
	- Arrancar por el capitulo de luces

	- Ver por que no puedo tomar los header files del proyecto e incluirlos con <>
	- Ver si se puede meter el `glad.c` de alguna forma automatica. Ya que va a ir en todos los ejecutables.

	CMake:
		- Meter glad en thirdparty
		- Consumir glad desde el engine y desde sandbox.
		- Meter la solution `imgui` y `glfw` en un folder que diga `thirdparty`


SUSPICIOUS THINGS:
	- I removed `glad.c` from the engine. It's only needed on `Sandbox`.
*/


//extern game create_game();
extern bool create_game(game* game_inst);

int main() {
	//game game_inst = create_game();
	//game_inst.update(1.0f);


	game game_inst;
	create_game(&game_inst);

	game_inst.init(&game_inst);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(game_inst.app_config.width, game_inst.app_config.height, game_inst.app_config.name, NULL, NULL);
	INFO("GLFW window created successfully!");
	if (window == NULL)
	{
		FATAL("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		FATAL("Failed to initialize GLAD");
		return -1;
	}
	INFO("OpenGL initialized successfully!");

	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//HelloTriangleApplication app;

	//app.run(&game_inst);



}
