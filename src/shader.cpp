#include "shader.h"

#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// Constructor implementation
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    // Read shader code from files
    std::ifstream vertexFile(vertexPath);
    std::ifstream fragmentFile(fragmentPath);

    if (!vertexFile.is_open() || !fragmentFile.is_open())
    {
        std::cerr << "Failed to open shader files.\n";
        ID = 0;
        return;
    }


    std::stringstream vertexStream;
    std::stringstream fragmentStream;

    vertexStream << vertexFile.rdbuf();
    fragmentStream << fragmentFile.rdbuf();

    std::string vertexCode = vertexStream.str();
    std::string fragmentCode = fragmentStream.str();

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << '\n';
    }

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << '\n';
    }

    // Create shader program and link shaders
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << '\n';
    }

    // Shader objects are no longer needed after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// Activate the shader program
void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setMat4(const std::string& name, const float* value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    glUniform3f(location, x, y, z);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}