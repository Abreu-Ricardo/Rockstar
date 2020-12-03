#ifndef OPENGLCONTEXT_H
#define OPENGLCONTEXT_H

#include <string>
#include <vector>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>








using namespace std;
class OpenGLContext
{
public:
    OpenGLContext(int argc, char *argv[]);     // Constructor
    ~OpenGLContext();    // Destructor

    void printVersion() const; // Show OpenGL Version
    void runLoop() const;

//private:
    unsigned int VAOId;
    unsigned int VBOId;
    unsigned int programId;

    static void glutRenderCallback();                       // Render window
    static void glutReshapeCallback(int width, int height); // Reshape window
   
    

    void initialize();   // Init our render
    void render() const;       // Render Loop
    void finalize() const;     // Finalize our render

    unsigned int loadAndCompileShader(const std::string& filename, const int glType) const;
    unsigned int linkShaderProgram(unsigned int vertexShaderId, unsigned int fragmentShaderId) const;
};

#endif // OPENGLCONTEXT_H
