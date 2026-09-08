#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
	constexpr int WindowWidth = 900;
	constexpr int WindowHeight = 600;

	void glfwErrorCallback(int error, const char* description)
	{
		std::cerr << "GLFW error (" << error << "): " << description << '\n';
	}

	std::string readTextFile(const std::string& path)
	{
		std::ifstream file(path);
		if (!file)
		{
			throw std::runtime_error("Could not open file: " + path);
		}

		std::ostringstream contents;
		contents << file.rdbuf();
		return contents.str();
	}

	GLuint compileShader(GLenum type, const std::string& source, const std::string& label)
	{
		const GLuint shader = glCreateShader(type);
		const char* sourcePtr = source.c_str();

		glShaderSource(shader, 1, &sourcePtr, nullptr);
		glCompileShader(shader);

		GLint success = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint logLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

			std::string log(static_cast<std::size_t>(logLength), '\0');
			glGetShaderInfoLog(shader, logLength, nullptr, log.data());

			glDeleteShader(shader);
			throw std::runtime_error("Shader compilation failed (" + label + "):\n" + log);
		}

		return shader;
	}

	GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath)
	{
		const std::string vertexSource = readTextFile(vertexPath);
		const std::string fragmentSource = readTextFile(fragmentPath);

		const GLuint vertexShader =
			compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
		const GLuint fragmentShader =
			compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

		const GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		GLint success = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint logLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

			std::string log(static_cast<std::size_t>(logLength), '\0');
			glGetProgramInfoLog(program, logLength, nullptr, log.data());

			glDeleteProgram(program);
			throw std::runtime_error("Shader program link failed:\n" + log);
		}

		return program;
	}

	void framebufferSizeCallback(GLFWwindow*, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void buildGridMesh(int resolution, float size, std::vector<float>& vertices)
	{
		vertices.clear();
		const float start = -size / 2.0f;
		const float step = size / resolution;

		for (int z = 0; z < resolution; ++z)
		{
			for (int x = 0; x < resolution; ++x)
			{
				float x0 = start + x * step;
				float z0 = start + z * step;
				float x1 = x0 + step;
				float z1 = z0 + step;

				vertices.push_back(x0); vertices.push_back(0.0f); vertices.push_back(z0);
				vertices.push_back(x1); vertices.push_back(0.0f); vertices.push_back(z0);
				vertices.push_back(x0); vertices.push_back(0.0f); vertices.push_back(z1);

				vertices.push_back(x0); vertices.push_back(0.0f); vertices.push_back(z1);
				vertices.push_back(x1); vertices.push_back(0.0f); vertices.push_back(z0);
				vertices.push_back(x1); vertices.push_back(0.0f); vertices.push_back(z1);
			}
		}
	}
	int currentNormalMode = 1;
	int currentResolution = 150;
	bool meshNeedsUpdate = false;

	void processInput(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && currentResolution != 25)
		{
			currentResolution = 25;
			meshNeedsUpdate = true;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && currentResolution != 75)
		{
			currentResolution = 75;
			meshNeedsUpdate = true;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && currentResolution != 150)
		{
			currentResolution = 150;
			meshNeedsUpdate = true;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && currentResolution != 1000)
		{
			currentResolution = 1000;
			meshNeedsUpdate = true;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) currentNormalMode = 0; //Approx
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) currentNormalMode = 1; //Exact
		if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) currentNormalMode = 2; //Finite Diff
	}
}

int main()
{
	glfwSetErrorCallback(glfwErrorCallback);
	if (glfwInit() != GLFW_TRUE)
	{
		std::cerr << "Failed to initialize GLFW.\n";
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	GLFWwindow* window =
		glfwCreateWindow(WindowWidth, WindowHeight, "Water Shader Benchmarking", nullptr, nullptr);

	if (window == nullptr)
	{
		std::cerr << "Failed to create a GLFW window.\n";
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSwapInterval(0);

	const int loadedVersion = gladLoadGL(glfwGetProcAddress);
	if (loadedVersion == 0)
	{
		std::cerr << "Failed to load OpenGL functions with GLAD.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	std::vector<float> vertices;
	const float meshSize = 10.0f;
	buildGridMesh(currentResolution, meshSize, vertices);

	GLuint vao = 0;
	GLuint vbo = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

	constexpr GLsizei stride = 3 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	GLuint shaderProgram = 0;
	try
	{
		shaderProgram = createShaderProgram("shaders/basic.vert", "shaders/basic.frag");
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	const GLint modelLocation = glGetUniformLocation(shaderProgram, "model");
	const GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
	const GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
	const GLint timeLocation = glGetUniformLocation(shaderProgram, "time");
	const GLint normalModeLocation = glGetUniformLocation(shaderProgram, "normalMode");

	const glm::mat4 model(1.0f);
	const glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 3.0f, 5.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	const float fieldOfView = glm::radians(45.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 100.0f;

	double lastTime = glfwGetTime();
	int frameCount = 0;

	while (glfwWindowShouldClose(window) == GLFW_FALSE)
	{
		processInput(window);

		if (meshNeedsUpdate)
		{
			buildGridMesh(currentResolution, meshSize, vertices);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
			meshNeedsUpdate = false;
		}

		double currentTime = glfwGetTime();
		frameCount++;
		if (currentTime - lastTime >= 0.25)
		{
			double fps = static_cast<double>(frameCount) / (currentTime - lastTime);
			double frameTimeMs = ((currentTime - lastTime) / frameCount) * 1000.0;
			int triangleCount = static_cast<int>(vertices.size() / 9);

			std::string modeStr = (currentNormalMode == 0) ? "Approx" : (currentNormalMode == 1) ? "Exact" : "FinDiff";

			std::string title = "Res: " + std::to_string(currentResolution) + "x" + std::to_string(currentResolution) +
				" | Mode: " + modeStr +
				" | " + std::to_string(static_cast<int>(fps)) + " FPS | " +
				std::to_string(frameTimeMs).substr(0, 5) + " ms";

			glfwSetWindowTitle(window, title.c_str());
			frameCount = 0;
			lastTime = currentTime;
		}

		int framebufferWidth = 0;
		int framebufferHeight = 0;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

		if (framebufferWidth == 0 || framebufferHeight == 0)
		{
			glfwPollEvents();
			continue;
		}

		const float aspectRatio = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
		const glm::mat4 projection = glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);

		glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1f(timeLocation, static_cast<float>(glfwGetTime()));
		glUniform1i(normalModeLocation, currentNormalMode);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 3));

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}