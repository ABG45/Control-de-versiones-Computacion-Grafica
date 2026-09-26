/*
Practica 5 - Holocron y Satelite
Carga de modelos por piezas, jerarquia y rotaciones con contexto.

QUE RESUELVE CADA PARTE DEL ENUNCIADO
-------------------------------------
1) Importar el Holocron con jerarquia .... CargarModelo() carga las 9 piezas;
                                           el Nucleo es la raiz y de el cuelgan
                                           las 8 esquinas.
2) Girar cada esquina alrededor del centro  Las 8 esquinas tienen offset (0,0,0),
   de forma independiente y con teclado ... su origen ES el centro del cubo, asi
                                           que rotarlas las hace girar en su sitio.
                                           Cada una tiene su propio angulo.
3) Importar el Satelite con jerarquia .... El Cuerpo es la raiz y las 4 piezas
                                           restantes cuelgan de el.
4) Mover el satelite en X, Y y Z ......... Se mueve la RAIZ (posSatelite). Como
                                           todo cuelga del cuerpo, el satelite
                                           completo se traslada junto.
5) Rotaciones con contexto ............... 4 rotaciones, cada una en su union
                                           con el cuerpo y no alrededor del
                                           centro del satelite:
                                             - ala izquierda sobre su brazo (Z)
                                             - ala derecha sobre su brazo (Z)
                                             - antena en su base (Y y Z)
                                             - modulo de cola sobre el eje largo (X)

CONTROLES
---------
  CAMARA
	W A S D / mouse ..... mover y mirar

  HOLOCRON
	1 a 8 ............... selecciona una de las 8 esquinas
	R / F ............... gira la esquina seleccionada (cada una es independiente)
	M ................... abre o cierra el Holocron: las 8 esquinas a la vez
	9 ................... regresa las 8 esquinas a cero

  SATELITE
	Flechas Izq / Der ... mueve el satelite en X
	Flechas Arr / Aba ... mueve el satelite en Z
	RePag / AvPag ....... mueve el satelite en Y
	T / G ............... gira el ala izquierda en su brazo
	Y / H ............... gira el ala derecha en su brazo
	B / N ............... gira las dos alas al mismo tiempo
	U / J ............... antena: giro lateral (eje Y)
	I / K ............... antena: elevacion (eje Z)
	O / L ............... modulo trasero sobre el eje largo (eje X)
	0 ................... regresa el satelite a su posicion y angulos iniciales

  ESC ................... salir
*/

//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <string>
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
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

#include "jerarquia_escena.h"   // struct ParteModelo + las dos tablas

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshListColor;
std::vector<MeshModel*> meshListModel;
std::vector<Shader> shaderList;

Camera camera;
Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

static const char* vShader = "shaders/shader_m.vert";
static const char* fShader = "shaders/shader_m.frag";

//====================== COLOCACION EN LA ESCENA ======================
const GLfloat kAlturaPiso = -2.0f;

// HOLOCRON: cubo de 10.5 de lado. Se deja flotando sobre el piso.
const GLfloat kEscalaHolocron = 0.30f;
const glm::vec3 kPosHolocron = glm::vec3(-5.0f, kAlturaPiso + 2.6f, 0.0f);

// SATELITE: 14.3 x 7.9 x 23.6. Tambien flota; ademas se mueve con el teclado.
const GLfloat kEscalaSatelite = 0.22f;
const glm::vec3 kPosSateliteInicial = glm::vec3(5.0f, kAlturaPiso + 3.0f, 0.0f);

//====================== ESTADO ======================
// --- Holocron ---
Model     HolocronParte[kNumPartesHolocron];
GLfloat   anguloHolocron[kNumPartesHolocron] = { 0.0f };
glm::mat4 matrizHolocron[kNumPartesHolocron];

int     esquinaActiva = 1;       // indices 1..8 son las esquinas; 0 es el nucleo
bool    holocronAbierto = false;
GLfloat aperturaHolocron = 0.0f; // 0 = cerrado, 1 = abierto
const GLfloat kAnguloApertura = 120.0f;

// --- Satelite ---
Model     SateliteParte[kNumPartesSatelite];
GLfloat   anguloSatelite[kNumPartesSatelite] = { 0.0f };
glm::mat4 matrizSatelite[kNumPartesSatelite];

glm::vec3 posSatelite = kPosSateliteInicial;  // punto 4: se mueve la raiz
GLfloat   anguloAntenaElev = 0.0f;            // segundo giro de la antena (eje Z)


void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		//	x      y      z			u	   v		nx	  ny    nz
			-10.0f, 0.0f, -10.0f,	0.0f,  0.0f,	0.0f, -1.0f, 0.0f,
			 10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
			-10.0f, 0.0f,  10.0f,	0.0f,  10.0f,	0.0f, -1.0f, 0.0f,
			 10.0f, 0.0f,  10.0f,	10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	MeshModel *piso = new MeshModel();
	piso->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(piso);
}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


// Carga las piezas de una tabla desde su carpeta
void CargarModelo(const ParteModelo* tabla, int n, Model* destino,
	const char* carpeta, const char* titulo)
{
	printf("--- %s ---\n", titulo);
	for (int i = 0; i < n; i++)
	{
		std::string ruta = std::string(carpeta) + tabla[i].archivo;
		destino[i].LoadModel(ruta);
		printf("  [%d] %-16s padre: %s\n", i, tabla[i].nombre,
			(tabla[i].padre < 0) ? "(raiz)" : tabla[tabla[i].padre].nombre);
	}
	printf("\n");
}


// Convierte el eje de la tabla en un vec3 utilizable
glm::vec3 EjeDeGiro(const float eje[3])
{
	return glm::vec3(eje[0], eje[1], eje[2]);
}


// True si la pieza no gira (su eje es {0,0,0})
bool NoGira(const float eje[3])
{
	return eje[0] == 0.0f && eje[1] == 0.0f && eje[2] == 0.0f;
}


//==================================================================
//  COLORES
//==================================================================
glm::vec3 ColorHolocron(int i)
{
	if (i == 0) return glm::vec3(0.45f, 0.28f, 0.08f);          // nucleo: bronce oscuro
	if (i == esquinaActiva) return glm::vec3(1.00f, 0.85f, 0.35f); // la esquina seleccionada, mas clara
	return glm::vec3(0.78f, 0.60f, 0.22f);                       // las demas esquinas: dorado
}

glm::vec3 ColorSatelite(int i)
{
	if (i == kSatAlaIzq || i == kSatAlaDer) return glm::vec3(0.12f, 0.18f, 0.50f); // celdas solares
	if (i == kSatAntena) return glm::vec3(0.78f, 0.78f, 0.80f); // antena gris claro
	if (i == kSatModulo) return glm::vec3(0.55f, 0.55f, 0.58f); // modulo gris
	return glm::vec3(0.85f, 0.68f, 0.30f);                       // cuerpo dorado
}


//==================================================================
//  TECLADO
//==================================================================
void ActualizarEscena(bool* keys, GLfloat dt)
{
	const GLfloat velGiro = 0.8f * dt;   // grados por cuadro
	const GLfloat velMov = 0.04f * dt;  // unidades por cuadro

	//---------------- HOLOCRON ----------------
	// Elegir cual esquina se mueve con R / F
	static int esquinaAnterior = -1;
	if (keys[GLFW_KEY_1]) esquinaActiva = 1;
	if (keys[GLFW_KEY_2]) esquinaActiva = 2;
	if (keys[GLFW_KEY_3]) esquinaActiva = 3;
	if (keys[GLFW_KEY_4]) esquinaActiva = 4;
	if (keys[GLFW_KEY_5]) esquinaActiva = 5;
	if (keys[GLFW_KEY_6]) esquinaActiva = 6;
	if (keys[GLFW_KEY_7]) esquinaActiva = 7;
	if (keys[GLFW_KEY_8]) esquinaActiva = 8;
	if (esquinaActiva != esquinaAnterior)
	{
		printf("Esquina seleccionada: %s   (R / F la giran)\n",
			kPartesHolocron[esquinaActiva].nombre);
		esquinaAnterior = esquinaActiva;
	}

	// Girar la esquina seleccionada. Cada esquina guarda su propio angulo,
	// por eso son independientes entre si.
	if (keys[GLFW_KEY_R]) anguloHolocron[esquinaActiva] += velGiro;
	if (keys[GLFW_KEY_F]) anguloHolocron[esquinaActiva] -= velGiro;

	// Abrir o cerrar las 8 al mismo tiempo (solo al momento de presionar M)
	static bool teclaMAnterior = false;
	if (keys[GLFW_KEY_M] && !teclaMAnterior)
	{
		holocronAbierto = !holocronAbierto;
		printf(holocronAbierto ? "Holocron: ABRIENDO\n" : "Holocron: cerrando\n");
	}
	teclaMAnterior = keys[GLFW_KEY_M];

	GLfloat paso = 0.02f * dt;
	if (holocronAbierto) aperturaHolocron += paso;
	else                 aperturaHolocron -= paso;
	if (aperturaHolocron > 1.0f) aperturaHolocron = 1.0f;
	if (aperturaHolocron < 0.0f) aperturaHolocron = 0.0f;

	// Regresar las esquinas a cero
	if (keys[GLFW_KEY_9])
	{
		for (int i = 0; i < kNumPartesHolocron; i++) anguloHolocron[i] = 0.0f;
		holocronAbierto = false;
	}

	//---------------- SATELITE ----------------
	// Punto 4: se mueve la RAIZ. Todo lo demas cuelga de ella y la sigue.
	if (keys[GLFW_KEY_LEFT])      posSatelite.x -= velMov;
	if (keys[GLFW_KEY_RIGHT])     posSatelite.x += velMov;
	if (keys[GLFW_KEY_PAGE_UP])   posSatelite.y += velMov;
	if (keys[GLFW_KEY_PAGE_DOWN]) posSatelite.y -= velMov;
	if (keys[GLFW_KEY_UP])        posSatelite.z -= velMov;
	if (keys[GLFW_KEY_DOWN])      posSatelite.z += velMov;

	// Punto 5: cada pieza gira en SU union con el cuerpo
	if (keys[GLFW_KEY_T]) anguloSatelite[kSatAlaIzq] += velGiro;   // ala izquierda
	if (keys[GLFW_KEY_G]) anguloSatelite[kSatAlaIzq] -= velGiro;

	if (keys[GLFW_KEY_Y]) anguloSatelite[kSatAlaDer] += velGiro;   // ala derecha
	if (keys[GLFW_KEY_H]) anguloSatelite[kSatAlaDer] -= velGiro;

	if (keys[GLFW_KEY_B])                                          // las dos alas juntas
	{
		anguloSatelite[kSatAlaIzq] += velGiro;
		anguloSatelite[kSatAlaDer] += velGiro;
	}
	if (keys[GLFW_KEY_N])
	{
		anguloSatelite[kSatAlaIzq] -= velGiro;
		anguloSatelite[kSatAlaDer] -= velGiro;
	}

	if (keys[GLFW_KEY_U]) anguloSatelite[kSatAntena] += velGiro;   // antena: giro lateral
	if (keys[GLFW_KEY_J]) anguloSatelite[kSatAntena] -= velGiro;
	if (keys[GLFW_KEY_I]) anguloAntenaElev += velGiro;             // antena: elevacion
	if (keys[GLFW_KEY_K]) anguloAntenaElev -= velGiro;

	if (keys[GLFW_KEY_O]) anguloSatelite[kSatModulo] += velGiro;   // modulo de cola
	if (keys[GLFW_KEY_L]) anguloSatelite[kSatModulo] -= velGiro;

	// La elevacion de la antena se limita para que el plato no atraviese el cuerpo
	if (anguloAntenaElev > 60.0f) anguloAntenaElev = 60.0f;
	if (anguloAntenaElev < -60.0f) anguloAntenaElev = -60.0f;

	// Regresar el satelite a su estado inicial
	if (keys[GLFW_KEY_0])
	{
		posSatelite = kPosSateliteInicial;
		for (int i = 0; i < kNumPartesSatelite; i++) anguloSatelite[i] = 0.0f;
		anguloAntenaElev = 0.0f;
	}
}


//==================================================================
//  DIBUJO DEL HOLOCRON
//  El nucleo es la raiz. Las 8 esquinas cuelgan de el y giran sobre
//  su propia diagonal. Como su offset es (0,0,0), giran alrededor del
//  centro del cubo sin desplazarse.
//==================================================================
void DibujarHolocron(glm::mat4 base, GLuint uniformModel, GLuint uniformColor)
{
	for (int i = 0; i < kNumPartesHolocron; i++)
	{
		const ParteModelo& parte = kPartesHolocron[i];

		glm::mat4 matrizPadre = (parte.padre < 0) ? base : matrizHolocron[parte.padre];

		glm::vec3 offset(parte.offset[0], parte.offset[1], parte.offset[2]);

		// El angulo de cada esquina es el suyo propio MAS la apertura general,
		// para que la tecla M abra las 8 sin borrar lo que se movio a mano.
		GLfloat angulo = anguloHolocron[i];
		if (i > 0) angulo += kAnguloApertura * aperturaHolocron;

		// M_pieza = M_padre * Traslacion(offset) * Rotacion(eje, angulo)
		glm::mat4 m = matrizPadre;
		m = glm::translate(m, offset);
		if (!NoGira(parte.eje) && angulo != 0.0f)
			m = glm::rotate(m, angulo * toRadians, EjeDeGiro(parte.eje));

		matrizHolocron[i] = m;

		glm::vec3 color = ColorHolocron(i);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
		HolocronParte[i].RenderModel();
	}
}


//==================================================================
//  DIBUJO DEL SATELITE
//  El cuerpo es la raiz. Las otras cuatro piezas se trasladan hasta su
//  union con el cuerpo y ahi giran, no en el centro del satelite.
//==================================================================
void DibujarSatelite(glm::mat4 base, GLuint uniformModel, GLuint uniformColor)
{
	for (int i = 0; i < kNumPartesSatelite; i++)
	{
		const ParteModelo& parte = kPartesSatelite[i];

		glm::mat4 matrizPadre = (parte.padre < 0) ? base : matrizSatelite[parte.padre];

		glm::vec3 offset(parte.offset[0], parte.offset[1], parte.offset[2]);

		// 1) llegar al punto de union con el padre
		glm::mat4 m = matrizPadre;
		m = glm::translate(m, offset);

		// 2) girar ahi mismo, sobre el eje que le corresponde a esa union
		if (!NoGira(parte.eje) && anguloSatelite[i] != 0.0f)
			m = glm::rotate(m, anguloSatelite[i] * toRadians, EjeDeGiro(parte.eje));

		// La antena tiene un segundo giro, la elevacion del plato sobre Z.
		// Se aplica despues del giro lateral, como una cabeza que primero
		// voltea y luego se inclina.
		if (i == kSatAntena && anguloAntenaElev != 0.0f)
			m = glm::rotate(m, anguloAntenaElev * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

		matrizSatelite[i] = m;

		glm::vec3 color = ColorSatelite(i);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
		SateliteParte[i].RenderModel();
	}
}


int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 2.0f, 14.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f, -8.0f, 0.3f, 0.3f);

	//Cargar los dos modelos pieza por pieza
	CargarModelo(kPartesHolocron, kNumPartesHolocron, HolocronParte,
		"Models/Holocron/", "HOLOCRON (9 piezas)");
	CargarModelo(kPartesSatelite, kNumPartesSatelite, SateliteParte,
		"Models/Satelite/", "SATELITE (5 piezas)");
	printf("Listo. 1-8 eligen esquina, R/F la giran, M abre el Holocron.\n");
	printf("Flechas y RePag/AvPag mueven el satelite.\n\n");

	//Crear Skybox con sus 6 texturas
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f,
		(GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = (GLfloat)glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / (GLfloat)limitFPS;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		ActualizarEscena(mainWindow.getsKeys(), deltaTime);

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

		//---------------- PISO ----------------
		color = glm::vec3(0.5f, 0.5f, 0.5f);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, kAlturaPiso, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshListModel[0]->RenderMeshModel();

		//---------------- HOLOCRON ----------------
		model = glm::mat4(1.0);
		model = glm::translate(model, kPosHolocron);
		model = glm::scale(model, glm::vec3(kEscalaHolocron));
		DibujarHolocron(model, uniformModel, uniformColor);

		//---------------- SATELITE ----------------
		// posSatelite es lo que mueven las flechas: al ser la matriz de la
		// raiz, arrastra las cuatro piezas que cuelgan del cuerpo.
		model = glm::mat4(1.0);
		model = glm::translate(model, posSatelite);
		model = glm::scale(model, glm::vec3(kEscalaSatelite));
		DibujarSatelite(model, uniformModel, uniformColor);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
