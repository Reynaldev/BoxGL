#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
#endif // _WIN32

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

/*
Shader only struct. Use Cube struct to add and modify a cube.
*/
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

/*
A struct to instantiate and modify a cube
*/
struct Cube
{
private:
	unsigned int id = 0;	// Position id;

public:
	glm::vec3 pos;

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);

	/*
	Add new cube, where it takes vector size of the cube and position, with default to 0.0f.
	Use App.retrieveNewCubeID() to retrieve ID.
	*/
	Cube(unsigned int id, glm::vec3 pos = glm::vec3(0.0f))
	{
		this->id = id;
		this->pos = pos;
	}

	unsigned int getPosID() { return this->id; }
};

struct
{
	enum CubeMod
	{
		INCREASE,
		DECREASE
	};

	// Window
	int width = 1280, height = 720;
	ImVec4 bgColor = ImVec4(0.2f, 0.3f, 0.4f, 1.0f);

	// Camera
	glm::mat4 cam = glm::mat4(1.0f);

	float camFoV = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	// Window toggles
	bool showBoxSettings = false;

	// Cube
	std::vector<Cube> cubes;
	int cubeAmount = 0;

	void updateCubes(Cube cube, CubeMod mod = INCREASE)
	{
		if (mod == INCREASE)
			cubes.push_back(cube);
		else
			cubes.erase(cubes.begin() + cube.getPosID());
	}

	unsigned int retrieveNewCubeID()
	{
		int id = 0;

		if (cubes.empty())
			return id;

		for (int i = 0; i < cubes.size(); i++)
		{
			if (cubes[i].getPosID() >= id)
				id += cubes[i].getPosID() + 1;
		}

		return id;
	}
} App;

template<typename T>
inline T clamp(T &v, T max, T min)
{
	T rv;

	if (v > max)
		v = max;

	if (v < min)
		v = min;

	return rv = v;
}

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

	GLFWwindow *window = glfwCreateWindow(App.width, App.height, "Box GL", NULL, NULL);
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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	//glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	//glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

	glEnable(GL_DEPTH_TEST);

	GLBuffer.init();

	Box box = Box();
	box.createShader("box.vert", "box.frag");
	box.createBuffer();
	box.createTexture("container.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

	App.cam = glm::perspective(glm::radians(App.camFoV), (float)(App.width / App.height), App.nearPlane, App.farPlane);

	unsigned int modelLoc = glGetUniformLocation(box.getProgram(), "uModel");
	unsigned int viewLoc = glGetUniformLocation(box.getProgram(), "uView");
	unsigned int projLoc = glGetUniformLocation(box.getProgram(), "uProj");

	while (!glfwWindowShouldClose(window))
	{
		double t = glfwGetTime();

		glClearColor(App.bgColor.x, App.bgColor.y, App.bgColor.z, App.bgColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Settings...", NULL, &App.showBoxSettings);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (App.showBoxSettings)
		{
			if (ImGui::Begin("Settings", &App.showBoxSettings))
			{
				static const char *label[] = {
					"Window",
					"Graphics"
				};
				static int selected = 0;

				if (ImGui::BeginChild("Left panel", ImVec2(150, 0), true))
				{
					for (int i = 0; i < (sizeof(label) / sizeof(char *)); i++)
					{
						if (ImGui::Selectable(label[i], selected == i))
							selected = i;
					}
				}
				ImGui::EndChild();

				ImGui::SameLine();

				if (ImGui::BeginChild("Right panel", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())))
				{
					switch (selected)
					{
					case 0:		// Window
						ImGui::SeparatorText("Camera");
						{
							ImGui::SliderFloat("Field of View", &App.camFoV, 0.0f, 100.0f);
							ImGui::SliderFloat("Near plane", &App.nearPlane, 0.1f, 100.0f);
							ImGui::SliderFloat("Far plane", &App.farPlane, 0.1f, 100.0f);
						}

						ImGui::SeparatorText("Window");
						{
							ImGui::ColorPicker4("Background Color", (float *)&App.bgColor, ImGuiColorEditFlags_NoAlpha);
						}
						break;
					case 1:		// Graphics
						ImGui::SeparatorText("Cubes");
						{
							if (ImGui::Button("Add"))
							{
								Cube cube(App.retrieveNewCubeID(), glm::vec3(0.0f));
								App.updateCubes(cube);
							}
							ImGui::SetItemTooltip("Add new cube");

							if (App.cubes.size() > 0)
							{
								if (ImGui::TreeNode("Cube settings"))
								{
									static int selected = 0;
									static char prevCombo[8];
									
									sprintf(prevCombo, "Cube %d", selected);

									if (ImGui::BeginCombo("Cubes", prevCombo))
									{
										for (int i = 0; i < App.cubes.size(); i++)
										{
											const bool isSelected = (selected == i);
											char label[8];

											sprintf(label, "Cube %d", i);
											
											if (ImGui::Selectable(label, isSelected))
												 selected = i;

											if (isSelected)
												ImGui::SetItemDefaultFocus();
										}
										ImGui::EndCombo();
									}

									Cube &cube = App.cubes.at(App.cubes[selected].getPosID());

									ImGui::DragFloat3("Position", glm::value_ptr(cube.pos));

									//if (ImGui::Button("Remove"))
									//{
									//	App.updateCubes();
									//}

									ImGui::TreePop();
								}
							}
						}
						break;
					default:
						break;
					}
				}
				ImGui::EndChild();
			}
			ImGui::End();
		}

		glBindVertexArray(GLBuffer.VAO);
		if (!App.cubes.empty())
		{
			for (int i = 0; i < App.cubes.size(); i++)
			{
				float angle = 20.0f * (i + 1);
				App.cubes[i].model = glm::mat4(1.0f);
				App.cubes[i].view = glm::mat4(1.0f);

				App.cubes[i].model = glm::rotate(
					App.cubes[i].model,
					glm::radians(angle * (float)(t / 10.0f)), glm::vec3(0.1f * (i + 1), 0.3f, 0.5f));

				App.cubes[i].view = glm::translate(App.cubes[i].view, App.cubes[i].pos);

				box.draw();

				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(App.cubes[i].model));
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(App.cubes[i].view));
				glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(App.cam));
			}
		}

		ImGui::Render();

		//int displayW, displayH;
		glfwGetFramebufferSize(window, &App.width, &App.height);
		glViewport(0, 0, App.width, App.height);

		App.cam = glm::perspective(glm::radians(App.camFoV), (float)(App.width / App.height), App.nearPlane, App.farPlane);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}