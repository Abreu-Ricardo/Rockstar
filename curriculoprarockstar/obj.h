#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
#include <vector>
#include <iostream>
#include <string.h>

using namespace std;


class Obj{

    public:
        char nome[20];
        unsigned int vbo;
        unsigned int vao;
        std::vector<glm::vec3> buffer;
        glm::mat4 fullTransformationMatrix; 
        
        glm::vec3 vecCor;
        float cam[2];
        float lookat[2];
        glm::vec3 lightPos; //Somente utilizado pra um objeto de luz 
        bool isLight;

    Obj(char nomeAux[20],unsigned int vboid,unsigned int vaoid, std::vector<glm::vec3> dados, glm::mat4 matriz){
        strcpy(nome,nomeAux);
        vbo=vboid;
        vao=vaoid;
        buffer=dados;
        fullTransformationMatrix=matriz;
        isLight=false;
    }



};