#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <string>

class Texture {
public:
    GLuint ID;
    int Width, Height, NrChannels;

    Texture(const char* path);
    ~Texture();

    void bind(GLuint textureUnit = GL_TEXTURE0) const;

private:
    bool loadTexture(const char* path);
};

#endif // TEXTURE_H
