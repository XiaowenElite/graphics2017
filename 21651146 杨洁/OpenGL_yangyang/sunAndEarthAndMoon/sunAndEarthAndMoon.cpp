// C++标准库
#include <string>
#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GL/glfw3.h>

// 相机、着色器和模型类
#include "Shader.h"
#include "Model.h"
#include "Camera.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 图片加载库
#include <SOIL/SOIL.h>

// 相机、屏幕宽高属性以及π值
Camera camera(glm::vec3(0.0f, 0.0f, 2.0f));
GLuint screenWidth = 800, screenHeight = 800;
const float PI = 3.1415927f;

// 按键响应
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// 加载纹理
GLuint loadTexture(GLchar* path);

GLFWwindow* window;

int main() {
	// GLFW初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(screenWidth, screenHeight, "simpleSolar", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// 注册鼠标事件
	glfwSetKeyCallback(window, key_callback);

	// GLEW初始化
	glewExperimental = GL_TRUE;
	glewInit();

	// 定义视口
	glViewport(0, 0, screenWidth, screenHeight);

	

	// 加载bg
	GLuint bgTexture = loadTexture("src/bg/bg.jpg");
	GLfloat vertices[] = {
		// Positions                    // Texture Coords
		1.0f,  1.0f, 0.0f,  0.0f, 0.0f, // Top Right
		1.0f, -1.0f, 0.0f,  0.0f, 1.0f, // Bottom Right
		-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, // Bottom Left
		-1.0f,  1.0f, 0.0f, 1.0f, 0.0f  // Top Left 
	};
	GLuint indices[] = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};
	GLuint bgVAO, bgVBO, bgEBO;
	glGenVertexArrays(1, &bgVAO);
	glGenBuffers(1, &bgVBO);
	glGenBuffers(1, &bgEBO);
	glBindVertexArray(bgVAO);

	glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// 加载着色器
	Shader simpleShader("Shaders/simpleShaderVertex.shader", "Shaders/simpleShaderFragment.shader");
	Shader bgShader("Shaders/bgShaderVertex.shader", "Shaders/bgShaderFragment.shader");
	// 加载行星模型
	Model earthModel("src/planet/earth/earth.obj");
	Model sunModel("src/planet/sun/sun.obj");
	Model moonModel("src/planet/moon/moon.obj");

	// 行星自转角度
	GLfloat earthRotate = 0.0f;
	GLfloat moonRotate = 0.0f;

	// 行星公转角度
	GLfloat earthRevolute = 0.0f;
	GLfloat moonRevolute = 0.0f;

	// game loop
	while (!glfwWindowShouldClose(window))
	{
		// 事件侦听
		glfwPollEvents();

		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		bgShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bgTexture);
		glBindVertexArray(bgVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// 开启深度测试
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		simpleShader.Use();

		// 太阳
		glm::mat4 model;
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(simpleShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		sunModel.Draw(simpleShader);

		// 地球
		model = glm::mat4();		
		model = glm::rotate(model, glm::radians(earthRevolute), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(earthRotate), glm::vec3(1.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.033f, 0.033f, 0.033f));
		glUniformMatrix4fv(glGetUniformLocation(simpleShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		earthModel.Draw(simpleShader);

		// 月球
		model = glm::mat4();
		model = glm::rotate(model, glm::radians(earthRevolute), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.5f + 0.1f * cos(glm::radians(moonRevolute)), 0.0f, 0.0f + 0.1f * sin(glm::radians(moonRevolute))));
		model = glm::rotate(model, glm::radians(moonRotate), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
		glUniformMatrix4fv(glGetUniformLocation(simpleShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		moonModel.Draw(simpleShader);
		
		glDisable(GL_DEPTH_TEST);
		// 自转
		earthRotate += 1.0f;
		moonRotate += 1.5f;

		// 公转
		earthRevolute += 2.5f / 5;
		moonRevolute += 4.5f / 5;

		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

GLuint loadTexture(GLchar* path) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}