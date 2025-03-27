#include "Texture.h"
#include <iostream>
#include <utility> // For std::swap

// Define STB_IMAGE_IMPLEMENTATION in this file only, before including stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Constructor implementation
Texture::Texture(const char* path, const GLUtil::TextureParams& params)
    : ID(0), Width(0), Height(0), NrChannels(0) // Initialize members
{
    glGenTextures(1, &ID);
    if (ID == 0) { // Check if glGenTextures failed (rare)
         std::cerr << "Error: Failed to generate texture handle." << std::endl;
         return;
    }
    // Bind the texture handle immediately so settings apply to it
    glBindTexture(GL_TEXTURE_2D, ID);
    if (!loadTexture(path, params)) { // Pass params to loadTexture
        glDeleteTextures(1, &ID); // Clean up if loading fails
        ID = 0; // Mark as invalid
        std::cerr << "Error: Failed to load texture: " << path << std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind after setup/loading attempt
}


Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}

// Move constructor
Texture::Texture(Texture&& other) noexcept
    : ID(other.ID), Width(other.Width), Height(other.Height), NrChannels(other.NrChannels) {
    other.ID = 0; // Leave moved-from object in a valid (but empty) state
    other.Width = 0;
    other.Height = 0;
    other.NrChannels = 0;
}

// Move assignment operator
Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        // Delete existing resource if any
        if (ID != 0) {
            glDeleteTextures(1, &ID);
        }
        // Transfer ownership
        ID = other.ID;
        Width = other.Width;
        Height = other.Height;
        NrChannels = other.NrChannels;
        // Reset moved-from object
        other.ID = 0;
        other.Width = 0;
        other.Height = 0;
        other.NrChannels = 0;
    }
    return *this;
}


// loadTexture implementation taking params
bool Texture::loadTexture(const char* path, const GLUtil::TextureParams& params) {
    // Texture ID should already be bound here

    // Set texture wrapping/filtering options from params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.texture_wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.texture_wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.texture_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.texture_mag_filter);

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &Width, &Height, &NrChannels, 0);
    if (data) {
        GLenum internalFormat = GL_RGB;
        GLenum dataFormat = GL_RGB;
         if (NrChannels == 1) { internalFormat = GL_RED; dataFormat = GL_RED; }
         else if (NrChannels == 3) { internalFormat = GL_RGB; dataFormat = GL_RGB; } // Consider GL_SRGB
         else if (NrChannels == 4) { internalFormat = GL_RGBA; dataFormat = GL_RGBA; } // Consider GL_SRGB_ALPHA
         else {
            std::cerr << "Warning: Unsupported texture channels (" << NrChannels << ") in: " << path << std::endl;
            stbi_image_free(data);
            return false;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Width, Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        // Generate mipmaps if min filter uses them
        if (params.texture_min_filter == GL_NEAREST_MIPMAP_NEAREST ||
            params.texture_min_filter == GL_LINEAR_MIPMAP_NEAREST ||
            params.texture_min_filter == GL_NEAREST_MIPMAP_LINEAR ||
            params.texture_min_filter == GL_LINEAR_MIPMAP_LINEAR) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        stbi_image_free(data);
        return true;
    } else {
        std::cerr << "Error: Failed to load texture data from: " << path << std::endl;
        std::cerr << "STB Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }
}

void Texture::bind(GLuint textureUnit) const {
    if (!isValid()) return; // Don't bind invalid texture
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, ID);
}
