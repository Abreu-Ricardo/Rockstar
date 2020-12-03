// Compile command:
// Linux
// g++ --std=c++11 hello.cpp -o main -lGLEW -lGL -lGLU -lglut
// Windows
// g++ --std=c++11 hello.cpp -o main -lglew32 -lfreeglut -lglu32 -lopengl32

#include "hello.h"
#include "obj.h"






#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
using namespace std;
#include <string.h>
#define MAX 128

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtx/transform2.hpp>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/freeglut.h>  // ou glut.h - GLUT, include glu.h and gl.h
#include <GL/gl.h>



int final=1;    //variavel que ajuda terminar o programa
std::vector<Obj*> objVector; //vetor com todos os objetos a serem renderizados
glm::vec3 vetorLuz; //vetor com todos as luzes a serem carregadas


unsigned int vboCounter=1; //Contador de vbo's
unsigned int vaoCounter=1; //Contador de vao's
unsigned int cont=0;       //contador de objetos
unsigned int wire=0;       // Variavel para ativarmos wire_on e wire_off



glm::vec3 ambientLight;
glm::vec3 diffuseLight;
glm::vec3 specularLight;


//int tipoIluminacao;      //define o tipo de iluminação a ser usada,0-Nenhuma 1-Ambiente 2-Difusa 3-Especular
//GLint ambientId;
//GLint difusaId;
//GLint specularId;

GLint diffuse = 0;
GLint ambient = 0;
GLint specular = 0;
GLint lightsID;





unsigned int vboAxis;
unsigned int vaoAxis;










//passar os dados do .obj para vetor 
void loadObj(const char* filename, vector<glm::vec3> &dadosDoObjeto) {
	vector<unsigned int> vertexIndices, normalIndices;
    vector<glm::vec3> vertices; // vetor de vetice
    vector<glm::vec3> normals; // vetor de normais

	//lendo o arquivo 
    FILE *arquivo = fopen(filename, "r");
	
	if (arquivo == NULL){
        printf("Arquivo vazio!\n");
        exit(1);
    }
	char primeiroDaline[MAX];
    int res = fscanf(arquivo, "%s", primeiroDaline);     //read the first word
    while (res != EOF){

        if( strcmp(primeiroDaline, "v") == 0 ){       //verify is it's a vertex position
            glm::vec3 vertex;
            fscanf(arquivo, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);    //copy the coordinates to auxiliar vertex
            vertices.push_back(vertex);                                //copy the coordinates to the temporary vecVertex

        }
        else if (strcmp(primeiroDaline, "vn") == 0){  //verify if it's a normal data
            glm::vec3 normal;
            fscanf(arquivo, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
        }
        else if (strcmp(primeiroDaline, "f") == 0){   //verify if it's a face, and make a index contents
            unsigned int vertexIndex[3], normalIndex[3];
            int matches = fscanf(arquivo, "%d//%d  %d//%d  %d//%d\n",  &vertexIndex[0], &normalIndex[0],
                                                                       &vertexIndex[1], &normalIndex[1],
                                                                       &vertexIndex[2], &normalIndex[2]);
            if (matches != 6){
                printf("Error em ler arquivo! tente outro.\n\n");
                exit(1);
            }
            //indexação dos vertices e normais
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);   
        }
        res = fscanf(arquivo, "%s", primeiroDaline); //continue reading
    }
    fclose(arquivo);
	//passing by each vertex (v/vn) of each triangule (face)
    //put the right vertex positions and normals on the out_vertexData        
	for(unsigned int i = 0; i < vertexIndices.size(); i++){
            
        unsigned int indexV = vertexIndices[i];
        unsigned int indexN = normalIndices[i];
            
        glm::vec3 vertex = vertices[indexV-1];
        glm::vec3 normal = normals[indexN-1];
        dadosDoObjeto.push_back(vertex);
        dadosDoObjeto.push_back(normal);
        
    }
}


namespace {
OpenGLContext* currentInstance = nullptr;
}

OpenGLContext::OpenGLContext(int argc, char *argv[]) {
	glutInit(&argc, argv);     // Initialize GLUT

	glutInitContextVersion(3, 0);               // IMPORTANT!
	glutInitContextProfile(GLUT_CORE_PROFILE); // IMPORTANT! or later versions, core was introduced only with 3.2
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // IMPORTANT! Double buffering!

	glutInitWindowSize(640, 640);
	glutCreateWindow("Trabalho Pratico");      // Create window with the given title 

	glutDisplayFunc(OpenGLContext::glutRenderCallback); // Register callback handler for window re-paint event
	glutReshapeFunc(OpenGLContext::glutReshapeCallback); // Register callback handler for window re-size event

	GLenum error = glewInit();

	if (error != GLEW_OK) {
		throw std::runtime_error("Error initializing GLEW.");
	}

	currentInstance = this;

	//this->initialize();
}

OpenGLContext::~OpenGLContext() {
	this->finalize();
}





//echnique where we draw many (equal mesh data) objects at once with a single rende

void OpenGLContext::initialize() {


	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black and opaque


	//Shader padrão
	GLint vertexShaderId = this->loadAndCompileShader("shader/none.vp",
	GL_VERTEX_SHADER);
	GLint fragmentShaderId = this->loadAndCompileShader("shader/none.fp",
	GL_FRAGMENT_SHADER);
	this->programId = this->linkShaderProgram(vertexShaderId, fragmentShaderId);


	

	//fazemos a leitura inicial dos comandos 
	char comando[20];

	scanf("%s", comando);


	//responsável por adicionar todas as formas desejadas
	if(strcmp(comando,"add_shape")==0){
		char forma[20];
		scanf("%s", forma);


		if(strcmp(forma,"cube")==0){

			char Objnome[20];
			scanf("%s", Objnome);   //nome a ser setado no objeto

			
			std::vector<glm::vec3> vertexAux;  //vetor auxiliar onde vamos carregar os dados do obj

			loadObj("cubo.obj", vertexAux);   //carregamento dos dados

			glm::mat4 matAux;    //matriz auxiliar para conseguirmos criar o nosso objeto


			glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);

			matAux=view*projection;




			Obj* objAux = new Obj(Objnome,vboCounter,vaoCounter,vertexAux,matAux);  //instanciação do objeto
			
			objAux->cam[0]=0.0f;
			objAux->cam[1]=0.0f;
			objAux->cam[2]=0.0f;

			objAux->lookat[0]=0.0f;
			objAux->lookat[1]=0.0f;
			objAux->lookat[2]=-1.0f;
			
			
			
			glm::vec3 corInicial = glm::vec3(1.0f,1.0f,1.0f);
			objAux->vecCor=corInicial;
			
			objVector.push_back(objAux); //atribuição do objeto no vetor de objetos
			vboCounter++;         //aumentamos essa variavel global para futuras atribuições de id para vbo
			vaoCounter++;		//aumentamos essa variavel global para futuras atribuições de id para vao

		}

		if(strcmp(forma,"sphere")==0){

			char Objnome[20];
			scanf("%s", Objnome);

			
			std::vector<glm::vec3> vertexAux;

			loadObj("bola.obj", vertexAux);

			glm::mat4 matAux;

			glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);

			matAux=view*projection;




			Obj* objAux = new Obj(Objnome,vboCounter,vaoCounter,vertexAux,matAux);
			
			objAux->cam[0]=0.0f;
			objAux->cam[1]=0.0f;
			objAux->cam[2]=0.0f;

			objAux->lookat[0]=0.0f;
			objAux->lookat[1]=0.0f;
			objAux->lookat[2]=-1.0f;
			
			
			
			
			glm::vec3 corInicial = glm::vec3(1.0f,1.0f,1.0f);
			objAux->vecCor=corInicial;
			objVector.push_back(objAux);
			vboCounter++;
			vaoCounter++;	

		}


		if(strcmp(forma,"torus")==0){

			char Objnome[20];
			scanf("%s", Objnome);

			
			std::vector<glm::vec3> vertexAux;

			loadObj("torus.obj", vertexAux);

			glm::mat4 matAux;

			glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);

			matAux=view*projection;


			Obj* objAux = new Obj(Objnome,vboCounter,vaoCounter,vertexAux,matAux);
			
			objAux->cam[0]=0.0f;
			objAux->cam[1]=0.0f;
			objAux->cam[2]=0.0f;

			objAux->lookat[0]=0.0f;
			objAux->lookat[1]=0.0f;
			objAux->lookat[2]=-1.0f;



			glm::vec3 corInicial = glm::vec3(1.0f,1.0f,1.0f);
			objAux->vecCor=corInicial;

			objVector.push_back(objAux);
			vboCounter++;
			vaoCounter++;	

		}

		if(strcmp(forma,"cone")==0){

			char Objnome[20];
			scanf("%s", Objnome);

			
			std::vector<glm::vec3> vertexAux;

			loadObj("cone.obj", vertexAux);

			glm::mat4 matAux;

			glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);

			matAux=view*projection;


			Obj* objAux = new Obj(Objnome,vboCounter,vaoCounter,vertexAux,matAux);
		
			objAux->cam[0]=0.0f;
			objAux->cam[1]=0.0f;
			objAux->cam[2]=0.0f;

			objAux->lookat[0]=0.0f;
			objAux->lookat[1]=0.0f;
			objAux->lookat[2]=-1.0f;


			glm::vec3 corInicial = glm::vec3(1.0f,1.0f,1.0f);
			objAux->vecCor=corInicial;
			
			objVector.push_back(objAux);
			vboCounter++;
			vaoCounter++;	

		}


	}
	
	if(strcmp(comando,"remove_shape")==0){
		
		
		char Objnome[20];
		scanf("%s", Objnome);

		for(int i=0;i<objVector.size();i++){

			if(strcmp(Objnome,objVector[i]->nome)==0){
				objVector.erase(objVector.begin()+i);
			}

		}



	}

	if(strcmp(comando,"quit")==0){
		final=0;
	}


	if(strcmp(comando,"translate")==0){
			float p1,p2,p3;
			char Objnome[20];

			scanf("%s", Objnome);

			scanf("%f %f %f",&p1,&p2,&p3);

			for(int i=0;i<objVector.size();i++){

			if(strcmp(Objnome,objVector[i]->nome)==0){
				glm::mat4 model = glm::translate( glm::mat4(1.0f), glm::vec3(p1,p2,p3) );
				objVector[i]->fullTransformationMatrix = objVector[i]->fullTransformationMatrix * model;

			}

		}


	}

	if(strcmp(comando,"rotate")==0){
			float p1,p2,p3,p4;
			char Objnome[20];

			scanf("%s", Objnome);

			scanf("%f %f %f %f",&p1,&p2,&p3,&p4);

			for(int i=0;i<objVector.size();i++){

			if(strcmp(Objnome,objVector[i]->nome)==0){
				glm::mat4 model = glm::rotate( glm::mat4(1.0f), glm::radians(p1) , glm::vec3(p2,p3,p4) );
				objVector[i]->fullTransformationMatrix = objVector[i]->fullTransformationMatrix * model;

			}

		}


	}
	
	if(strcmp(comando,"scale")==0){
			float p1,p2,p3;
			char Objnome[20];

			scanf("%s", Objnome);

			scanf("%f %f %f",&p1,&p2,&p3);

			for(int i=0;i<objVector.size();i++){

				if(strcmp(Objnome,objVector[i]->nome)==0){
					glm::mat4 model = glm::scale( objVector[i]->fullTransformationMatrix , glm::vec3(p1,p2,p3) );
					objVector[i]->fullTransformationMatrix = objVector[i]->fullTransformationMatrix * model;

				}

			}


	}


	if(strcmp(comando,"shear")==0){
		float p1,p2,p3,p4,p5,p6;
		char Objnome[20];

		scanf("%s", Objnome);

		scanf("%f %f %f %f %f %f",&p1,&p2,&p3,&p4,&p5,&p6);

		for(int i=0;i<objVector.size();i++){

			if(strcmp(Objnome,objVector[i]->nome)==0){
				glm::mat4 model1 = glm::shearX3D( glm::mat4(1.0f),  p1,p2);
				glm::mat4 model2 = glm::shearY3D( glm::mat4(1.0f),  p3,p4);
				glm::mat4 model3 = glm::shearZ3D( glm::mat4(1.0f),  p5,p6);
				


				objVector[i]->fullTransformationMatrix = objVector[i]->fullTransformationMatrix * model1 * model2* model3 ;

			}
		}
	}




	

	if(strcmp(comando,"lookat")==0){
		float p1,p2,p3;
		scanf("%f %f %f",&p1,&p2,&p3);

		for(int i=0;i<objVector.size();i++){
	
				glm::mat4 model = glm::lookAt(glm::vec3(objVector[i]->cam[0], objVector[i]->cam[1]=p2, objVector[i]->cam[2]=p3),
				glm::vec3(p1, p2, p3), glm::vec3(0.0f, 1.0f, 0.0f));
				
				objVector[i]->fullTransformationMatrix = objVector[i]->fullTransformationMatrix * model;

				objVector[i]->lookat[0]=p1;
				objVector[i]->lookat[1]=p2;
				objVector[i]->lookat[2]=p3;
			
		}
	}

	if(strcmp(comando,"cam")==0){
		float p1,p2,p3;
	

		scanf("%f %f %f",&p1,&p2,&p3);

		for(int i=0;i<objVector.size();i++){
	
				glm::mat4 model = glm::lookAt(glm::vec3(p1, p2, p3),
				glm::vec3(objVector[i]->lookat[0], objVector[i]->lookat[1], objVector[i]->lookat[2]), glm::vec3(0.0f, 1.0f, 0.0f));

				objVector[i]->fullTransformationMatrix = objVector[i]->fullTransformationMatrix * model;

				
				objVector[i]->cam[0]=p1;
				objVector[i]->cam[1]=p2;
				objVector[i]->cam[2]=p3;

			
		}
	}



	if(strcmp(comando,"color")==0){
		float p1,p2,p3;
		char Objnome[20];

		scanf("%s", Objnome);

		scanf("%f %f %f",&p1,&p2,&p3);

		for(int i=0;i<objVector.size();i++){

			if(strcmp(Objnome,objVector[i]->nome)==0){
				   glm::vec3 corAux(p1,p2,p3);
				   objVector[i]->vecCor=corAux;

			}

		}


	}
	
	if(strcmp(comando,"wire_on")==0){
		wire=1;
	}


	if(strcmp(comando,"wire_off")==0){
		wire=0;
	}



    if(strcmp(comando,"axis_on")==0){


		//esse trecho de codigo gera 3 objetos que vão ser os eixos x,y,z seta os seus vertices e cores e os eixos são renderizados
		//junto com os objetos da cena.

		//Ajusta eixo X
        std::vector<glm::vec3> eixoX;

        eixoX.push_back(glm::vec3(-2.0,0.0,0.0));
        eixoX.push_back(glm::vec3(1.0,1.0,1.0));
        eixoX.push_back(glm::vec3(2.0,0.0,0.0));
        eixoX.push_back(glm::vec3(1.0,1.0,1.0));

		char Objnome[20] = "eixoX";

        glm::mat4 matAux;
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f);

        matAux=view*projection;

        Obj* objAux = new Obj(Objnome,vboCounter,vaoCounter,eixoX,matAux);

        objAux->cam[0]=0.0f;
        objAux->cam[1]=0.0f;
        objAux->cam[2]=0.0f;

        objAux->lookat[0]=0.0f;
        objAux->lookat[1]=0.0f;
        objAux->lookat[2]=-1.0f;


        glm::vec3 corInicial = glm::vec3(1.0f,0.0f,0.0f);
        objAux->vecCor=corInicial;

        objVector.push_back(objAux);
				
		vboCounter++;
        vaoCounter++;


		//Ajusta eixo Y
		std::vector<glm::vec3> eixoY;

		eixoY.push_back(glm::vec3(0.0,-2.0,0.0));
        eixoY.push_back(glm::vec3(1.0,1.0,1.0));
        eixoY.push_back(glm::vec3(0.0,2.0,0.0));
        eixoY.push_back(glm::vec3(1.0,1.0,1.0));

		char Objnome2[20] = "eixoY";

        glm::mat4 matAux2;
        glm::mat4 view2 = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection2 = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f);

        matAux2=view2*projection2;

        Obj* objAux2 = new Obj(Objnome2,vboCounter,vaoCounter,eixoY,matAux2);

        objAux2->cam[0]=0.0f;
        objAux2->cam[1]=0.0f;
        objAux2->cam[2]=0.0f;

        objAux2->lookat[0]=0.0f;
        objAux2->lookat[1]=0.0f;
        objAux2->lookat[2]=-1.0f;


        glm::vec3 corInicial2 = glm::vec3(0.0f,1.0f,0.0f);
        objAux2->vecCor=corInicial2;

        objVector.push_back(objAux2);
					
		vboCounter++;
        vaoCounter++;

		//Ajustar o eixo Z
	

		std::vector<glm::vec3> eixoZ;

		eixoZ.push_back(glm::vec3(0.0,0.0,-2.0));
        eixoZ.push_back(glm::vec3(1.0,1.0,1.0));
        eixoZ.push_back(glm::vec3(0.0,0.0,2.0));
        eixoZ.push_back(glm::vec3(1.0,1.0,1.0));

		char Objnome3[20] = "eixoZ";

        glm::mat4 matAux3;
        glm::mat4 view3 = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection3 = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f);

        matAux3=view3*projection3;

        Obj* objAux3 = new Obj(Objnome3,vboCounter,vaoCounter,eixoZ,matAux3);

        objAux3->cam[0]=0.0f;
        objAux3->cam[1]=0.0f;
        objAux3->cam[2]=0.0f;

        objAux3->lookat[0]=0.0f;
        objAux3->lookat[1]=0.0f;
        objAux3->lookat[2]=-1.0f;


        glm::vec3 corInicial3 = glm::vec3(0.0f,0.0f,1.0f);
        objAux3->vecCor=corInicial3;

        objVector.push_back(objAux3);
					
		vboCounter++;
        vaoCounter++;



    }




	if(strcmp(comando,"axis_off")==0){
	
		for(int i=0;i<objVector.size();i++){
		
			if(strcmp(objVector[i]->nome,"eixoZ")==0 ||  strcmp(objVector[i]->nome,"eixoY")==0 || strcmp(objVector[i]->nome,"eixoX")==0 ){
				objVector.erase(objVector.begin()+i);
				i--;         //como o erase diminui o tamanho do vetor, precisamos fazer um ajuste no indice.	                               
			}
		}


	}



	if(strcmp(comando,"reflection_on")==0){
		char tipo[20];
		scanf("%s", tipo);

		
		GLint LightsvpID = this->loadAndCompileShader("shader/lights.vp",
		GL_VERTEX_SHADER);
		GLint LightsfpID = this->loadAndCompileShader("shader/lights.fp",
		GL_FRAGMENT_SHADER);
		lightsID = this->linkShaderProgram(LightsvpID, LightsfpID);
		

		
		if(strcmp(tipo,"ambient")==0){
			float indice;
			scanf("%f", &indice);

			ambientLight = glm::vec3(indice,indice,indice);
			ambient=1;


		}
		
		if(strcmp(tipo,"diffuse")==0){
			float indice;
			scanf("%f", &indice);

			diffuseLight = glm::vec3(indice,indice,indice);
			diffuse=1;


		}


		if(strcmp(tipo,"specular")==0){
			float indice;
			scanf("%f", &indice);

			specularLight = glm::vec3(indice,indice,indice);
			specular=1;

		}





	}








	if(strcmp(comando,"reflection_off")==0){
		char tipo[20];
		scanf("%s", tipo);

		if(strcmp(tipo,"ambient")==0){
			ambient=0;
		}

		if(strcmp(tipo,"diffuse")==0){
			diffuse=0;
		}
		if(strcmp(tipo,"specular")==0){
			specular=0;
		}



	}



	if(strcmp(comando,"add_light")==0){
	
		char nome[20];
		float p1,p2,p3;

		scanf("%s", nome);
		scanf("%f %f %f", &p1,&p2,&p3);
			
		std::vector<glm::vec3> vertexAux;
		

		vertexAux.push_back(glm::vec3(p1,p2,p3));
		//vertexAux.push_back(glm::vec3(1.0,1.0,1.0));


		glm::mat4 matAux;

		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f);

		matAux=view*projection;


		Obj* objAux = new Obj(nome,vboCounter,vaoCounter,vertexAux,matAux);
	
		objAux->cam[0]=0.0f;
		objAux->cam[1]=0.0f;
		objAux->cam[2]=0.0f;

		objAux->lookat[0]=0.0f;
		objAux->lookat[1]=0.0f;
		objAux->lookat[2]=-1.0f;
		
		vetorLuz = glm::vec3(p1,p2,p3);




		glm::vec3 corInicial = glm::vec3(1.0f,0.5f,0.0f);
		objAux->vecCor=corInicial;
		objAux->isLight=true;
		objVector.push_back(objAux);
		vboCounter++;
		vaoCounter++;
		

	}







	//Esse trecho do código Seta todos os vbo's e vao's de todos os objetos.
    for(int i=0;i<objVector.size();i++){
			
		glGenVertexArrays(1, static_cast<GLuint*>(&objVector[i]->vao));
		glBindVertexArray(static_cast<GLuint>(objVector[i]->vao));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glGenBuffers(1, static_cast<GLuint*>(&objVector[i]->vbo));	
		glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(objVector[i]->vbo)); 
		glBufferData(GL_ARRAY_BUFFER, objVector[i]->buffer.size()*sizeof(glm::vec3),objVector[i]->buffer.data(), GL_STATIC_DRAW); 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) 0);
		glBindAttribLocation(this->programId, 0, "vertexPosition");
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) (sizeof(GLfloat) * 3));
		glBindAttribLocation(this->programId, 1, "vertexNormal");	




		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		vboCounter++;
		vaoCounter++;
	}


/*
	std::vector<glm::vec3> vertexAux2;
	char nomeObj2[20]="cube2";

	loadObj("cubo2.obj", vertexAux2);

	glm::mat4 matAux2;
	Obj* objAux2 = new Obj(nomeObj,vboCounter,vaoCounter,vertexAux2,matAux2);
	objVector.push_back(objAux2);


			
	glGenVertexArrays(1, static_cast<GLuint*>(&objVector[1]->vao));
	glBindVertexArray(static_cast<GLuint>(objVector[1]->vao));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, static_cast<GLuint*>(&objVector[1]->vbo));	
	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(objVector[1]->vbo)); 
	glBufferData(GL_ARRAY_BUFFER, objVector[1]->buffer.size()*sizeof(glm::vec3),objVector[1]->buffer.data(), GL_STATIC_DRAW); 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) 0);
	glBindAttribLocation(this->programId, 0, "vertexPosition");
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) (sizeof(GLfloat) * 3));
	glBindAttribLocation(this->programId, 1, "vertexNormal");	



	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
*/



	

}

void OpenGLContext::glutReshapeCallback(int width, int height) {
	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);
}

void OpenGLContext::glutRenderCallback() {
	currentInstance->render();
}




void OpenGLContext::render() const {
	
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	for(int i=0;i<objVector.size();i++){
		
		
			glBindVertexArray(objVector[i]->vao);
			glUseProgram(this->programId);


							
			GLint fullTransformationMatrixLocation = glGetUniformLocation(this->programId, "fullTransformationMatrix");
			glUniformMatrix4fv(fullTransformationMatrixLocation,1,GL_FALSE,&objVector[i]->fullTransformationMatrix[0][0]);

			GLint corLoc = glGetUniformLocation(this->programId, "vertexColor");
			glUniform3fv(corLoc, 1, &objVector[i]->vecCor[0]);
			
			if(diffuse==1 || specular==1 || ambient==1){ //Nenhuma
				glUseProgram(0);
				glUseProgram(lightsID);
				
				
				GLint fullTransformationMatrixLocation = glGetUniformLocation(lightsID, "fullTransformationMatrix");
				glUniformMatrix4fv(fullTransformationMatrixLocation,1,GL_FALSE,&objVector[i]->fullTransformationMatrix[0][0]);

				GLint corLoc = glGetUniformLocation(lightsID, "vertexColor");
				glUniform3fv(corLoc, 1, &objVector[i]->vecCor[0]);


				
				GLint diffuseLightUniformLocation = glGetUniformLocation(lightsID,"lightPosition");
				glUniform3fv(diffuseLightUniformLocation,1,&vetorLuz[0]);
				
				GLint eyeLightUniformLocation = glGetUniformLocation(lightsID,"eyePositionWorld");
				glUniform3fv(eyeLightUniformLocation,1,&objVector[i]->cam[0]);
				

				GLint specularIntensityLightUniformLocation = glGetUniformLocation(lightsID,"specularIntensity");
				glUniform3fv(specularIntensityLightUniformLocation,1,&specularLight[0]);


				GLint diffuseLocation = glGetUniformLocation(lightsID,"diffuse");
				glUniform1i(diffuseLocation,diffuse);

				GLint ambientLocation = glGetUniformLocation(lightsID,"ambient");
				glUniform1i(ambientLocation,ambient);
				
				GLint specularLocation = glGetUniformLocation(lightsID,"specular");
				glUniform1i(specularLocation,specular);




			}			
			
			
			glBindBuffer(GL_ARRAY_BUFFER, objVector[i]->vbo);






			if(wire==1){

				if(strcmp(objVector[i]->nome,"eixoX")==0 || strcmp(objVector[i]->nome,"eixoY")==0 || strcmp(objVector[i]->nome,"eixoZ")==0){
					glDrawArrays(GL_LINES , 0, objVector[i]->buffer.size());
				}else if(objVector[i]->isLight==true){			        
					glDrawArrays(GL_POINT , 0, objVector[i]->buffer.size());
				}else{
					glDrawArrays(GL_LINE_STRIP , 0, objVector[i]->buffer.size());	
				}



			}else{
				
				if(strcmp(objVector[i]->nome,"eixoX")==0 || strcmp(objVector[i]->nome,"eixoY")==0 || strcmp(objVector[i]->nome,"eixoZ")==0){
					glDrawArrays(GL_LINES , 0, objVector[i]->buffer.size());
				}else if(objVector[i]->isLight==true){			
					glDrawArrays(GL_POINTS , 0, objVector[i]->buffer.size());	
				}else{
					glDrawArrays(GL_TRIANGLES , 0, objVector[i]->buffer.size());
				}

			}
		
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		

	}










	glutSwapBuffers();


}

void OpenGLContext::printVersion() const {
	std::string glRender = reinterpret_cast<char const*>(glGetString(
	GL_RENDERER));
	std::string glVersion = reinterpret_cast<char const*>(glGetString(
	GL_VERSION));
	std::string glslVersion = reinterpret_cast<char const*>(glGetString(
	GL_SHADING_LANGUAGE_VERSION));

	std::cout << "OpenGL Renderer  : " << glRender << '\n'
			<< "OpenGL Version   : " << glVersion << '\n'
			<< "OpenGLSL Version : " << glslVersion << std::endl;
}

void OpenGLContext::runLoop() const {
	this->render();
	//this->render();
	
	glutMainLoop();
}

void OpenGLContext::finalize() const {
	// Properly de-allocate all resources once they've outlived their purpose
	glDisableVertexAttribArray(0);
	glDeleteVertexArrays(1, &(this->VAOId));
	glDeleteBuffers(1, &(this->VBOId));
	glUseProgram(0);
}

unsigned int OpenGLContext::loadAndCompileShader(const std::string &filename,
		const int glType) const {
	std::string shaderCode;

	// Read the Vertex Shader code from the file
	{
		// Open file stream
		std::ifstream shaderStream(filename, std::ios::in);

		if (shaderStream.good() == false) {
			throw std::runtime_error(
					"Error when opening " + filename
							+ ". Are you in the right directory ?");
		}

		// Copy file contents to string
		shaderCode.assign((std::istreambuf_iterator<char>(shaderStream)),
				(std::istreambuf_iterator<char>()));

	} // End of scope, RAII shaderStream closed.

	// Some temporaries for using in the code ahead...
	char const * sourcePointer = shaderCode.c_str(); // C style pointer to string
	GLint result = GL_FALSE;               // Return result for OGL API calls
	int infoLogLength = 0;                   // Length of log (in case of error)
	unsigned int shaderId = glCreateShader(glType); // Create a shader on GPU

	// Compile Shader
	//std::cout << "Compiling shader : " << filename << std::endl;

	glShaderSource(shaderId, 1, &sourcePointer, nullptr);
	glCompileShader(shaderId);

	// Check Shader
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

	// In case of error...
	if (result == GL_FALSE) {
		// Allocate vector of char to store error message
		std::vector<char> errorMessage(infoLogLength + 1);

		// Copy error message to string
		glGetShaderInfoLog(shaderId, infoLogLength, nullptr, &errorMessage[0]);

		// Print error message
		std::cerr << "Error compiling " << filename << '\n' << "  "
				<< std::string(errorMessage.begin(), errorMessage.end())
				<< std::endl;
	}

	return shaderId;
}

unsigned int OpenGLContext::linkShaderProgram(unsigned int vertexShaderId,
		unsigned int fragmentShaderId) const {
	// Create shader program on GPU
	unsigned int shaderProgramId = glCreateProgram();

	// Attach shader to be linked
	glAttachShader(shaderProgramId, vertexShaderId);
	glAttachShader(shaderProgramId, fragmentShaderId);

	// Link the program!
	glLinkProgram(shaderProgramId);

	// Some temporaries for using in the code ahead...
	GLint result = GL_FALSE;
	int InfoLogLength = 0;

	// Check the program
	glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &result);
	glGetProgramiv(shaderProgramId, GL_INFO_LOG_LENGTH, &InfoLogLength);

	// In case of error...
	if (result == GL_FALSE) {
		// Allocate vector of char to store error message
		std::vector<char> errorMessage(InfoLogLength + 1);

		// Copy error message to string
		glGetProgramInfoLog(shaderProgramId, InfoLogLength, nullptr,
				&errorMessage[0]);

		// Print error message
		std::cerr << std::string(errorMessage.begin(), errorMessage.end())
				<< std::endl;
	}

	// Detach...
	glDetachShader(shaderProgramId, vertexShaderId);
	glDetachShader(shaderProgramId, fragmentShaderId);

	// ...  and delete the shaders source code, work done
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	return shaderProgramId;
}


int main(int argc, char *argv[]) {
	OpenGLContext context { argc, argv };



	context.printVersion();
	//context.runLoop();
	
	while(final==1){
		context.initialize();
		context.render();
	
	}

	
	return 0;
}
