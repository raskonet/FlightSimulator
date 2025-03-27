#ifndef OPENGL_UTILS_H
#define OPENGL_UTILS_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <stdexcept> // For runtime_error

namespace GLUtil {

// Define primitive restart index value globally accessible
constexpr GLuint PRIMITIVE_RESTART_INDEX = 0xFFFFU;

// Simple Vertex Buffer Object Wrapper
class VertexBuffer {
public:
    GLuint ID = 0;
    VertexBuffer() { glGenBuffers(1, &ID); }
    ~VertexBuffer() { if (ID != 0) glDeleteBuffers(1, &ID); }

    // Disable copy/move for simplicity
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&&) = delete;
    VertexBuffer& operator=(VertexBuffer&&) = delete;

    void bind() const { glBindBuffer(GL_ARRAY_BUFFER, ID); }
    void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    template<typename T>
    void buffer(const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW) {
        bind();
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), usage);
    }
     void buffer(const void* data, size_t sizeBytes, GLenum usage = GL_STATIC_DRAW) {
        bind();
        glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, usage);
    }
};

// Simple Element Buffer Object Wrapper
class ElementBufferObject {
public:
    GLuint ID = 0;
    ElementBufferObject() { glGenBuffers(1, &ID); }
    ~ElementBufferObject() { if (ID != 0) glDeleteBuffers(1, &ID); }

    // Disable copy/move
    ElementBufferObject(const ElementBufferObject&) = delete;
    ElementBufferObject& operator=(const ElementBufferObject&) = delete;
    ElementBufferObject(ElementBufferObject&&) = delete;
    ElementBufferObject& operator=(ElementBufferObject&&) = delete;

    void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }
    void unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

    template<typename T>
    void buffer(const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW) {
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), usage);
    }
     void buffer(const void* data, size_t sizeBytes, GLenum usage = GL_STATIC_DRAW) {
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeBytes, data, usage);
    }
};

// Simple Vertex Array Object Wrapper
class VertexArrayObject {
public:
    GLuint ID = 0;
    VertexArrayObject() { glGenVertexArrays(1, &ID); }
    ~VertexArrayObject() { if (ID != 0) glDeleteVertexArrays(1, &ID); }

    // Disable copy/move
    VertexArrayObject(const VertexArrayObject&) = delete;
    VertexArrayObject& operator=(const VertexArrayObject&) = delete;
    VertexArrayObject(VertexArrayObject&&) = delete;
    VertexArrayObject& operator=(VertexArrayObject&&) = delete;

    void bind() const { glBindVertexArray(ID); }
    void unbind() const { glBindVertexArray(0); }
};

// Simple Texture Parameter Struct
struct TextureParams {
    GLenum texture_wrap = GL_CLAMP_TO_EDGE;
    GLenum texture_mag_filter = GL_LINEAR;
    GLenum texture_min_filter = GL_LINEAR_MIPMAP_LINEAR;
};

} // namespace GLUtil

#endif // OPENGL_UTILS_H
