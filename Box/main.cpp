#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600

typedef std::string String;

struct
{
    GLuint VAO, VBO;

    void init()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }
} GLBuffer;

struct Box
{
private:
    GLuint progID;
	float vertices[180];

	unsigned int texture = 0;
	int textureW, textureH, textureCh;

public:
	Box()
	{
        float newVerts[] = {
            // Position             // Texture coords
            -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,    0.0f, 1.0f
        };

        memcpy(vertices, newVerts, sizeof(newVerts));
	}

    void createShader(const char *vertexPath, const char *fragmentPath)
    {
		// Retrieve the vertex/fragment source code from filepath.
		String vertexCode, fragmentCode;
		std::ifstream vShaderFile, fShaderFile;

		// Ensure ifstream objects can throw exceptions.
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		// Debug
		//printf("Vertex path: %s\n", vertexPath.c_str());
		//printf("Fragment path: %s\n", fragmentPath.c_str());

		try
		{
			// Open files.
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);

			std::stringstream vShaderStream, fShaderStream;

			// Read file's contents into streams.
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			// Close file handlers.
			vShaderFile.close();
			fShaderFile.close();

			// Convert stream into string.
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (const std::ifstream::failure e)
		{
			printf("Error. Faield to read shader files.\n");
		}

		const char *vShaderCode = vertexCode.c_str();
		const char *fShaderCode = fragmentCode.c_str();

		//printf("%s\n", vShaderCode);

		// Compile shader.
		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		// Vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			printf("Error. Vertex shader compilation failed.\n%s\n", infoLog);
		}

		// Fragment shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		// Print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			printf("Error. Fragment shader compilation failed.\n%s\n", infoLog);
		}

		// Shader program
		this->progID = glCreateProgram();
		glAttachShader(progID, vertex);
		glAttachShader(progID, fragment);
		glLinkProgram(progID);

		// Print linking errors if any
		glGetProgramiv(progID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(progID, 512, NULL, infoLog);
			printf("Error. Shader linking failed.\n%s\n", infoLog);
		}

		// Delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertex);
		glDeleteShader(fragment);
    }

	void createBuffer()
	{
		glBindVertexArray(GLBuffer.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, GLBuffer.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid *)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	void createTexture(const char *file, GLuint s, GLuint t, GLuint min, GLuint mag, GLuint fmt = GL_RGB)
	{
		glGenTextures(1, &texture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);

		unsigned char *data = stbi_load(file, &textureW, &textureH, &textureCh, 0);
		if (!data)
		{
			printf("Failed to load texture\nPath: %s\n", file);
			return;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, fmt, textureW, textureH, 0, fmt, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}

	void use()
	{
		glUseProgram(this->progID);
	}

	void draw()
	{
		use();

		if (!texture)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
		}

		glBindVertexArray(GLBuffer.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	GLuint getProgram() { return this->progID; }
};

void frameBufferSizeCallback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFW_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Box GL", NULL, NULL);
	if (!window)
	{
		printf("Failed to create window\n");
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD");
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    GLBuffer.init();

    Box box = Box();
	box.createShader("box.vert", "box.frag");
	box.createBuffer();
	box.createTexture("container.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::mat4 model(1.0f);
	glm::mat4 view(1.0f);
	glm::mat4 projection(1.0f);

	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	unsigned int modelLoc = glGetUniformLocation(box.getProgram(), "uModel");
	unsigned int viewLoc = glGetUniformLocation(box.getProgram(), "uView");
	unsigned int projLoc = glGetUniformLocation(box.getProgram(), "uProj");

	while (!glfwWindowShouldClose(window))
	{
		double t = glfwGetTime();

		glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glBindVertexArray(GLBuffer.VAO);
		for (int i = 0; i < 10; i++)
		{
			float angle = 20.0f * (i + 1);
			model = glm::mat4(1.0f);

			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model, glm::radians(angle * (float)(t / 10.0f)), glm::vec3(0.1f * (i + 1), 0.3f, 0.5f));

			box.draw();

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}