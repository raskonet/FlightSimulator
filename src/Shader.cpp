#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) : ID(0) { // Initialize ID
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // Ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (const std::ifstream::failure& e) { // Catch by const reference
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        std::cerr << "Vertex Path: " << vertexPath << std::endl;
        std::cerr << "Fragment Path: " << fragmentPath << std::endl;
        // ID is already 0, indicating failure
        return;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. Compile shaders
    GLuint vertex = 0, fragment = 0; // Initialize handles
    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // Shader Program
    ID = glCreateProgram();
    if (ID == 0) {
        std::cerr << "ERROR::SHADER::PROGRAM_CREATION_FAILED" << std::endl;
        if (vertex != 0) glDeleteShader(vertex);
        if (fragment != 0) glDeleteShader(fragment);
        return;
    }
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Delete the shaders as they're linked into our program now and no longer necessary
    // Check handles before deleting
    if (vertex != 0) glDeleteShader(vertex);
    if (fragment != 0) glDeleteShader(fragment);

     // Check link status after deleting shaders? Or before? Usually check link status first.
     GLint success;
     glGetProgramiv(ID, GL_LINK_STATUS, &success);
     if (!success) {
         // ID might be invalid after failed link, maybe cleanup?
         // For now, ID remains non-zero but program is unusable.
         std::cerr << "Shader program linking failed." << std::endl;
         // Optionally delete program: glDeleteProgram(ID); ID = 0;
     }
}

Shader::~Shader() {
    if (ID != 0) {
        // Check if program is actually marked for deletion before deleting?
        // GLint deleteStatus;
        // glGetProgramiv(ID, GL_DELETE_STATUS, &deleteStatus);
        // if (!deleteStatus) { // Only delete if not already marked
             glDeleteProgram(ID);
        // }
    }
}

// --- Implementation of modified use() method ---
void Shader::use(bool activate) const {
    if (!activate) {
        glUseProgram(0); // Deactivate shader program
    } else if (ID != 0) { // Only activate if ID is valid
        glUseProgram(ID);
    } else {
        // Optionally log error if trying to use an invalid shader
        // std::cerr << "Warning: Attempting to use invalid shader program (ID=0)" << std::endl;
    }
}

// --- checkCompileErrors Implementation ---
void Shader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

// --- Utility uniform functions implementation ---
void Shader::setBool(const std::string &name, bool value) const { if(ID) glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
void Shader::setInt(const std::string &name, int value) const { if(ID) glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setFloat(const std::string &name, float value) const { if(ID) glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setVec2(const std::string &name, const glm::vec2 &value) const { if(ID) glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec2(const std::string &name, float x, float y) const { if(ID) glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); }
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const { if(ID) glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec3(const std::string &name, float x, float y, float z) const { if(ID) glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); }
void Shader::setVec4(const std::string &name, const glm::vec4 &value) const { if(ID) glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const { if(ID) glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); }
void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const { if(ID) glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const { if(ID) glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const { if(ID) glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
