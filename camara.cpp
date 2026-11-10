#define GLM_ENABLE_EXPERIMENTAL
#include "camara.h" 
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

// Constructor
camara::camara(int width, int height, glm::vec3 position)
{
	camara::width = width;
	camara::height = height;
	Position = position;
}

// Configura y envía las matrices al shader
void camara::Matrix(float FOVdeg, float nearPlane, float farPlane, GLuint& shaderProgram, const char* uniform)
{
	// Inicializar matrices
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Configurar la matriz de vista (View)
	view = glm::lookAt(Position, Position + Orientation, Up);
	// Configurar la matriz de proyección en perspectiva (Projection)
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

	// Multiplicar matrices
	glm::mat4 camMatrix = projection * view;

	// Enviar la matriz combinada al uniform en el shader
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, uniform), 1, GL_FALSE, glm::value_ptr(camMatrix));
}

// Manejo de entradas de teclado y ratón
void camara::Inputs(GLFWwindow* window)
{
	// --- MOVIMIENTO CON TECLADO (WASD) ---

	// W: Avanzar
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Position += speed * Orientation;
	}
	// S: Retroceder
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Position -= speed * Orientation;
	}
	// A: Mover a la Izquierda (Restar el vector derecho)
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Position -= speed * glm::normalize(glm::cross(Orientation, Up));
	}
	// D: Mover a la Derecha (Sumar el vector derecho)
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	}

	// Movimiento vertical (Espacio = Arriba, Ctrl = Abajo)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		Position += speed * Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		Position -= speed * Up;
	}

	// Aumentar velocidad al presionar Shift
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed = 0.15f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		speed = 0.05f;
	}

	// --- MANEJO DE ORIENTACIÓN CON EL MOUSE (CLIC IZQUIERDO) ---
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		// Ocultar el cursor mientras se arrastra con el mouse
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2.0), (height / 2.0));
			firstClick = false;
		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Normalizar coordenadas del mouse
		float rotX = sensitivity * (float)(mouseY - (height / 2.0)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2.0)) / width;

		// Prevenir que la cámara gire en vertical 360 grados (Gimbal Lock)
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		// Rotación horizontal (Y)
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		// Centrar el cursor
		glfwSetCursorPos(window, (width / 2.0), (height / 2.0));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		// Mostrar el cursor al soltar la vista
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}