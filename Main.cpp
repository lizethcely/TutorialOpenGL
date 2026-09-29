#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camara.h"

const unsigned int width = 800;
const unsigned int height = 800;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"uniform mat4 camMatrix;\n"
"uniform mat4 model;\n" 
"void main()\n"
"{\n"
"   gl_Position = camMatrix * model * vec4(aPos, 1.0);\n" 
"   TexCoord = aTexCoord;\n"
"}\0";

// Fragment shader con sampler2D
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D ourTexture;\n"
"void main()\n"
"{\n"
"   FragColor = texture(ourTexture, TexCoord);\n"
"}\n\0";

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "triangulo con textura y camara", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glViewport(0, 0, width, height);

	// Vértices del triángulo: Posición (X,Y,Z) + Coordenadas UV
	GLfloat vertices[] = {
		-0.5f, -0.5f * float(sqrt(3)) / 3.0f, 0.0f,    0.0f, 0.0f,
		 0.5f, -0.5f * float(sqrt(3)) / 3.0f, 0.0f,    1.0f, 0.0f,
		 0.0f,  0.5f * float(sqrt(3)) * 2 / 3.0f, 0.0f, 0.5f, 1.0f
	};

	// Compilación de shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// VAO y VBO
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Crear y Cargar Textura
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//Definen qué pasa si las coordenadas UV se salen del rango 0.0 a 1.0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //eje horizontal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //eje vertical
	//Define cómo se procesa la textura cuando el triángulo se ve lejano/pequeño.
	//usa filtrado trilineal para suavizar los píxeles y evitar parpadeos visuales.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//Define cómo se procesa cuando el triángulo está muy cerca/grande.
	//aplica una interpolación suave entre píxeles adyacentes para evitar que se vea pixelado.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Invierte la imagen verticalmente al leerla.
	stbi_set_flip_vertically_on_load(true);

	//Abre el archivo de imagen en el disco duro, lee las dimensiones
	int texWidth, texHeight, nrChannels;
	unsigned char* data = stbi_load("textura1.jpg", &texWidth, &texHeight, &nrChannels, 0);

	if (data)
	{
		//Detecta automáticamente si la imagen tiene canal de transparencia Alpha png y jpg
		GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		//Envía los píxeles desde la RAM de la computadora a la memoria dedicada VRAM de la tarjeta gráfica (GPU).
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	//Activa el Z-Buffer (Buffer de profundidad) de OpenGL.
	// Esto asegura que si hay varios objetos en la escena 3D,
	// los objetos que estén por delante tapen correctamente a los que están guardados por detrás en el espacio 
	glEnable(GL_DEPTH_TEST);

	// Crear el objeto de camara
	camara camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		// Procesa las teclas WASD y el mouse para mover la cámara 
		camera.Inputs(window);
		camera.Matrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");

		//Calcula la rotacion sobre el eje Y basandose en el tiempo transcurrido
		float t = (float)glfwGetTime();
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// dibuja el triangulo renderizado con su textura
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Limpieza de objetos OpenGL
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteTextures(1, &texture);
	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}