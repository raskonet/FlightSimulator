#include "Texture.h"
#include <iostream>

// Define STB_IMAGE_IMPLEMENTATION in this file only, before including stb_image.h
#define STB_IMAGE_IMPLEMENTATION // <--- ADD THIS LINE
#include <stb_image.h>           // <--- Include AFTER the define

// Ensure GL types are known (comes via Texture.h -> glew.h or include glew.h directly)
#include <GL/glew.h>


Texture::Texture(const char* path) : ID(0), Width(0), Height(0), NrChannels(0) {
    glGenTextures(1, &ID);
    // Bind the texture handle immediately so settings apply to it
    glBindTexture(GL_TEXTURE_2D, ID);
    if (!loadTexture(path)) {
        // If loading failed, cleanup the potentially generated texture handle
        glDeleteTextures(1, &ID);
        ID = 0; // Indicate failure
        std::cerr << "Error: Failed to load texture: " << path << std::endl;
        // Optionally unbind texture handle (though it's 0 now if deleted)
        // glBindTexture(GL_TEXTURE_2D, 0);
    }
     // Keep texture bound after successful load? Depends on usage, often unbound here.
     // glBindTexture(GL_TEXTURE_2D, 0); // Unbind after setup
}

Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
        ID = 0; // Good practice
    }
}

bool Texture::loadTexture(const char* path) {
    // Texture should already be bound from the constructor or bind call

    // Set texture wrapping/filtering options (on the currently bound texture ID)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image using stb_image
    stbi_set_flip_vertically_on_load(true); // Match OpenGL coordinate system (0,0 at bottom-left)
    unsigned char *data = stbi_load(path, &Width, &Height, &NrChannels, 0);
    if (data) {
        GLenum internalFormat = GL_RGB; // Default format
        GLenum dataFormat = GL_RGB;
        if (NrChannels == 1) {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        } else if (NrChannels == 3) {
            internalFormat = GL_RGB; // Often use GL_SRGB for color textures if doing gamma correction
            dataFormat = GL_RGB;
        } else if (NrChannels == 4) {
            internalFormat = GL_RGBA; // Often use GL_SRGB_ALPHA for color+alpha textures
            dataFormat = GL_RGBA;
        } else {
            std::cerr << "Warning: Unsupported number of channels (" << NrChannels << ") in texture: " << path << std::endl;
            stbi_image_free(data); // Free loaded data even if format is wrong
            return false; // Indicate failure
        }

        // Upload texture data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Width, Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        // Generate mipmaps AFTER uploading the base level
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the CPU-side image data, it's now on the GPU
        stbi_image_free(data);
        return true; // Success
    } else {
        std::cerr << "Error: Failed to load texture data from: " << path << std::endl;
        std::cerr << "STB Reason: " << stbi_failure_reason() << std::endl;
        return false; // Failure
    }
}

void Texture::bind(GLuint textureUnit) const {
    if (ID == 0) {
        // std::cerr << "Warning: Attempting to bind invalid texture (ID=0)" << std::endl;
        return; // Don't try to bind an invalid texture
    }
    glActiveTexture(textureUnit); // Activate the desired texture unit
    glBindTexture(GL_TEXTURE_2D, ID); // Bind this texture's ID to the active unit
}
