#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class camara
{
public:
	// Vectores de la cámara
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	// Prevenir saltos de cámara al hacer primer clic
	bool firstClick = true;

	// Ancho y alto de la ventana
	int width;
	int height;

	// Ajustes de velocidad y sensibilidad
	float speed = 0.05f;
	float sensitivity = 100.0f;

	// Constructor
	camara(int width, int height, glm::vec3 position);

	// Actualiza la matriz MVP de la cámara y la envía al shader
	void Matrix(float FOVdeg, float nearPlane, float farPlane, GLuint& shaderProgram, const char* uniform);
	// Maneja las entradas de teclado (WASD) y ratón
	void Inputs(GLFWwindow* window);
};
#endif
