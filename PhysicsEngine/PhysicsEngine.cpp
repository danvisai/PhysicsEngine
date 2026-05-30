// PhysicsEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <fstream>
#include<sstream>
#include<string>
//shader file reading code

static std::string readFile(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

//compile a single shader stage (vertex, fragment, geometry, etc)
//returns the shader id if compilation is successful, otherwise returns 0
static GLuint compileShader(GLenum stage, const std::string& src) {
	//create empty shader object of the given type
	//glfragment or compute or vertex shader
	GLuint shader = glCreateShader(stage);
	//attach our source code to it the signature is awkwardly C-flavoured: (shader, numofstrings, arrayofstringspointers, arrayoflengths)
	//numstruings =1 because we have one source of string
	// array of lengths = nullpts means "the strings are null terminated", figure
	//out lengths yourself. both piecs let GL accept source into splits into multiple chunks which we do not need
	const char* csrc = src.c_str();
	glShaderSource(shader, 1, &csrc, nullptr);
	//actually run the glsl compiler on the attached source
	glCompileShader(shader);

	//was it ok?  glgetshaderiv reads an intege property from the shader object
	GLint ok = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char infoLog[1024];
		glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "Shader compile error:\n" << infoLog << "\n";
		glDeleteShader(shader);
		return 0;
	}
	return shader;



}

// Build a complete program from a vertex and fragment shader source pair.
// Returns the program name on success, 0 on failure.
static GLuint makeProgram(const std::string& vsPath, const std::string& fsPath) {
	GLuint vs = compileShader(GL_VERTEX_SHADER, readFile(vsPath));
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, readFile(fsPath));
	if (!vs || !fs) return 0;

	// Create an empty program object.
	GLuint prog = glCreateProgram();

	// Attach the compiled shader stages.
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	// Link: verify that the stages' outputs and inputs are compatible,
	// resolve uniform locations, finalize everything.
	glLinkProgram(prog);

	// Check link status, same pattern as compile.
	GLint ok = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		char infoLog[1024];
		glGetProgramInfoLog(prog, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "Program link error:\n" << infoLog << "\n";
		glDeleteProgram(prog);
		glDeleteShader(vs);
		glDeleteShader(fs);
		return 0;
	}

	// Once linked, the program has its own internal copy of the compiled
	// code. The shader objects are no longer needed � delete them.
	// This is a common point of confusion: people think deleting the shader
	// breaks the program. It doesn't. The program is self-contained after linking.
	glDeleteShader(vs);
	glDeleteShader(fs);

	return prog;
}


static void GLAPIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id, 
											GLenum severity, GLsizei length, const GLchar* message,const void* userParam)
{
	std::cerr << "OpenGL Debug Message: " << message << std::endl;
	std::cerr << "Source: " << source << ", Type: " << type << ", ID: " << id << ", Severity: " << severity << std::endl;
}


int main()
{
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return 1;
	}

	///window hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //major version of opengl
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5); //minor version of opengl
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //core profile means we will not have access to deprecated features of opengl
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); //enable debug context for better error reporting and debugging
	// create a window

	GLFWwindow* window = glfwCreateWindow(1280, 720, "PhysicsEngine", nullptr, nullptr); //last two pointers are for fulscreen and shared context both are disabbled for now

	if (!window) {
		std::cerr << "failed to create window" << std::endl;
		glfwTerminate();
		return 1;
	}
	//after window creation glfw window needs to be set as current context for this
	glfwMakeContextCurrent(window);

	//glad looks up every gl function pointer and loads the address of the function in the driver so that we can call it later
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return 1;
	}
	//now that we have a valid opengl context and glad is loaded we can call any opengl function
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglDebugCallback, nullptr);

	glfwSwapInterval(1); //enable vsync


	//vertices of a triangle
	float vertices[] = {
		0.0f,  0.5f, 0.0f, //top vertex
		-0.5f, -0.5f, 0.0f, //bottom left vertex
		0.5f, -0.5f, 0.0f //bottom right vertex
	};

	//vbo to upload vertex data to gpu
	GLuint vbo;
	glCreateBuffers(1, &vbo); //create 1 buffer and store its id in vbo
	//glGenBuffers(1, &vbo); //generate 1 buffer and store its id in vbo
	//glBindBuffer(GL_ARRAY_BUFFER, vbo); //bind the buffer to the GL_ARRAY_BUFFER target
	glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW); //upload vertex data to gpu


	GLuint vao;
	glCreateVertexArrays(1, &vao); //create 1 vertex array object and store its id in vao
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * 3); //bind the vbo to the vao at binding index 0 with an offset of 0 and a stride of 3 floats (x,y,z)
	glEnableVertexArrayAttrib(vao, 0); //enable the vertex attribute at index 0
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0); //specify the format of the vertex attribute at index 0 (3 floats for x,y,z)
	glVertexArrayAttribBinding(vao, 0, 0); //bind the vertex attribute at index 0 to the vertex buffer binding index 0
	// SHADER_DIR is the macro we set up in CMakeLists.txt.
	// It expands to a string literal pointing at our shaders/ folder.
	std::string vsPath = std::string(SHADER_DIR) + "/triangle.vert";
	std::string fsPath = std::string(SHADER_DIR) + "/triangle.frag";

	GLuint program = makeProgram(vsPath, fsPath);
	if (!program) {
		std::cerr << "Shader program creation failed, exiting.\n";
		return 1;
	}
	//main loop
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
