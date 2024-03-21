#if defined(_WIN32)
	#define _CRT_SECURE_NO_WARNINGS
#endif // _WIN32

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <Windows.h>
#include <ShObjIdl.h>

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

template<typename T>
constexpr T clamp(T &v, T max, T min);

template <typename T>
constexpr T arrLen(T &arr);

void frameBufferSizeCallback(GLFWwindow *window, int width, int height);

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

	GLuint texture[2];
	int textureW[2], textureH[2], textureCh[2];

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

	bool createTexture(int index, const char *file, GLuint s, GLuint t, GLuint min, GLuint mag, GLuint fmt = GL_RGB)
	{
		glGenTextures(1, &texture[index]);

		if (index > 1)
			return false;

		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, texture[index]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);

		stbi_set_flip_vertically_on_load(true);

		unsigned char *data = stbi_load(file, &textureW[index], &textureH[index], &textureCh[index], 0);
		if (!data)
		{
			printf("Failed to load texture\nPath: %s\n", file);
			return false;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, fmt, textureW[index], textureH[index], 0, fmt, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		return true;
	}

	void use()
	{
		glUseProgram(this->progID);
	}

	void draw()
	{
		use();

		if (!texture)
			for (int i = 0; i < 2; i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, texture[i]);
			}

		glBindVertexArray(GLBuffer.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	GLuint getProgram() { return this->progID; }

	GLuint getTexture(int index) { return texture[index]; }
};

/*
A struct to instantiate and modify a cube
*/
struct Cube
{
private:
	unsigned int id = 0;	// Position id;

public:
	glm::vec3 pos	= glm::vec3(0.0f); 
	glm::vec3 rot	= glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view	= glm::mat4(1.0f);

	ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	// Is rotation continuous/auto?
	bool isAutoRot = false;

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
	enum CubeModFlags
	{
		INCREASE,
		DECREASE
	};

	enum ChangeCubeTextureFlags
	{
		TEXTURE0 = 1 << 0,
		TEXTURE1 = 1 << 1
	};

	enum CameraProjection
	{
		CAM_PROJ_ORTHOGRAPHIC	= 1 << 0,
		CAM_PROJ_PERSPECTIVE	= 1 << 1
	};

	// Window
	int width = 1280, height = 720;
	ImVec4 bgColor = ImVec4(0.2f, 0.3f, 0.4f, 1.0f);

	// Camera
	glm::mat4 cam = glm::mat4(1.0f);

	String modalName;
	String filePath;

	float camFoV = 75.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	// For orthogrphic projection
	float orthoL = 0.0f;	// Left orthographic
	float orthoR = 1.0f;	// Right orthographic
	float orthoB = 0.0f;	// Bottom orthographic
	float orthoT = 1.0f;	// Top orthographic

	// Window toggles
	int changeCubeTextureFlags;
	int cameraProjectionFlags = CAM_PROJ_PERSPECTIVE;

	bool showBoxSettings = false;

	// Cube
	std::vector<Cube> cubes;

	void updateCubes(Cube cube, CubeModFlags mod = INCREASE)
	{
		switch (mod)
		{
		case INCREASE:
			cubes.push_back(cube);
			break;
		case DECREASE:
			int pos = getCubePos(cube);

			if (pos == -1)
			{
				printf("Cannot find the cube with ID %d\n", cube.getPosID());
				return;
			}

			cubes.erase(cubes.begin() + pos);
			break;
		}
	}

	// Create a modal, use showModal() to show the modal and always end the modal by calling endModal()
	void beginModal(const char *name)
	{
		this->modalName = name;

		ImGui::OpenPopup(modalName.c_str());
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	}

	bool showModal()
	{
		return ImGui::BeginPopupModal(modalName.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize);
	}

	void endModal()
	{
		modalName.clear();
		ImGui::EndPopup();
	}

	// Show change texture modal. Automatically creates and ends the modal.
	void changeTexture(int index, Box &box)
	{
		String label = "Add/Change texture ";
		label.append(std::to_string(index));

		App.beginModal(label.c_str());
		if (App.showModal())
		{
			GLuint wrapper[] = { GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER };
			GLuint filters[] = { GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR };
			GLuint fmts[] = { GL_RGB, GL_RGBA };

			static int wrapperCurrent = 0;
			static int filterCurrent = 0;
			static int formatCurrent = 0;

			ImGui::Combo("Wrapper", &wrapperCurrent, "Repeat\0Mirrored-Repeat\0Clamp to edge\0Clamp to border");
			ImGui::Combo("Filter", &filterCurrent, "Nearest\0Linear\0Linear - Mipmap Nearest\0Linear - Mipmap Linear");
			ImGui::Combo("Format", &formatCurrent, "RGB\0RGBA");

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				box.createTexture(
					index,
					filePath.c_str(),
					wrapper[wrapperCurrent],
					wrapper[wrapperCurrent],
					filters[filterCurrent],
					filters[filterCurrent],
					fmts[formatCurrent]
				);

				filePath.clear();

				changeCubeTextureFlags &= ~(1 << index);
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				filePath.clear();

				changeCubeTextureFlags &= ~(1 << index);
			}

			App.endModal();
		}
	}

	String showOpenFileDialog()
	{
		String output;

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			IFileOpenDialog *fileOpenDialog;

			// Create the FileOpenDialog
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
				IID_IFileOpenDialog, reinterpret_cast<void **>(&fileOpenDialog));
			if (SUCCEEDED(hr))
			{
				// Show the Open File Dialog
				hr = fileOpenDialog->Show(NULL);

				// Get the file name from the dialog box
				if (SUCCEEDED(hr))
				{
					IShellItem *shItem;

					hr = fileOpenDialog->GetResult(&shItem);
					if (SUCCEEDED(hr))
					{
						PWSTR filePath;
						hr = shItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

						// Display the file name to the user
						if (SUCCEEDED(hr))
						{
							std::wstring wFilePath(filePath);
							output = String(wFilePath.begin(), wFilePath.end());

							CoTaskMemFree(filePath);
						}

						shItem->Release();
					}
				}

				fileOpenDialog->Release();
			}

			CoUninitialize();
		}

		return output;
	}

	unsigned int retrieveNewCubeID()
	{
		int id = 0;

		if (cubes.empty())
			return id;

		for (int i = 0; i < cubes.size(); i++)
		{
			if (cubes[i].getPosID() >= id)
				id = cubes[i].getPosID() + 1;
		}

		return id;
	}

	int getCubePos(Cube nCube)
	{
		int pos = -1;

		for (int i = 0; i < cubes.size(); i++)
		{
			if (cubes[i].getPosID() == nCube.getPosID())
				return pos = i;
		}

		return pos;
	}
} App;

template<typename T>
constexpr T clamp(T &v, T max, T min)
{
	T rv;

	if (v > max)
		v = max;

	if (v < min)
		v = min;

	return rv = v;
}

template <typename T>
constexpr int arrLen(T *arr)
{
	return sizeof(arr) / sizeof(*arr);
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
	box.createTexture(0, "container.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

	box.use();
	glUniform1i(glGetUniformLocation(box.getProgram(), "uTexture1"), 0);
	glUniform1i(glGetUniformLocation(box.getProgram(), "uTexture2"), 1);

	App.cam = glm::perspective(glm::radians(App.camFoV), (float)(App.width / App.height), App.nearPlane, App.farPlane);

	unsigned int modelLoc = glGetUniformLocation(box.getProgram(), "uModel");
	unsigned int viewLoc = glGetUniformLocation(box.getProgram(), "uView");
	unsigned int projLoc = glGetUniformLocation(box.getProgram(), "uProj");

	while (!glfwWindowShouldClose(window))
	{
		double updateTime = glfwGetTime();

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

				ImGui::Separator();

				if (ImGui::MenuItem("Exit", "Esc"))
					break;

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (App.showBoxSettings)
		{
			if (ImGui::Begin("Settings", &App.showBoxSettings))
			{
				static const char * const label[] = {
					"Window",
					"Graphics"
				};
				static int selectedItemSetting = 0;

				if (ImGui::BeginChild("Left panel", ImVec2(150, 0), true))
				{
					for (int i = 0; i < (sizeof(label) / sizeof(char *)); i++)
					{
						if (ImGui::Selectable(label[i], selectedItemSetting == i))
							selectedItemSetting = i;
					}
				}
				ImGui::EndChild();

				ImGui::SameLine();

				if (ImGui::BeginChild("Right panel", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())))
				{
					switch (selectedItemSetting)
					{
					case 0:		// Window
						ImGui::SeparatorText("Camera");
						{
							if (App.cameraProjectionFlags & App.CAM_PROJ_ORTHOGRAPHIC)
							{
								ImGui::Spacing();

								ImGui::DragFloat("Left", &App.orthoL, 1.0f, 0.0f, (float)App.width);
								ImGui::DragFloat("Right", &App.orthoR, 1.0f, 0.0f, (float)App.width);
								ImGui::DragFloat("Bottom", &App.orthoB, 1.0f, 0.0f, (float)App.height);
								ImGui::DragFloat("Top", &App.orthoT, 1.0f, 0.0f, (float)App.height);

								ImGui::Spacing();
								ImGui::Separator();
								ImGui::Spacing();
							}

							ImGui::SliderFloat("Field of View", &App.camFoV, 1.0f, 100.0f);
							ImGui::SliderFloat("Near plane", &App.nearPlane, -100.0f, 10.0f);
							ImGui::SliderFloat("Far plane", &App.farPlane, 0.0f, 100.0f);

							static int selectedCamProj = 1;
							if (ImGui::Combo("Projection", &selectedCamProj, "Orthographic\0Projection"))
							{
								App.cameraProjectionFlags = (1 << selectedCamProj);
								App.cam = glm::mat4(1.0f);
							}
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
								Cube cube(App.retrieveNewCubeID(), glm::vec3(0.0f, 0.0f, -3.0f));
								App.updateCubes(cube);
							}
							ImGui::SetItemTooltip("Add new cube");

							if (App.cubes.size() > 0)
							{
								if (ImGui::TreeNode("Cube settings"))
								{
									static int selectedItemCube;
									static char prevCombo[8];

									if (ImGui::BeginCombo("Cube selected", prevCombo))
									{
										for (int i = 0; i < App.cubes.size(); i++)
										{
											const bool isSelected = (selectedItemCube == i);
											char label[8];

											sprintf(label, "Cube %d", App.cubes[i].getPosID());
											
											if (ImGui::Selectable(label, isSelected))
												selectedItemCube = i;

											if (isSelected)
												ImGui::SetItemDefaultFocus();
										}
										ImGui::EndCombo();
									}

									unsigned int cubeID = App.cubes[selectedItemCube].getPosID();

									sprintf(prevCombo, "Cube %d", cubeID);

									Cube &cube = App.cubes.at(App.getCubePos(cubeID));

									static float textureMixRatio = 0.5f;

									ImGui::DragFloat3("Position", glm::value_ptr(cube.pos));
									ImGui::DragFloat3("Rotation", glm::value_ptr(cube.rot));
									ImGui::Checkbox("Auto rotation", &cube.isAutoRot);

									ImGui::DragFloat3("Scale", glm::value_ptr(cube.scale));

									if ((ImGui::ColorPicker4("Color", (float *)&cube.color)))
										glUniform4f(
											glGetUniformLocation(box.getProgram(), "uColor"), 
											cube.color.x, cube.color.y, cube.color.z, cube.color.w
										);

									for (int i = 0; i <= 1; i++)
									{
										String label = "Add/Change texture ";
										label.append(std::to_string(i));

										ImGui::Text("Texture %d", i);
										ImGui::Image(
											(ImTextureID)box.getTexture(i), 
											ImVec2(64, 64), 
											ImVec2(0.0f, 1.0f), 
											ImVec2(1.0f, 0.0f)
										);
										ImGui::SameLine();

										if (ImGui::Button(label.c_str()))
										{
											App.filePath = App.showOpenFileDialog();

											if (!App.filePath.empty())
												App.changeCubeTextureFlags |= (1 << i);
										}
									}

									ImGui::SliderFloat("Texture ratio", &textureMixRatio, 0.0f, 1.0f);
									glUniform1f(glGetUniformLocation(box.getProgram(), "uMix"), textureMixRatio);

									if (ImGui::Button("Remove"))
									{
										App.updateCubes(cube, App.DECREASE);
										selectedItemCube = 0;
									}
									ImGui::SetItemTooltip("Remove this Cube?");

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

		if (App.changeCubeTextureFlags & App.TEXTURE0)
			App.changeTexture(0, box);

		if (App.changeCubeTextureFlags & App.TEXTURE1)
			App.changeTexture(1, box);

		if (!App.cubes.empty())
		{
			for (Cube &cube : App.cubes)
			{
				cube.model = glm::mat4(1.0f);
				cube.view = glm::mat4(1.0f);

				// Use glfwTime() if rotation is continuous
				float ut = (cube.isAutoRot) ? (float)(updateTime) : 1;

				cube.model = glm::scale(cube.model, cube.scale);

				if (cube.rot.x != 0.0f)
				{
					cube.model = glm::rotate(
						cube.model,
						glm::radians(cube.rot.x * ut / 10.0f),
						glm::vec3(1.0f, 0.0f, 0.0f)
					);
				}

				if (cube.rot.y != 0.0f)
				{
					cube.model = glm::rotate(
						cube.model,
						glm::radians(cube.rot.y * ut / 10.0f),
						glm::vec3(0.0f, 1.0f, 0.0f)
					);
				}
				
				if (cube.rot.z != 0.0f)
				{
					cube.model = glm::rotate(
						cube.model,
						glm::radians(cube.rot.z * ut / 10.0f),
						glm::vec3(0.0f, 0.0f, 1.0f)
					);
				}

				cube.view = glm::translate(cube.view, cube.pos);

				box.draw();

				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cube.model));
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cube.view));
			}
		}

		ImGui::Render();

		//int displayW, displayH;
		glfwGetFramebufferSize(window, &App.width, &App.height);
		glViewport(0, 0, App.width, App.height);

		if (App.cameraProjectionFlags & App.CAM_PROJ_ORTHOGRAPHIC)
			App.cam = glm::ortho(App.orthoL, App.orthoR, App.orthoB, App.orthoT, App.nearPlane, App.farPlane);
		else if (App.cameraProjectionFlags & App.CAM_PROJ_PERSPECTIVE)
			App.cam = glm::perspective(glm::radians(App.camFoV), (float)(App.width / App.height), App.nearPlane, App.farPlane);

		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(App.cam));

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