#ifndef SHADER_H
#define SHADER_H

#include <string>

// A minimal shader class for loading, compiling, and using OpenGL shaders.
class Shader
{
public:
    unsigned int ID;

    // Constructor: load shader source code from files, compile, and link them
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    // Activate the shader program
    void use() const;

    void setMat4(const std::string& name, const float* value) const;
	void setVec3(const std::string& name, float x, float y, float z) const;

    void setInt(const std::string& name, int value) const;
};

#endif