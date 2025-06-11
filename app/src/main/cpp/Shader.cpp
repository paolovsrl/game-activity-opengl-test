#include "Shader.h"

#include "AndroidOut.h"
#include "Model.h"
#include "Utility.h"

Shader *Shader::loadShader(
        const std::string &vertexSource,
        const std::string &fragmentSource,
        const std::string &positionAttributeName,
        const std::string &uvAttributeName,
        const std::string &projectionMatrixUniformName) {
    Shader *shader = nullptr;

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader) {
        return nullptr;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmentShader) {
        aout << "Failed to load fragment shader" << std::endl;
        glDeleteShader(vertexShader);
        return nullptr;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

            // If we fail to link the shader program, log the result for debugging
            if (logLength) {
                GLchar *log = new GLchar[logLength];
                glGetProgramInfoLog(program, logLength, nullptr, log);
                aout << "Failed to link program with:\n" << log << std::endl;
                delete[] log;
            }

            glDeleteProgram(program);
        } else {
            // Get the attribute and uniform locations by name. You may also choose to hardcode
            // indices with layout= in your shader, but it is not done in this sample
            // ... after linking ...
            GLint positionAttribute = glGetAttribLocation(program, positionAttributeName.c_str());
            if (positionAttribute == -1) {
                aout << "Failed to find required attribute: " << positionAttributeName << std::endl;
                glDeleteProgram(program);
                return nullptr; // Position is likely always required
            }

            GLint uvAttribute = -1; // Default to not found
            if (!uvAttributeName.empty()) { // Only try to get it if a name is provided
                uvAttribute = glGetAttribLocation(program, uvAttributeName.c_str());
                if (uvAttribute == -1) {
                    aout << "Warning: Could not find UV attribute: " << uvAttributeName
                         << ". This may be expected if the shader doesn't use it." << std::endl;
                    // Don't fail here, just record -1. The Shader object needs to handle it.
                }
            }

            GLint projectionMatrixUniform = glGetUniformLocation(program, projectionMatrixUniformName.c_str());
            if (projectionMatrixUniform == -1) {
                aout << "Failed to find required uniform: " << projectionMatrixUniformName << std::endl;
                glDeleteProgram(program);
                return nullptr; // Projection is likely always required
            }
            // Only create a new shader if all the attributes are found.
            if (positionAttribute != -1
               // && uvAttribute != -1
                && projectionMatrixUniform != -1) {

                shader = new Shader(
                        program,
                        positionAttribute,
                        uvAttribute,
                        projectionMatrixUniform);
            } else {
                glDeleteProgram(program);
                aout << "Failed to find attribute or uniform "  << positionAttribute << " " << uvAttribute << " " << projectionMatrixUniform << std::endl;
            }
        }
    }

    // The shaders are no longer needed once the program is linked. Release their memory.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shader;
}

GLuint Shader::loadShader(GLenum shaderType, const std::string &shaderSource) {
    Utility::assertGlError();
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        auto *shaderRawString = (GLchar *) shaderSource.c_str();
        GLint shaderLength = shaderSource.length();
        glShaderSource(shader, 1, &shaderRawString, &shaderLength);
        glCompileShader(shader);

        GLint shaderCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);

        // If the shader doesn't compile, log the result to the terminal for debugging
        if (!shaderCompiled) {
            GLint infoLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

            if (infoLength) {
                auto *infoLog = new GLchar[infoLength];
                glGetShaderInfoLog(shader, infoLength, nullptr, infoLog);
                aout << "Failed to compile with:\n" << infoLog << std::endl;
                delete[] infoLog;
            }

            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

void Shader::activate() const {
    glUseProgram(program_);
}

void Shader::deactivate() const {
    glUseProgram(0);
}

void Shader::drawModel(const Model &model) const {
    // The position attribute is 3 floats
    glVertexAttribPointer(
            position_, // attrib
            3, // elements
            GL_FLOAT, // of type float
            GL_FALSE, // don't normalize
            sizeof(Vertex), // stride is Vertex bytes
            model.getVertexData() // pull from the start of the vertex data
    );
    glEnableVertexAttribArray(position_);

    // Only setup UVs if the attribute exists in this shader
    if(uv_ != -1) {
        // The uv attribute is 2 floats
        glVertexAttribPointer(
                uv_, // attrib
                2, // elements
                GL_FLOAT, // of type float
                GL_FALSE, // don't normalize
                sizeof(Vertex), // stride is Vertex bytes
                ((uint8_t *) model.getVertexData()) +
                sizeof(Vector3) // offset Vector3 from the start
        );
        glEnableVertexAttribArray(uv_);


        // Setup the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.getTexture().getTextureID());
    }

    // Draw as indexed triangles
    glDrawElements(GL_TRIANGLES, model.getIndexCount(), GL_UNSIGNED_SHORT, model.getIndexData());
    if(uv_ != -1) {
        glDisableVertexAttribArray(uv_);
    }
    glDisableVertexAttribArray(position_);
}

void Shader::setProjectionMatrix(float *projectionMatrix) const {
    glUniformMatrix4fv(projectionMatrix_, 1, false, projectionMatrix);
}