/*
Práctica 6: Texturizado - EJERCICIO EN CLASE
Ejercicio 1: dado de OpenGL texturizado con logos de Star Wars
Ejercicio 2: dado texturizado importado desde el programa de modelado
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION


#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList; //solo recibe xyz
std::vector<MeshColor*> meshListColor; // recibe xyz rgb
std::vector<MeshModel*> meshListModel; // recibe xyz uv nx ny nz

std::vector<Shader> shaderList;

Camera camera;

Texture plainTexture;
Texture pisoTexture;
Texture dadoTexture;
Texture holocronTexture;

Model Kitt_M;
Model Llanta_M;
Model Dado_M;
Model Holocron_M;

Skybox skybox;

//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;


// Vertex Shader
static const char* vShader = "shaders/shader_texture.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_texture.frag";





void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
		0, 1, 2,
		0, 2, 3,
		4,5,6,
		4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,
	};


	
	MeshModel *obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel *obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel *obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(obj3);

	MeshModel* obj4 = new MeshModel();
	obj4->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(obj4);

}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

void CrearDado()
{
	//Dado de 6 caras con logos de Star Wars (textura optimizada: Textures/dado_starwars.png, 1024x512, celdas de 256x256)
	//Cada cara tiene sus 4 vértices propios para poder asignar coordenadas S,T distintas por cara.
	//Orden de vértices por cara: abajo-izq, abajo-der, arriba-der, arriba-izq (vistos desde afuera) -> la imagen no sale espejeada
	//Normales invertidas (convención del curso, igual que en Model.cpp)
	unsigned int cubo_indices[] = {
		0, 1, 2,
		2, 3, 0,  // front
		4, 5, 6,
		6, 7, 4,  // right
		8, 9, 10,
		10, 11, 8,  // back
		12, 13, 14,
		14, 15, 12,  // left
		16, 17, 18,
		18, 19, 16,  // top
		20, 21, 22,
		22, 23, 20,  // bottom
	};

	GLfloat cubo_vertices[] = {
		//x		y		z			S			T				NX		NY		NZ
		// front
		-0.50f,	-0.50f,	0.50f,		0.0029f,	0.5059f,		0.0f,	0.0f,	-1.0f,	//0
		0.50f,	-0.50f,	0.50f,		0.2471f,	0.5059f,		0.0f,	0.0f,	-1.0f,	//1
		0.50f,	0.50f,	0.50f,		0.2471f,	0.9941f,		0.0f,	0.0f,	-1.0f,	//2
		-0.50f,	0.50f,	0.50f,		0.0029f,	0.9941f,		0.0f,	0.0f,	-1.0f,	//3
		// right
		0.50f,	-0.50f,	0.50f,		0.2529f,	0.5059f,		-1.0f,	0.0f,	0.0f,	//4
		0.50f,	-0.50f,	-0.50f,		0.4971f,	0.5059f,		-1.0f,	0.0f,	0.0f,	//5
		0.50f,	0.50f,	-0.50f,		0.4971f,	0.9941f,		-1.0f,	0.0f,	0.0f,	//6
		0.50f,	0.50f,	0.50f,		0.2529f,	0.9941f,		-1.0f,	0.0f,	0.0f,	//7
		// back
		0.50f,	-0.50f,	-0.50f,		0.5029f,	0.5059f,		0.0f,	0.0f,	1.0f,	//8
		-0.50f,	-0.50f,	-0.50f,		0.7471f,	0.5059f,		0.0f,	0.0f,	1.0f,	//9
		-0.50f,	0.50f,	-0.50f,		0.7471f,	0.9941f,		0.0f,	0.0f,	1.0f,	//10
		0.50f,	0.50f,	-0.50f,		0.5029f,	0.9941f,		0.0f,	0.0f,	1.0f,	//11
		// left
		-0.50f,	-0.50f,	-0.50f,		0.7529f,	0.5059f,		1.0f,	0.0f,	0.0f,	//12
		-0.50f,	-0.50f,	0.50f,		0.9971f,	0.5059f,		1.0f,	0.0f,	0.0f,	//13
		-0.50f,	0.50f,	0.50f,		0.9971f,	0.9941f,		1.0f,	0.0f,	0.0f,	//14
		-0.50f,	0.50f,	-0.50f,		0.7529f,	0.9941f,		1.0f,	0.0f,	0.0f,	//15
		// top
		-0.50f,	0.50f,	0.50f,		0.0029f,	0.0059f,		0.0f,	-1.0f,	0.0f,	//16
		0.50f,	0.50f,	0.50f,		0.2471f,	0.0059f,		0.0f,	-1.0f,	0.0f,	//17
		0.50f,	0.50f,	-0.50f,		0.2471f,	0.4941f,		0.0f,	-1.0f,	0.0f,	//18
		-0.50f,	0.50f,	-0.50f,		0.0029f,	0.4941f,		0.0f,	-1.0f,	0.0f,	//19
		// bottom
		-0.50f,	-0.50f,	-0.50f,		0.2529f,	0.0059f,		0.0f,	1.0f,	0.0f,	//20
		0.50f,	-0.50f,	-0.50f,		0.4971f,	0.0059f,		0.0f,	1.0f,	0.0f,	//21
		0.50f,	-0.50f,	0.50f,		0.4971f,	0.4941f,		0.0f,	1.0f,	0.0f,	//22
		-0.50f,	-0.50f,	0.50f,		0.2529f,	0.4941f,		0.0f,	1.0f,	0.0f,	//23
	};
	//caras: front=Alianza Rebelde, right=Imperio, back=Orden Jedi, left=Imperio Sith, top=Mandalorian, bottom=Primera Orden

	MeshModel* dado = new MeshModel();
	dado->CreateMeshModel(cubo_vertices, cubo_indices, 192, 36);
	meshListModel.push_back(dado);

}


void CrearHolocron()
{
	unsigned int holocron_indices[] = {

	12, 1, 0,
	 12, 4, 15,
	 14, 9, 2,
	 2, 12, 15,
	 8,  3,  4,
	 15,  4,  3,
	 3, 6, 14,
	 4, 5,  8,
	 6, 7, 13,
	 9, 10, 11,
	 11, 2, 9,
	 10, 7, 5,
	 13, 14, 6,
	 2, 3, 14,
	 2, 15, 3,
	 11, 12, 2,
	 13, 9, 14,
	 8, 6, 3,
	 12, 0, 4,
	 12, 11, 1,
	 4, 0, 5,
	 6, 8, 7,
	 9, 13, 10,
	 5, 0, 1,
	 1, 11, 10,
	 10, 13, 7,
	 7, 8, 5,
	 5, 1, 10

	};
	//Ejercicio 1: reemplazar con sus dados de 6 caras texturizados, agregar normales
// average normals
	GLfloat holocron_vertices[] = {
		// front
		//x					y			z			S		T		NX		NY		NZ
		 -2.517274,		-2.579095,		0.002115,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//0
		-1.185055,		-2.530499,		-1.198261,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//1
		0.049649,		2.440128,		-2.417217,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//2
		0.051152,		2.399781,		2.545043,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//3
		-2.646786,		-0.139047,		2.493757,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//4
		-1.328995,		-2.578386,		1.297838,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//5
		2.593003,		-0.085222,		2.505755,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//6
		1.385028,		-2.549476,		1.308677,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//7
		-0.140696,		-2.601934,		2.558561,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//8
		2.534944,		-0.061146,		-2.580280,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//9
		1.344182,		-2.542571,		-1.294387,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//10
		0.081432,		-2.513204,		-2.426005,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//11
		-2.453980,		 -0.063388,		-2.477451,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//12
		2.632201,		-2.553892,		0.007226,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//13
		2.521206,		2.448241,		0.063388,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//14
		-2.451372,		2.421759,		0.077538,	0.0f, 0.0f,		0.0f,	0.0f,	0.0f,	//15
	};

	MeshModel* holocron = new MeshModel();
	holocron->CreateMeshModel(holocron_vertices, holocron_indices, 120, 84);
	meshListModel.push_back(holocron);

}



int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CrearDado();
	CrearHolocron();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	dadoTexture = Texture("Textures/dado_starwars.png"); //imagen optimizada con logos de Star Wars
	dadoTexture.LoadTextureA();
	holocronTexture = Texture("Textures/holocronunwrap.png");
	holocronTexture.LoadTextureA();
	
	Dado_M = Model();
	Dado_M.LoadModel("Models/dado_starwars.obj"); //cubo texturizado en el programa de modelado (usa Textures/dado_starwars.png)

	Holocron_M = Model();
	Holocron_M.LoadModel("Models/holocron_simple.obj");
	
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	
	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		color = glm::vec3(1.0f, 1.0f, 1.0f);//color blanco, multiplica a la información de color de la textura

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		meshListModel[2]->RenderMeshModel();


		

		//Dado de Opengl
		//Ejercicio 1: Texturizar su dado con la imagen ya optimizada por ustedes con logos de star wars
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-1.5f, 4.5f, -2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dadoTexture.UseTexture();
		meshListModel[4]->RenderMeshModel();
		
		//Ejercicio 2:Importar el cubo texturizado en el programa de modelado con 
		//la imagen ya optimizada por ustedes
		
		//Dado importado
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-3.0f, 3.0f, -2.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Dado_M.RenderModel();
	



		/*Reporte de práctica :
		
		Ejercicio 1: Crear o modificar el holocron y texturizarlo por medio de código
		Ejercicio 2: Importar el modelo del holocron texturizardo en el programa de modelado
		Ejercicio 3: Importar un modelo de avión con con la textura de la cara del personaje de la imagen del previo:
		Vidrio fonrtal: OJOS
		Frente del avión: Nariz y Sonrisa
		Alas: Logos del universo del personaje
		
		*/


		//Holocrones
		color = glm::vec3(0.0f, 1.0f, 0.0f);//color que multiplica a la información de color de la textura
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-8.5f, 4.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		holocronTexture.UseTexture();
		meshListModel[5]->RenderMeshModel();
		

		color = glm::vec3(1.0f, 1.0f, 1.0f);//color blanco, multiplica a la información de color de la textura
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-4.5f, 2.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Holocron_M.RenderModel();

		


	
		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
/*
//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		logofiTexture.UseTexture(); //textura con transparencia o traslucidez
		FIGURA A RENDERIZAR de OpenGL, si es modelo importado no se declara UseTexture
		glDisable(GL_BLEND);
*/