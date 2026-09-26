#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return (GLfloat)bufferWidth; }
	GLfloat getBufferHeight() { return (GLfloat)bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);
	}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }

	~Window();
private:
	GLFWwindow* mainWindow = nullptr;
	GLint width = 800, height = 600;
	bool keys[1024] = {};
	GLint bufferWidth = 0, bufferHeight = 0;

	// Control del mouse
	GLfloat lastX = 0.0f, lastY = 0.0f;
	GLfloat xChange = 0.0f, yChange = 0.0f;
	bool mouseFirstMoved = true;

	void createCallbacks();
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};