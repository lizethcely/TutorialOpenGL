#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Libraries/Include/stb/stb_image.h"
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"uniform mat4 uMVP;\n"
"void main()\n"
"{\n"
"   gl_Position = uMVP * vec4(aPos, 1.0);\n"
"   TexCoord = aTexCoord;\n"
"}\0";

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
	//Inicializar GLFW
	glfwInit();

	//decirle a GLFW que versión de OpenGL queremos usar	
	//Usar openGL 3.3

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//Decirle a GLFW que queremos usar el perfil core (usar funciones modernas)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Vértices del triángulo: [Posición X, Y, Z] + [Coordenadas Textura U, V]
	GLfloat vertices[] = {
		// Posiciones                                                // Coordenadas UV
		-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,                    0.0f, 0.0f, // Inferior Izquierda
		 0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,                    1.0f, 0.0f, // Inferior Derecha
		 0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f,                0.5f, 1.0f  // Superior Centro
	};

	//Crear una ventana de 800x600 y ponerle un título
	//GLfloat vertices[] = {
	//	-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,  //Esquina inferior izq
	//	 0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, //Esquina inferior der
	//	 0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f //Esquina superior
	//};


	//Crear una ventana de 800x600 pixeles
	GLFWwindow* window = glfwCreateWindow(800, 800, "Tutorial", NULL, NULL);

	//Verificar que la ventana se haya creado correctamente
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//Indicar a GLFW que la ventana que acabamos de crear es la ventana actual
	glfwMakeContextCurrent(window);

	//Inicializar GLAD para que configure OpenGL
	gladLoadGL();


	//Tamano de la ventana
	glViewport(0, 0, 800, 800);
	glEnable(GL_DEPTH_TEST);            // enable depth testing
	glDepthFunc(GL_LESS);


	//Crear vertices del triángulo
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//asignar el código fuente al vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	//compilar el vertex shader
	glCompileShader(vertexShader);

	
	//crear el fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//asignar el código fuente al fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	//compilar el fragment shader
	glCompileShader(fragmentShader);

	
	//crear el shader program y obtener su referencia
	GLuint shaderProgram = glCreateProgram();		
	//adjuntar los shaders al shader program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	//linkear el shader program
	glLinkProgram(shaderProgram);

	
	//eliminar los shaders ya que no son necesarios después de linkear el shader program
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//crear los contenedores de referencia para el objeto de matriz de vertices y el objeto de buffer de vértices
	GLuint VAO, VBO;

	//generar el VAO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	//hacer que el VAO sea el actual
	glBindVertexArray(VAO);

	//enlaza el VBO como el buffer de vertices actual
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//introduce los vertices en l VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Atributo 0: Posición (3 floats: X, Y, Z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Atributo 1: Coordenadas de textura (2 floats: U, V)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//vincular el VBO a 0 para que no se pueda modificar accidentalmente
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Color de fondo
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

	//cambiar el buffer posterior por el buffer frontal
	glfwSwapBuffers(window);


	// ==========================================
	// CARGAR Y CREAR LA TEXTURA DE OPENGL
	// ==========================================
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Configurar parámetros de repetición y filtrado
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Invertir imagen verticalmente para coincidir con el sistema de coordenadas de OpenGL
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	// Cambia "container.jpg" por el nombre de tu imagenS
	unsigned char* data = stbi_load("textura1.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//Loop hasta que el usuario cierre la ventana
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//le habla a OpenGL que use el shader program que creamos
		glUseProgram(shaderProgram);

		glBindTexture(GL_TEXTURE_2D, texture);
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		float aspect = width > 0 ? (float)width / (float)height : 4.0f / 3.0f;
		//Vincular el VAO para que OpenGL sepa que atributos de vértices usar
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);

		// view: camera at (0,0,3) looking at origin
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		// model: identity or transform your objects (rotate for demo)
		float t = (float)glfwGetTime();
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 mvp = projection * view * model;
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
		//Revisar si hubo algún evento (input, teclado, ratón, 
		glfwPollEvents();
	}

	//eliminar los objetos de OpenGL
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);		
	glDeleteProgram(shaderProgram);

	//Eliminar la ventana y terminar GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}