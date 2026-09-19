/*Practica 4: Modelado Jerarquico - ROVER  ***VERSION 2***
Contiene: CABINA + BASE + BRAZO CON PINZA + LAS 6 LLANTAS (cada una gira por
separado) + una SONDA ESPACIAL construida tambien de forma jerarquica.

JERARQUIA IMPLEMENTADA (arbol de la practica)
  ORIGEN
   +-- CABINA
        +-- BASE
             +-- EJE FRONTAL -- BARRA -- EJE B-PL -- PATA L -- EJE PL-LL -- LLANTA  (x2 lados)
             +-- EJE LL2     -- PATA L -- EJE PL-LL -- LLANTA                       (x2 lados)
             +-- EJE LL3     -- PATA L -- EJE PL-LL -- LLANTA                       (x2 lados)
             +-- EJE BRAZO   -- BASE BRAZO -- EJE BB-BR1 -- BRAZO 1
                              -- EJE BR1-BR2 -- BRAZO 2 -- EJE BR2-PINZA -- PINZA

  SONDA (jerarquia propia, independiente del rover)
   ORIGEN SONDA -- PLATAFORMA -- MASTIL -- CUERPO
                                    +-- EJE PANEL -- BRAZO PANEL -- PANEL SOLAR (x2)
                                    +-- EJE ANTENA -- MASTIL ANTENA -- EJE INCLINACION -- PLATO

CONTROLES
  W A S D + mouse : camara
  E / R / T       : rotar TODO el rover en X / Y / Z
  --- BRAZO (cada articulacion gira en los 3 ejes) ---
  1 / 2 / 3       : seleccionar articulacion (hombro / codo / muneca)
  F / G / H       : girar la articulacion seleccionada en X / Y / Z
  J               : girar la base del brazo (EJE BRAZO, sobre Y)
  C / V           : abrir / cerrar la pinza
  --- LLANTAS (cada una por separado) ---
  4 / 5 / 6       : seleccionar llanta del lado CERCA (delantera / media / trasera)
  7 / 8 / 9       : seleccionar llanta del lado LEJOS (delantera / media / trasera)
  K               : girar SOLO la llanta seleccionada
  L               : girar las 6 llantas al mismo tiempo
  --- SUSPENSION ---
  B               : EJE FRONTAL  (gira la barra delantera)
  N               : EJE B-PL     (gira la pata delantera)
  M               : EJE LL2      (patas de en medio)
  Y               : EJE LL3      (patas traseras)
  --- SONDA ESPACIAL ---
  O               : girar los paneles solares
  P               : girar la antena sobre Y
  I               : inclinar el plato de la antena sobre X
  SHIFT + tecla   : gira en sentido contrario
  ESC             : salir
*/
#include <stdio.h>
#include <string.h>
#include<cmath>
#include<vector>
#include <glew.h>
#include <glfw3.h>
//glm
#include<glm.hpp>
#include<gtc\matrix_transform.hpp>
#include<gtc\type_ptr.hpp>
#include <gtc\random.hpp>
//clases para dar orden y limpieza al codigo
#include"Mesh.h"
#include"Shader.h"
#include"Sphere.h"
#include"Window.h"
#include"Camera.h"

using std::vector;

//Dimensiones de la ventana
const float toRadians = 3.14159265f / 180.0f; //grados a radianes
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader>shaderList;
//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
Sphere sp = Sphere(1.0, 20, 20); //recibe radio, slices, stacks

/*=====================  MEDIDAS DEL ROVER  =====================
Todo se mide respecto al CENTRO DE LA BASE (la caja verde), que es el punto
que guardamos en modelaux2. La base mide 10 (X) x 1.5 (Y) x 8 (Z), entonces:
   frente del rover -> -X        atras -> +X
   lado "cerca"     -> +Z        lado "lejos" -> -Z
   tapa de la base  -> y = +0.75  (ahi se monta la cabina y el brazo)
   piso             -> y = -4.0
*/
const GLfloat RADIO_LLANTA = 1.5f;	//radio de las llantas
const GLfloat ANCHO_LLANTA = 1.2f;	//grosor de las llantas
const GLfloat Z_PATA = 4.0f;		//plano donde van las patas (costado de la base)
const GLfloat Z_LLANTA = 0.9f;		//la llanta va un poco mas afuera que la pata

//--- cadena de la llanta DELANTERA: EJE FRONTAL -> BARRA -> EJE B-PL -> PATA L -> LLANTA
const GLfloat EJE_FRONTAL_X = -4.0f;	//el eje frontal sale de la cara delantera de la base
const GLfloat EJE_Y = -0.4f;		//altura de todos los ejes de la suspension
/*La barra se dibuja a lo largo de su eje X local. Su largo y su angulo salen de
la distancia entre el EJE FRONTAL (-4.0,-0.4) y el EJE B-PL (-6.5,-1.3).*/
const GLfloat LARGO_BARRA = 2.657f;
const GLfloat ANG_BARRA = -160.2f;
const GLfloat LARGO_PATA_DEL = 1.2f;	//de la punta de la barra al centro de la llanta

//--- cadenas de las llantas MEDIA y TRASERA: EJE LLn -> PATA L -> EJE PL-LL -> LLANTA
const GLfloat EJE_LL2_X = -0.5f;	//eje de la llanta de en medio
const GLfloat EJE_LL3_X = 3.5f;		//eje de la llanta trasera
const GLfloat LARGO_PATA = 2.1f;	//largo de esas dos patas (del eje al centro de la llanta)

//--- SONDA ESPACIAL: se coloca a un lado del rover, parada sobre el mismo piso
const GLfloat SONDA_X = 20.0f, SONDA_Y = -0.75f, SONDA_Z = -4.0f;

//--- medidas del BRAZO
const GLfloat BRAZO_BASE_X = -3.5f, BRAZO_BASE_Y = 1.25f, BRAZO_BASE_Z = 2.5f;
const GLfloat LARGO_BRAZO1 = 4.0f;
const GLfloat LARGO_BRAZO2 = 3.5f;
//posicion de reposo del brazo (sobre Z) para que no arranque acostado
const GLfloat POSE_HOMBRO = 40.0f;
const GLfloat POSE_CODO = -80.0f;
const GLfloat POSE_MUNECA = -25.0f;

//--- colores
const glm::vec3 COLOR_CABINA = glm::vec3(0.55f, 0.55f, 0.58f);
const glm::vec3 COLOR_BASE = glm::vec3(0.20f, 0.55f, 0.30f);
const glm::vec3 COLOR_EJE = glm::vec3(0.10f, 0.30f, 0.95f);
const glm::vec3 COLOR_METAL = glm::vec3(0.75f, 0.75f, 0.78f);
const glm::vec3 COLOR_OSCURO = glm::vec3(0.15f, 0.15f, 0.17f);
const glm::vec3 COLOR_ROJO = glm::vec3(0.90f, 0.25f, 0.15f);
const glm::vec3 COLOR_AMARILLO = glm::vec3(0.90f, 0.70f, 0.10f);

void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};

	GLfloat cubo_vertices[] = {
		// front
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		// back
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo);
}

// Piramide triangular regular
void CrearPiramideTriangular()
{
	unsigned int indices_piramide_triangular[] = {
			0,1,2,
			1,3,2,
			3,0,2,
			1,0,3
	};
	GLfloat vertices_piramide_triangular[] = {
		-0.5f, -0.5f,0.0f,	//0
		0.5f,-0.5f,0.0f,	//1
		0.0f,0.5f, -0.25f,	//2
		0.0f,-0.5f,-0.5f,	//3
	};
	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
	meshList.push_back(obj1);
}

/*Crear cilindro y cono con arreglos dinamicos vector creados en el Semestre 2023 - 1
por Sanchez Perez Omar Alejandro*/
void CrearCilindro(int res, float R) {

	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	//ciclo for para crear los vertices de las paredes del cilindro
	for (n = 0; n <= (res); n++) {
		if (n != res) {
			x = R * cos((n)*dt);
			z = R * sin((n)*dt);
		}
		else {
			x = R * cos((0)*dt);
			z = R * sin((0)*dt);
		}
		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(y); break;
			case 2: vertices.push_back(z); break;
			case 3: vertices.push_back(x); break;
			case 4: vertices.push_back(0.5); break;
			case 5: vertices.push_back(z); break;
			}
		}
	}

	//ciclo for para crear la circunferencia inferior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(-0.5f); break;
			case 2: vertices.push_back(z); break;
			}
		}
	}

	//ciclo for para crear la circunferencia superior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(0.5); break;
			case 2: vertices.push_back(z); break;
			}
		}
	}

	for (i = 0; i < vertices.size(); i++) indices.push_back(i);

	Mesh *cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro);
}

//funcion para crear un cono
void CrearCono(int res, float R) {

	int n, i;
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	vertices.push_back(0.0);
	vertices.push_back(0.5);
	vertices.push_back(0.0);

	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0: vertices.push_back(x); break;
			case 1: vertices.push_back(y); break;
			case 2: vertices.push_back(z); break;
			}
		}
	}
	vertices.push_back(R * cos(0) * dt);
	vertices.push_back(-0.5);
	vertices.push_back(R * sin(0) * dt);

	for (i = 0; i < res + 2; i++) indices.push_back(i);

	Mesh *cono = new Mesh();
	cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
	meshList.push_back(cono);
}

//funcion para crear piramide cuadrangular unitaria
void CrearPiramideCuadrangular()
{
	vector<unsigned int> piramidecuadrangular_indices = {
		0,3,4,
		3,2,4,
		2,1,4,
		1,0,4,
		0,1,2,
		0,2,4
	};
	vector<GLfloat> piramidecuadrangular_vertices = {
		0.5f,-0.5f,0.5f,
		0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,0.5f,
		0.0f,0.5f,0.0f,
	};
	Mesh *piramide = new Mesh();
	piramide->CreateMeshGeometry(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramide);
}

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

/*===================  FUNCIONES DE APOYO PARA DIBUJAR  ===================
Se manda la matriz de modelo YA COLOCADA donde va la pieza y la funcion solo
manda los uniforms y dibuja. Asi el main se lee como el arbol jerarquico y no
como una lista interminable de glUniformMatrix4fv.*/
GLuint uModel = 0, uColor = 0;

void DibujarPrisma(glm::mat4 model, glm::vec3 color)
{
	glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uColor, 1, glm::value_ptr(color));
	meshList[0]->RenderMesh();			//indice 0: cubo
}

void DibujarCilindro(glm::mat4 model, glm::vec3 color)
{
	glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uColor, 1, glm::value_ptr(color));
	meshList[2]->RenderMeshGeometry();	//indice 2: cilindro
}

void DibujarCono(glm::mat4 model, glm::vec3 color)
{
	glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uColor, 1, glm::value_ptr(color));
	meshList[3]->RenderMeshGeometry();	//indice 3: cono (ojo: se creo con R = 2.0)
}

//las esferas se usan para marcar las articulaciones (los circulos azules del dibujo)
void DibujarEsfera(glm::mat4 model, glm::vec3 color)
{
	glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uColor, 1, glm::value_ptr(color));
	sp.render();
}

/*LLANTA (EJE PL-LL + LLANTA del arbol).
El cilindro se genera con su eje en Y, asi que se gira 90 grados sobre X para
acostarlo y que su eje quede sobre Z, que es el eje real de una llanta.
El parametro giro es el EJE PL-LL: el giro individual de esa llanta.*/
void DibujarLlanta(glm::mat4 model, GLfloat giro)
{
	model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); //acostar el cilindro
	model = glm::rotate(model, giro * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));  //EJE PL-LL
	glm::mat4 ejeLlanta = model;

	//neumatico
	model = glm::scale(model, glm::vec3(RADIO_LLANTA, ANCHO_LLANTA, RADIO_LLANTA));
	DibujarCilindro(model, COLOR_OSCURO);

	//rin: mas chico de radio y mas ancho para que se note el giro
	model = ejeLlanta;
	model = glm::scale(model, glm::vec3(RADIO_LLANTA * 0.6f, ANCHO_LLANTA * 1.15f, RADIO_LLANTA * 0.6f));
	DibujarCilindro(model, COLOR_METAL);

	//birlo central + una marca para ver hacia donde gira
	model = ejeLlanta;
	model = glm::scale(model, glm::vec3(0.25f, ANCHO_LLANTA * 1.3f, 0.25f));
	DibujarCilindro(model, COLOR_AMARILLO);

	model = ejeLlanta;
	model = glm::translate(model, glm::vec3(RADIO_LLANTA * 0.5f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(RADIO_LLANTA * 0.9f, ANCHO_LLANTA * 1.25f, 0.18f));
	DibujarPrisma(model, COLOR_ROJO);
}

int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	CrearCubo();					//indice 0 en MeshList
	CrearPiramideTriangular();		//indice 1 en MeshList
	CrearCilindro(18, 1.0f);		//indice 2 en MeshList
	CrearCono(25, 2.0f);			//indice 3 en MeshList
	CrearPiramideCuadrangular();	//indice 4 en MeshList
	CreateShaders();

	/*Camara: glm::lookAt(posicion, orientacion, up).
	Se coloco alejada y mirando hacia -Z para que el rover se vea completo al arrancar.*/
	camera = Camera(glm::vec3(0.0f, 4.0f, 22.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.3f, 0.3f);

	GLuint uniformProjection = 0;
	GLuint uniformView = 0;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 200.0f);

	sp.init(); //inicializar esfera
	sp.load(); //enviar la esfera al shader

	glm::mat4 model(1.0);		//matriz de modelo 4x4
	glm::mat4 modelaux(1.0);	//origen del rover
	glm::mat4 modelaux2(1.0);	//CENTRO DE LA BASE: el punto al que regresamos muchas veces
	glm::mat4 modelaux3(1.0);	//bifurcacion nivel 3 (ejes de la suspension / base del brazo)
	glm::mat4 modelaux4(1.0);	//bifurcacion nivel 4
	glm::mat4 modelaux5(1.0);	//bifurcacion nivel 5

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;
		//Recibir eventos del usuario
		glfwPollEvents();
		//Camara
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//Limpiar la ventana
		glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //se limpia tambien el buffer de profundidad
		shaderList[0].useShader();
		uModel = shaderList[0].getModelLocation();
		uColor = shaderList[0].getColorLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();

		//proyeccion y vista se mandan una sola vez por frame
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		/*==================================================================
		  ORIGEN DEL ROVER  (la X amarilla del dibujo)
		  Aqui se aplica la rotacion global: todo lo que cuelgue de este punto
		  se mueve junto, que es justamente la idea del modelado jerarquico.
		==================================================================*/
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, -4.0f)); //NOS POSICIONAMOS EN EL CENTRO DEL OBJETO
		model = glm::rotate(model, mainWindow.getrotax() * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, mainWindow.getrotay() * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, mainWindow.getrotaz() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		modelaux = model;

		/*=========================== CABINA ===============================*/
		model = glm::translate(model, glm::vec3(1.0f, 2.0f, 0.0f)); //del origen al centro de la cabina
		modelaux = model;	//se guarda ANTES de escalar: el escalado NO se hereda
		model = glm::scale(model, glm::vec3(8.0f, 4.0f, 6.0f));
		DibujarPrisma(model, COLOR_CABINA);

		/*============================ BASE ================================
		Se parte de modelaux (centro de la cabina) y se baja al centro de la
		base. Si aqui pusieramos model = glm::mat4(1.0) la base se desconectaria
		del rover y ya no heredaria la rotacion global.*/
		model = modelaux;
		model = glm::translate(model, glm::vec3(-1.0f, -2.75f, 0.0f)); //centro de la base
		modelaux2 = model;	//TODO lo de abajo cuelga de aqui: es la bifurcacion principal
		model = glm::scale(model, glm::vec3(10.0f, 1.5f, 8.0f));
		DibujarPrisma(model, COLOR_BASE);

		/*==================================================================
		  LAS 6 LLANTAS (3 de cada lado)
		  EJE FRONTAL -> BARRA -> EJE B-PL -> PATA L -> EJE PL-LL -> LLANTA
		  El ciclo corre dos veces: lado = +1 (cerca, +Z) y lado = -1 (lejos, -Z).
		  Es la misma jerarquia espejeada en Z, por eso conviene el ciclo en
		  lugar de copiar y pegar todo el bloque.
		==================================================================*/
		for (int lado = 1; lado >= -1; lado -= 2)
		{
			GLfloat z = Z_PATA * lado;			//plano donde va la suspension
			GLfloat zLl = Z_LLANTA * lado;		//la llanta va mas afuera que la pata
			/*indices de las 6 llantas: 0,1,2 del lado cerca y 3,4,5 del lado lejos.
			Cada una tiene su propio angulo, por eso se pueden girar por separado.*/
			int iLlanta = (lado == 1) ? 0 : 3;	//delantera
			int iMedia = (lado == 1) ? 1 : 4;	//de en medio
			int iTrasera = (lado == 1) ? 2 : 5;	//trasera

			//---------- EJE FRONTAL (tecla B) ----------
			model = modelaux2;
			model = glm::translate(model, glm::vec3(EJE_FRONTAL_X, EJE_Y, z));
			model = glm::rotate(model, mainWindow.getEjeFrontal() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			modelaux3 = model;	//este marco ya gira con el eje frontal

			model = modelaux3;
			model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
			DibujarEsfera(model, COLOR_EJE);

			//---------- BARRA ----------
			//se orienta con su angulo y se dibuja a lo largo de su eje X local
			model = modelaux3;
			model = glm::rotate(model, ANG_BARRA * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			modelaux4 = model;	//marco alineado con la barra, con el origen todavia en el eje
			model = glm::translate(model, glm::vec3(LARGO_BARRA / 2.0f, 0.0f, 0.0f)); //centro de la barra
			model = glm::scale(model, glm::vec3(LARGO_BARRA, 0.45f, 0.35f));
			DibujarPrisma(model, COLOR_METAL);

			//---------- EJE B-PL (tecla N) ----------
			model = modelaux4;
			model = glm::translate(model, glm::vec3(LARGO_BARRA, 0.0f, 0.0f)); //punta de la barra
			/*Se deshace el angulo de la barra para que la pata vuelva a quedar
			alineada con el rover; si no, la pata heredaria la inclinacion.*/
			model = glm::rotate(model, -ANG_BARRA * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::rotate(model, mainWindow.getEjeBarraPata() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			modelaux5 = model;

			model = modelaux5;
			model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
			DibujarEsfera(model, COLOR_EJE);

			//---------- PATA L ----------
			model = modelaux5;
			model = glm::translate(model, glm::vec3(0.0f, -LARGO_PATA_DEL / 2.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.4f, LARGO_PATA_DEL, 0.4f));
			DibujarPrisma(model, COLOR_METAL);

			//---------- EJE PL-LL + LLANTA ----------
			model = modelaux5;
			model = glm::translate(model, glm::vec3(0.0f, -LARGO_PATA_DEL, zLl));
			DibujarLlanta(model, mainWindow.getLlantaRot(iLlanta));


			/*---------- EJE LL2 (tecla M) -> PATA L -> EJE PL-LL -> LLANTA ----------
			Esta cadena es mas corta que la delantera: la pata cuelga directo del
			eje, sin barra de por medio, tal como aparece en el arbol.*/
			model = modelaux2;
			model = glm::translate(model, glm::vec3(EJE_LL2_X, EJE_Y, z));
			model = glm::rotate(model, mainWindow.getEjeLL2() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			modelaux3 = model;

			model = modelaux3;
			model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
			DibujarEsfera(model, COLOR_EJE);

			model = modelaux3;	//PATA L
			model = glm::translate(model, glm::vec3(0.0f, -LARGO_PATA / 2.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.45f, LARGO_PATA, 0.45f));
			DibujarPrisma(model, COLOR_METAL);

			model = modelaux3;	//EJE PL-LL + LLANTA
			model = glm::translate(model, glm::vec3(0.0f, -LARGO_PATA, zLl));
			DibujarLlanta(model, mainWindow.getLlantaRot(iMedia));

			//---------- EJE LL3 (tecla Y) -> PATA L -> EJE PL-LL -> LLANTA ----------
			model = modelaux2;
			model = glm::translate(model, glm::vec3(EJE_LL3_X, EJE_Y, z));
			model = glm::rotate(model, mainWindow.getEjeLL3() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			modelaux3 = model;

			model = modelaux3;
			model = glm::scale(model, glm::vec3(0.55f, 0.55f, 0.55f));
			DibujarEsfera(model, COLOR_EJE);

			model = modelaux3;	//PATA L
			model = glm::translate(model, glm::vec3(0.0f, -LARGO_PATA / 2.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.45f, LARGO_PATA, 0.45f));
			DibujarPrisma(model, COLOR_METAL);

			model = modelaux3;	//EJE PL-LL + LLANTA
			model = glm::translate(model, glm::vec3(0.0f, -LARGO_PATA, zLl));
			DibujarLlanta(model, mainWindow.getLlantaRot(iTrasera));

		} //fin del ciclo de los dos lados (cerca y lejos)

		/*==================================================================
		  BRAZO CON PINZA
		  EJE BRAZO -> BASE BRAZO -> EJE BB-BR1 -> BRAZO 1 -> EJE BR1-BR2 ->
		  BRAZO 2 -> EJE BR2-PINZA -> PINZA

		  Patron que se repite en cada eslabon:
		    1) se llega a la articulacion,
		    2) se gira (aqui en los 3 ejes: Y, Z y X),
		    3) se guarda esa matriz en una auxiliar,
		    4) se avanza MEDIO eslabon para dibujarlo,
		    5) se regresa a la auxiliar y se avanza el eslabon COMPLETO para
		       llegar a la siguiente articulacion.
		==================================================================*/
		model = modelaux2;
		model = glm::translate(model, glm::vec3(BRAZO_BASE_X, BRAZO_BASE_Y, BRAZO_BASE_Z));

		//---------- EJE BRAZO (tecla J): gira todo el brazo sobre Y ----------
		model = glm::rotate(model, mainWindow.getBaseBrazo() * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux3 = model;

		//---------- BASE BRAZO ----------
		model = modelaux3;
		DibujarPrisma(model, COLOR_ROJO);	//cubo unitario, no se escala

		//---------- EJE BB-BR1: el hombro (articulacion 1, teclas F G H) ----------
		model = modelaux3;
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f)); //tapa del cubo de la base
		model = glm::rotate(model, mainWindow.getBrazoRot(0, 1) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, (POSE_HOMBRO + mainWindow.getBrazoRot(0, 2)) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, mainWindow.getBrazoRot(0, 0) * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelaux4 = model;

		model = modelaux4;
		model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		DibujarEsfera(model, COLOR_EJE);

		//---------- BRAZO 1 ----------
		model = modelaux4;
		model = glm::translate(model, glm::vec3(0.0f, LARGO_BRAZO1 / 2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, LARGO_BRAZO1, 0.5f));
		DibujarCilindro(model, COLOR_METAL);

		//---------- EJE BR1-BR2: el codo (articulacion 2) ----------
		model = modelaux4;
		model = glm::translate(model, glm::vec3(0.0f, LARGO_BRAZO1, 0.0f)); //punta del brazo 1
		model = glm::rotate(model, mainWindow.getBrazoRot(1, 1) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, (POSE_CODO + mainWindow.getBrazoRot(1, 2)) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, mainWindow.getBrazoRot(1, 0) * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelaux5 = model;

		model = modelaux5;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		DibujarEsfera(model, COLOR_EJE);

		//---------- BRAZO 2 ----------
		model = modelaux5;
		model = glm::translate(model, glm::vec3(0.0f, LARGO_BRAZO2 / 2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.42f, LARGO_BRAZO2, 0.42f));
		DibujarCilindro(model, COLOR_METAL);

		//---------- EJE BR2-PINZA: la muneca (articulacion 3) ----------
		model = modelaux5;
		model = glm::translate(model, glm::vec3(0.0f, LARGO_BRAZO2, 0.0f));
		model = glm::rotate(model, mainWindow.getBrazoRot(2, 1) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, (POSE_MUNECA + mainWindow.getBrazoRot(2, 2)) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, mainWindow.getBrazoRot(2, 0) * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelaux4 = model;

		model = modelaux4;
		model = glm::scale(model, glm::vec3(0.42f, 0.42f, 0.42f));
		DibujarEsfera(model, COLOR_EJE);

		//---------- PINZA: palma + dos dedos que abren y cierran ----------
		model = modelaux4;
		model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.7f, 0.5f));
		DibujarPrisma(model, COLOR_OSCURO);

		model = modelaux4;
		model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f)); //base de los dedos
		modelaux5 = model;

		model = modelaux5;	//dedo 1
		model = glm::rotate(model, -mainWindow.getPinza() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.22f, 0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f, 1.0f, 0.4f));
		DibujarPrisma(model, COLOR_ROJO);

		model = modelaux5;	//dedo 2
		model = glm::rotate(model, mainWindow.getPinza() * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.22f, 0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f, 1.0f, 0.4f));
		DibujarPrisma(model, COLOR_ROJO);

		/*==================================================================
		  SONDA ESPACIAL  (jerarquia propia, aparte del rover)
		  ORIGEN -> PLATAFORMA -> MASTIL -> CUERPO
		                                     +-- EJE PANEL -> BRAZO -> PANEL (x2)
		                                     +-- EJE ANTENA -> MASTIL -> EJE INCL. -> PLATO
		  Se empieza otra vez con la identidad porque la sonda NO cuelga del
		  rover: es un objeto independiente con su propio origen.
		==================================================================*/
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(SONDA_X, SONDA_Y, SONDA_Z)); //origen de la sonda, sobre el piso
		modelaux = model;

		//---------- PLATAFORMA ----------
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 0.2f, 0.0f));
		model = glm::scale(model, glm::vec3(2.6f, 0.4f, 2.6f));
		DibujarCilindro(model, COLOR_OSCURO);

		//---------- MASTIL ----------
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 1.9f, 0.0f));
		model = glm::scale(model, glm::vec3(0.6f, 3.0f, 0.6f));
		DibujarCilindro(model, COLOR_METAL);

		//---------- CUERPO ----------
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, 4.6f, 0.0f)); //centro del cuerpo
		modelaux2 = model;	//de aqui cuelgan los paneles y la antena
		model = glm::scale(model, glm::vec3(3.0f, 2.4f, 3.0f));
		DibujarCilindro(model, COLOR_AMARILLO);

		//---------- PANELES SOLARES (tecla O) ----------
		//mismo ciclo de dos lados: uno hacia +X y otro hacia -X
		for (int lado = 1; lado >= -1; lado -= 2)
		{
			//EJE PANEL: el panel gira sobre X para "buscar el sol"
			model = modelaux2;
			model = glm::translate(model, glm::vec3(1.5f * lado, 0.0f, 0.0f));
			model = glm::rotate(model, mainWindow.getPanelSolar() * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			modelaux3 = model;

			model = modelaux3;
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			DibujarEsfera(model, COLOR_EJE);

			//BRAZO DEL PANEL
			model = modelaux3;
			model = glm::translate(model, glm::vec3(0.75f * lado, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.5f, 0.25f, 0.25f));
			DibujarPrisma(model, COLOR_METAL);

			//PANEL SOLAR
			model = modelaux3;
			model = glm::translate(model, glm::vec3(4.0f * lado, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(5.0f, 0.15f, 2.6f));
			DibujarPrisma(model, glm::vec3(0.10f, 0.15f, 0.55f));
		}

		//---------- EJE ANTENA (tecla P): gira toda la antena sobre Y ----------
		model = modelaux2;
		model = glm::translate(model, glm::vec3(0.0f, 1.2f, 0.0f)); //tapa del cuerpo
		model = glm::rotate(model, mainWindow.getAntenaY() * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		modelaux3 = model;

		model = modelaux3;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		DibujarEsfera(model, COLOR_EJE);

		//---------- MASTIL DE LA ANTENA ----------
		model = modelaux3;
		model = glm::translate(model, glm::vec3(0.0f, 0.9f, 0.0f));
		model = glm::scale(model, glm::vec3(0.35f, 1.8f, 0.35f));
		DibujarCilindro(model, COLOR_METAL);

		//---------- EJE DE INCLINACION (tecla I): inclina el plato sobre X ----------
		model = modelaux3;
		model = glm::translate(model, glm::vec3(0.0f, 1.8f, 0.0f));
		model = glm::rotate(model, mainWindow.getAntenaX() * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelaux4 = model;

		model = modelaux4;
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		DibujarEsfera(model, COLOR_EJE);

		/*---------- PLATO DE LA ANTENA ----------
		El cono se creo con radio 2.0, por eso se escala 0.6 para dejarlo en 1.2.
		Se gira 180 grados sobre X para voltearlo y que quede como plato.*/
		model = modelaux4;
		model = glm::translate(model, glm::vec3(0.0f, 0.6f, 0.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.6f, 1.2f, 0.6f));
		DibujarCono(model, COLOR_METAL);

		//---------- RECEPTOR DEL PLATO ----------
		model = modelaux4;
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f, 1.4f, 0.12f));
		DibujarCilindro(model, COLOR_OSCURO);

		model = modelaux4;
		model = glm::translate(model, glm::vec3(0.0f, 2.2f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		DibujarEsfera(model, COLOR_ROJO);

		//---------- INSTRUMENTO LATERAL DEL CUERPO ----------
		model = modelaux2;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.8f));
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.2f));
		DibujarPrisma(model, COLOR_ROJO);

		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}
