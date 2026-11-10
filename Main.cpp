// libreria de carga de OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// libreria estandar de C++
#include <iostream>
#include <vector>
#include <string>

// carga de imagenes y calculos 3D (GLM)
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// importador de modelos (Assimp)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// clase camara

#include "camara.h"

const unsigned int width = 800;
const unsigned int height = 800;

// Vertex Shader
const char* vertexShaderSource =
"#version 330 core\n"
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

// Fragment Shader 
const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D baseColor;\n"
"uniform sampler2D tipoMaterial;\n"
"uniform sampler2D tipoMaterial2;\n"
"void main()\n"
"{\n"
"   vec4 albedo = texture(baseColor, TexCoord);\n"
"   vec3 lightDir = normalize(vec3(0.5, 1.0, 1.0));\n"
"   vec3 normal = normalize(texture(tipoMaterial2, TexCoord).rgb * 2.0 - 1.0);\n"
"   float diff = max(dot(normal, lightDir), 0.3);\n"
"   vec3 ambient = vec3(0.4);\n"
"   vec3 result = (ambient + diff) * albedo.rgb;\n"
"   FragColor = vec4(result, albedo.a);\n"
"}\n\0";

// Estructura para almacenar cada sub-malla del modelo
struct Mesh {
    GLuint VAO, VBO, EBO;
    unsigned int numIndices;
};

std::vector<Mesh> meshesGLTF;

// =================================================================
// NUEVA FUNCIÓN: CALCULAR CURVA DE BÉZIER CÚBICA
// =================================================================
glm::vec3 CalcularBezierCubica(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 punto = uuu * p0;
    punto += 3.0f * uu * t * p1;
    punto += 3.0f * u * tt * p2;
    punto += ttt * p3;

    return punto;
}

// =================================================================
// FUNCIÓN DE CARGA GLTF CON ASSIMP
// =================================================================
bool CargarGLTF(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            if (mesh->mTextureCoords[0]) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            }
            else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        Mesh nuevaMesh;
        nuevaMesh.numIndices = static_cast<unsigned int>(indices.size());

        glGenVertexArrays(1, &nuevaMesh.VAO);
        glGenBuffers(1, &nuevaMesh.VBO);
        glGenBuffers(1, &nuevaMesh.EBO);

        glBindVertexArray(nuevaMesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, nuevaMesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nuevaMesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        meshesGLTF.push_back(nuevaMesh);
    }

    return true;
}

// =================================================================
// FUNCIÓN AUXILIAR DE CARGA SEGURA DE TEXTURAS
// =================================================================
GLuint CargarTexturaSegura(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, nrComponents;
    unsigned char* data = stbi_load(path, &w, &h, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        std::cout << "Exito al cargar textura: " << path << std::endl;
    }
    else {
        std::cout << "Error al cargar textura en: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// =================================================================
// FUNCIÓN PRINCIPAL MAIN
// =================================================================
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Modelo GLTF con Bezier", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, width, height);

    if (!CargarGLTF("scene.gltf"))
    {
        std::cout << "Error al cargar el archivo GLTF" << std::endl;
        glfwTerminate();
        return -1;
    }

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

    stbi_set_flip_vertically_on_load(true);

    GLuint texBaseColor = CargarTexturaSegura("baseColor.jpeg");
    GLuint texMetallicRoughness = CargarTexturaSegura("tipoMaterial.png");
    GLuint texNormal = CargarTexturaSegura("tipoMaterial2.png");

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "baseColor"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "tipoMaterial"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "tipoMaterial2"), 2);

    glEnable(GL_DEPTH_TEST);

    camara camera(width, height, glm::vec3(0.0f, 0.0f, 4.0f));

    // --- NUEVO: DEFINICIÓN DE PUNTOS DE CONTROL PARA LA CURVA DE BÉZIER ---
    glm::vec3 P0 = glm::vec3(-1.5f, -0.8f, 0.0f); // Inicio
    glm::vec3 P1 = glm::vec3(-0.8f, 1.5f, 0.5f); // Control 1 (Subida)
    glm::vec3 P2 = glm::vec3(0.8f, -1.5f, -0.5f); // Control 2 (Bajada)
    glm::vec3 P3 = glm::vec3(1.5f, 0.8f, 0.0f); // Final
    float velocidadBezier = 0.5f;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        camera.Inputs(window);
        camera.Matrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        float tiempo = (float)glfwGetTime();

        // --- NUEVO: CÁLCULO DE POSICIÓN SOBRE LA CURVA DE BÉZIER ---
        float t = (sin(tiempo * velocidadBezier) + 1.0f) / 2.0f; // Oscila continuamente entre 0 y 1
        glm::vec3 posicionObj = CalcularBezierCubica(t, P0, P1, P2, P3);

        // APLICAR TRASLACIÓN A LA POSICIÓN BÉZIER Y ROTACIÓN AL OBJETO
        glm::mat4 modelGLTF = glm::mat4(1.0f);
        modelGLTF = glm::translate(modelGLTF, posicionObj);
        modelGLTF = glm::rotate(modelGLTF, tiempo * glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelGLTF));

        // VINCULAR TEXTURAS
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texBaseColor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texMetallicRoughness);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texNormal);

        // DIBUJAR OBJETO
        for (const auto& mesh : meshesGLTF) {
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto& mesh : meshesGLTF) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
    glDeleteTextures(1, &texBaseColor);
    glDeleteTextures(1, &texMetallicRoughness);
    glDeleteTextures(1, &texNormal);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}