#pragma once

#include "platform.h"
//#include <glad/glad.h>
#include <GL/glew.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include "stb/stb_image.h"

static std::string
get_file_contents(const char *filename)
{
    std::ifstream in(filename, std::ios::binary);

    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return (contents);
    }

    throw (errno);
}

static void
compileErrors(u32 shader, const char *type)
{
    // Stores status of compilation
    s32  hasCompiled;
    // Character array to store error message in
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n"
                      << infoLog << std::endl;
        }
    } else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for:" << type << "\n"
                      << infoLog << std::endl;
        }
    }
}

static u32
createShaders(const char *vertexFile, const char *fragmentFile)
{
    std::string vertexCode   = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);

    const char *vertexSource   = vertexCode.c_str();
    const char *fragmentSource = fragmentCode.c_str();

    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    compileErrors(vertexShader, "VERTEX");

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    compileErrors(fragmentShader, "FRAGMENT");

    u32 program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    compileErrors(program, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

inline void
SetBoolUniform(const char *Name, b32 Value, u32 ShaderID)
{
    glUniform1i(glGetUniformLocation(ShaderID, Name), (int) Value);
}

// ------------------------------------------------------------------------
inline void
SetIntUniform(const char *Name, int Value, u32 ShaderID)
{
    glUniform1i(glGetUniformLocation(ShaderID, Name), Value);
}

// ------------------------------------------------------------------------
inline void
SetFloatUniform(const char *Name, r32 Value, u32 ShaderID)
{
    glUniform1f(glGetUniformLocation(ShaderID, Name), Value);
}

// ------------------------------------------------------------------------
inline void
SetVec2Uniform(const char *Name, const glm::vec2 &Value, u32 ShaderID)
{
    glUniform2fv(glGetUniformLocation(ShaderID, Name), 1, &Value[0]);
}

inline void
SetVec2Uniform(const char *Name, r32 x, r32 y, u32 ShaderID)
{
    glUniform2f(glGetUniformLocation(ShaderID, Name), x, y);
}

inline void
SetVec3Uniform(const char *Name, glm::vec3 Value, u32 ShaderID)
{
    glUniform3fv(glGetUniformLocation(ShaderID, Name), 1, glm::value_ptr(Value));
}

inline void
SetVec3Uniform(const char *Name, r32 x, r32 y, r32 z, u32 ShaderID)
{
    glUniform3f(glGetUniformLocation(ShaderID, Name), x, y, z);
}

// ------------------------------------------------------------------------
inline void
SetVec4Uniform(const char *Name, glm::vec4 Value, u32 ShaderID)
{
    glUniform4fv(glGetUniformLocation(ShaderID, Name), 1, glm::value_ptr(Value));
}

inline void
SetVec4Uniform(const char *Name, r32 x, r32 y, r32 z, r32 w, u32 ShaderID)
{
    glUniform4f(glGetUniformLocation(ShaderID, Name), x, y, z, w);
}

inline void
SetTextUniform(const char *Name, s32 unit, u32 shaderProgram)
{
    s32 texUniform = glGetUniformLocation(shaderProgram, Name);
    glUseProgram(shaderProgram);
    glUniform1i(texUniform, unit);
}

inline void
SetMat4Uniform(const char *Name, glm::mat4 matrix, u32 shaderProgram)
{
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, Name), 1, GL_FALSE, glm::value_ptr(matrix));
}

inline u32
createVBO(FileData Data)
{
    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, Data.size, Data.data, GL_STATIC_DRAW);

    return VBO;
}

inline u32
createEBO(FileData Data)
{
    u32 EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Data.size, Data.data, GL_STATIC_DRAW);

    return EBO;
}

inline u32
createVBO(std::vector<Vertex> &vertices)
{
    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    return VBO;
}

inline u32
createVBO(std::vector<float> &vertices)
{
    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    return VBO;
}

inline u32
createEBO(std::vector<u32> &indices)
{
    u32 EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_STATIC_DRAW);

    return EBO;
}

inline u32
createVAO()
{
    u32 VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    return VAO;
}

inline void
linkAttrib(u32 VBO, u32 layout, GLint numComponent, GLenum type, GLsizeiptr stride, void *offset)
{
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(layout, numComponent, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
}

inline void
unbindVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

inline void
unbindVAO()
{
    glBindVertexArray(0);
}

inline void
unbindEBO()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

inline void
bindVAO(u32 VAO)
{
    glBindVertexArray(VAO);
}

static void
UploadTexture(texture *texture, unsigned char *bytes, int imgWidth, int imgHeight, int numColCh)
{
    glGenTextures(1, &texture->ID);
    glActiveTexture(GL_TEXTURE0 + texture->unit);
    glBindTexture(GL_TEXTURE_2D, texture->ID);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GLFW_CONTEXT_VERSION_MAJOR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, format, pixelType, bytes);

    GLenum format;

    if (numColCh == 1)
    {
        format = GL_RED;
    } else if (numColCh == 3)
    {
        format = GL_RGB;
    } else if (numColCh == 4)
    {
        format = GL_RGBA;
    } else
    {
        throw std::invalid_argument("Automatic Texture type recognition failed");
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, bytes);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void
LoadTexture(const char *filename, texture *texture)
{
    int imgWidth, imgHeight, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *bytes = stbi_load(filename, &imgWidth, &imgHeight, &numColCh, 0);

    UploadTexture(texture, bytes, imgWidth, imgHeight, numColCh);
}

inline void
bindTexture(texture *texture)
{
    glActiveTexture(GL_TEXTURE0 + texture->unit);
    glBindTexture(GL_TEXTURE_2D, texture->ID);
}
