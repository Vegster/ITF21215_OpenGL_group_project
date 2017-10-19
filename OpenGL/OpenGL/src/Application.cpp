#include <GL/glew.h>
#include <GLFW/glfw3.h>
// STB: Texture loader for several different types of image files
#include "stb_image.h"
// GLM: Matrix and vector math-library (temporary) 
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
// Shaders and camera class, based on Joey de Vries' camera and shader class from learnOpenGL.com.
#include "shader.h"
#include "camera.h"
// Dungeon class created by Thomas Angeland for creating random-generated dungeons in adjustable sizes.
#include "Dungeon.h"
// 3D Object classes
#include "Rectangle.h"
#include "Cube.h"
#include "Texture.h"
// General C++ (and C++11) libraries
#include <iostream>
#include <string>
#include <vector>
#include <time.h>

// Presets
#define DEBUG true
#define FULLSCREEN false
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
double getTimeSeconds(clock_t startTime, clock_t endTime);

// Window
const char window_title[] = "ITF21215 OpenGL group project";
const int openGL_min = 4, openGL_max = 4;

clock_t start_time_init, start_time_shaders, start_time_meshes, time_textures;

// Camera
Camera camera;
const glm::vec3 SPAWN_POSITION = glm::vec3(0.0f, 2.0f, 3.0f);
double lastX = WINDOW_WIDTH / 2.0f;
double lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// Global positions
glm::vec3 lightPosition = glm::vec3(0.0f, 5.0f, 0.0f);

// ===========================================================================================
// PROGRAM START
// ===========================================================================================

int main(void)
{
	if (DEBUG) {
		std::cout << "[DEBUG MODUS]" << std::endl;
		start_time_init = clock();
	}

	// Initialize GLFW
	int glfw = glfwInit();
	if (glfw == GLFW_FALSE)
	{
		std::cout << "Error: Failed to initialize GLFW" << std::endl;
		return -1;
	}
	// Set GLFW to use OpenGL version 4.5, both minor and major
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGL_max);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGL_min);
	// Get access to a smaller subset of OpenGL features (no backwards-compatibility)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window and it's OpenGL context
	GLFWwindow* window;
	if (FULLSCREEN) window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_title, glfwGetPrimaryMonitor(), NULL);
	else window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_title, NULL, NULL);
	if (!window)
	{
		std::cout << "Error: Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set actionlisteners for window resize, mouse and scrolling
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Tell GLFW to capture the players mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);

	// Initialize GLEW and check if it's OK
	GLenum glew = glewInit();
	if (GLEW_OK != glew)
	{
		std::cout << "Error: " << glewGetErrorString(glew) << std::endl;
		return -1;
	}
	if (DEBUG) {
		std::cout << "OpenGL version " << glGetString(GL_VERSION) << std::endl;
		std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
		std::cout << "Initialation time: " << getTimeSeconds(start_time_init, clock()) << " seconds" << std::endl;
	}

	// ===========================================================================================
	// CAMERA - for player movement, sets spawn position
	// ===========================================================================================
	camera = Camera(SPAWN_POSITION);

	// ===========================================================================================
	// SHADER - Build and compile the shader program (with LearnOpenGL's provided shader class)
	// ===========================================================================================
	if (DEBUG) start_time_shaders = clock();
	
	Shader shader("src/shaders/vertex_shader.vert", "src/shaders/fragment_shader.frag");
	Shader lightShader("src/shaders/light_shader.vert", "src/shaders/light_shader.frag");
	
	if (DEBUG) std::cout << "Shader create time: " << getTimeSeconds(start_time_shaders, clock()) << " seconds" << std::endl;
	// ===========================================================================================
	// 3D OBJECTS - Set up vertex data and buffers and configure vertex attributes
	// ===========================================================================================
	if (DEBUG) start_time_meshes = clock();

	Cube cube = Cube();
	cube.storeOnGPU();
	Rect plane = Rect(4.0f, 4.0f);
	plane.storeOnGPU();

	if (DEBUG) std::cout << "Mesh create/store time: " << getTimeSeconds(start_time_meshes, clock()) << " seconds" << std::endl;
	// ===========================================================================================
	// LOAD TEXTURES
	// ===========================================================================================
	if (DEBUG) time_textures = clock();

	Texture metal = Texture();
	metal.addDiffuse("src/resources/textures/1857-diffuse.jpg");
	metal.addSpecular("src/resources/textures/1857-specexponent.jpg");
	metal.addNormal("src/resources/textures/1857-normal.jpg");

	Texture tile = Texture();
	tile.addDiffuse("src/resources/textures/10744-diffuse.jpg");
	tile.addSpecular("src/resources/textures/10744-specstrength.jpg");
	tile.addNormal("src/resources/textures/10744-normal.jpg");
	tile.addAO("src/resources/textures/10744-ambientocclusion.jpg");
	
	Texture::SetShaderSampler(&shader);

	if (DEBUG) std::cout << "Texture create time: " << getTimeSeconds(time_textures, clock()) << " seconds" << std::endl;
	// ===========================================================================================
	// RENDER LOOP
	// ===========================================================================================
	while (!glfwWindowShouldClose(window)) {

		// Per-frame time logic
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Process input (if any)
		processInput(window);

		// Background color (world color)
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		// Clear the depth buffer before each render iteration
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader when setting uniforms/drawing objects
		shader.use();
		shader.setVec3("light.position", lightPosition);
		shader.setVec3("viewPos", camera.Position);

		// light properties
		shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		// material properties
		shader.setFloat("material.shininess", 64.0f);

		// Calculate View/Projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		// -------------------------------------------------------------------------------------------
		// DRAW OBJECTS (see Rectangle/Cube class for draw functions)
		// -------------------------------------------------------------------------------------------

		cube.drawObject(&shader, metal);
		cube.drawObject(&shader, glm::vec3(2.0f, 0.0f, 2.0f) , tile);

		// Activate light shader and configure it
		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);

		// Draw light object
		plane.drawObject(&lightShader, lightPosition, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f), metal);

		// GLFW: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// GLFW: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// ===========================================================================================
// PLAYER INPUTS
// ===========================================================================================
// Process all input: Query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// GLFW: whenever the window size changes, this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// GLFW: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// GLFW: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.setFOV((float)yoffset);
}

// Calculate time passed between two timestamps and return it in seconds
double getTimeSeconds(clock_t time_begin, clock_t time_end) {
	return double(time_end - time_begin) / CLOCKS_PER_SEC;
}