#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <string>
#include "OpenGLUtils.h" // Include for TextureParams definition

class Texture {
public:
    GLuint ID;
    int Width, Height, NrChannels;

    // Default constructor for invalid texture state
    Texture() : ID(0), Width(0), Height(0), NrChannels(0) {}
    // Constructor that loads from path
    Texture(const char* path, const GLUtil::TextureParams& params = {}); // Added params
    ~Texture();

    // Prevent copying, allow moving
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;


    void bind(GLuint textureUnit = GL_TEXTURE0) const;
    bool isValid() const { return ID != 0; }

private:
    bool loadTexture(const char* path, const GLUtil::TextureParams& params); // Added params
};

#endif // TEXTURE_H
