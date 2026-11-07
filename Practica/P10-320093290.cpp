#define STB_IMAGE_IMPLEMENTATION

#include <vector>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"
#include "DatosEscena.h"
#include "KeyframesCohete.h"

//iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// ================== 1. VARIABLES GLOBALES ==================

Window mainWindow;
std::vector<MeshModel*> meshListModel; //mallas hechas a mano, cada vertice lleva x y z de la posicion, u v de la textura y nx ny nz de la normal
std::vector<Shader> shaderList;

Camera camera;

Skybox skybox;

Material Material_brillante;
Material Material_opaco;
Material Material_metal;
Material Material_avatar;

//deltaTime suma como 61 por segundo, por eso las esperas en segundos se dividen entre 61
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

//luz direccional, es como el sol y solo hay una
DirectionalLight mainLight;
//arreglos de luces, el maximo de cada una esta en CommonValues.h
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

//shaders que usan todos los objetos
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

glm::vec3 posXen = glm::vec3(15000.0f, 0.0f, 0.0f);
glm::vec3 posPlaneta2 = glm::vec3(-15000.0f, 0.0f, 0.0f);
glm::vec3 posVerdania = glm::vec3(0.0f, 0.0f, 15000.0f);

//--- personajes ---
Model CapitanFalcon_M[NUM_PIEZAS_FALCON];
glm::vec3 rotFalcon[NUM_PIEZAS_FALCON];
Model Carnivora_M[NUM_PIEZAS_CARNIVORA];
glm::vec3 rotCarnivora[NUM_PIEZAS_CARNIVORA]; //las globales empiezan en 0
Model Gordon_M[NUM_PIEZAS_GORDON];
glm::vec3 rotGordon[NUM_PIEZAS_GORDON];
Model Kirby_M[NUM_PIEZAS_KIRBY];
glm::vec3 rotKirby[NUM_PIEZAS_KIRBY];

//--- planetas: Xen y planeta 2 ---
//Xen
Skybox skyboxXenDia;
Skybox skyboxXenNoche;
Model XenPlaneta_M;
glm::vec3 posPlanetaXen = glm::vec3(300.0f, 625.0f, -3600.0f);
float giroPlanetaXen = 0.0f;
float giroPlanetaXenOffset = 0.05f;
Model XenSuelo_M;
//objetos de la superficie
Model XenCristal_M;
Model XenIsla_M;
Model XenTallo_M;
Model XenFoco_M;
//las criaturas son por piezas y se animan con el seno de anguloXen por numeros enteros
Model Houndeye_M[NUM_PIEZAS_HOUNDEYE];
glm::vec3 rotHoundeye[NUM_PIEZAS_HOUNDEYE];
Model Bullsquid_M[NUM_PIEZAS_BULLSQUID];
glm::vec3 rotBullsquid[NUM_PIEZAS_BULLSQUID];
Model Vortigaunt_M[NUM_PIEZAS_VORTIGAUNT];
glm::vec3 rotVortigaunt[NUM_PIEZAS_VORTIGAUNT];
Model AlienGrunt_M[NUM_PIEZAS_GRUNT];
glm::vec3 rotGrunt[NUM_PIEZAS_GRUNT];
float anguloXen = 0.0f;
float anguloXenOffset = 2.0f;
//estadoHoundeye vale 0 caminando, 1 agachandose y 2 levantandose de golpe (su ataque sonico)
const int NUM_HOUNDEYES = 3;
glm::vec3 centroHoundeyes = glm::vec3(7.0f, 0.0f, -5.0f); //se mide desde posXen
float radioHoundeyes = 2.2f;
float anguloHoundeyes = 0.0f;
float anguloHoundeyesOffset = 0.5f;
int estadoHoundeye = 0;
float agacheHoundeye = 0.0f;
float tiempoHoundeye = 0.0f;
float tiempoCaminaHoundeye = 300.0f; //como deltaTime suma como 61 en un segundo, son como 5 segundos
float bajaHoundeye = 0.012f;
float subeHoundeye = 0.15f;
//estadoBullsquid vale 0 esperando, 1 echandose, 2 escupiendo y 3 regresando
int estadoBullsquid = 0;
float inclinaBullsquid = 0.0f;
float esperaBullsquid = 0.0f;
float tiempoEsperaBullsquid = 240.0f;
float atrasBullsquid = -15.0f;
float adelanteBullsquid = 10.0f;
//estadoVortigaunt vale 0 cargando, 1 soltando y 2 esperando
int estadoVortigaunt = 0;
float cargaVortigaunt = 0.0f;
float esperaVortigaunt = 0.0f;
float tiempoEsperaVortigaunt = 120.0f;
glm::vec3 posVortigaunt = glm::vec3(0.5f, 0.0f, -6.0f); //se mide desde posXen
//la cabeza va de un lado a otro mientras espera y vuelve al frente al cargar
float cabezaVortigaunt = 0.0f;
float cabezaVortigauntOffset = 0.6f;
float limiteCabezaVortigaunt = 25.0f;
bool cabezaVortigauntIzq = true;
//estadoGrunt vale 0 caminando, 1 levantando el arma, 2 apuntando y 3 bajandola
glm::vec3 centroGrunt = glm::vec3(6.0f, 0.0f, -13.0f); //se mide desde posXen
float posGrunt = 0.0f;
float posGruntOffset = 0.02f;
float limiteGrunt = 3.5f;
bool gruntAdelante = true;
float rumboGrunt = 90.0f;
float rumboGruntOffset = 3.0f;
int estadoGrunt = 0;
float apuntaGrunt = 0.0f;
float esperaGrunt = 0.0f;
float tiempoApuntaGrunt = 90.0f;
//el charco se recorre con un offset de UV para que el liquido se vea fluir
Model XenCharco_M;
glm::vec3 posCharco = glm::vec3(-6.0f, 0.02f, -8.0f); //se sube un poco para que no se pelee con el suelo
float charcoU = 0.0f;
float charcoV = 0.0f;
//estadoArbol vale 0 esperando, 1 levantando el brazo y 2 azotandolo; golpeArbol son los grados del brazo en X
Model ArbolXen_M[NUM_PIEZAS_ARBOL_XEN];
glm::vec3 rotArbolXen[NUM_PIEZAS_ARBOL_XEN];
int estadoArbol[NUM_ARBOLES_XEN] = { 0, 0, 0, 0 };
float golpeArbol[NUM_ARBOLES_XEN] = { 0.0f, 0.0f, 0.0f, 0.0f };
float esperaArbol[NUM_ARBOLES_XEN] = { 0.0f, 100.0f, 200.0f, 50.0f }; //empiezan con tiempos distintos para no ir juntos
float tiempoEsperaArbol = 300.0f;
float subeArbol = 1.0f;
float bajaArbol = 8.0f;
float arribaArbol = -60.0f; //con los negativos la punta del brazo sube hacia el frente
float pisoArbol = 12.0f;  //con 12 grados el brazo queda un poco mas abajo que como viene en el modelo
Model Boid_M[NUM_PIEZAS_BOID];
glm::vec3 rotBoid[NUM_PIEZAS_BOID];
const int NUM_BOIDS = 5;
glm::vec3 centroBoids = glm::vec3(0.0f, 22.0f, -12.0f); //se mide desde posXen
float radioBoids = 20.0f;
float anguloBoids = 0.0f;
float anguloBoidsOffset = 0.25f;
Model Nihilanth_M;
glm::vec3 posNihilanth = glm::vec3(127.0f, 50.0f, -265.0f); //se mide desde posXen; asi queda arriba a la derecha y las islas no lo tapan
float giroNihilanth = 0.0f;
float giroNihilanthOffset = 0.5f;
float anguloIslas = 0.0f;
float anguloIslasOffset = 0.6f;
//alturaPlantas escala el tallo en Y, de 1 parada a 0.1 escondida
float alturaPlantas[NUM_PLANTAS] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
float altoTallo = 1.62f; //altura de la punta del tallo, ahi va el foco
//tiempoPlantas cuenta lo que lleva el estado actual
float tiempoPlantas = 0.0f;
bool plantasEscondidas = false;
float tiempoParadas = 300.0f;
float tiempoEscondidas = 180.0f;
//la F prende y apaga la caminata del headcrab
Model Headcrab_M[NUM_PIEZAS_HEADCRAB];
glm::vec3 rotHeadcrab[NUM_PIEZAS_HEADCRAB];
glm::vec3 centroHeadcrab = glm::vec3(0.0f, 0.0f, 2.0f); //el centro del circulo se mide desde posXen
float radioHeadcrab = 2.5f;
float anguloHeadcrab = 0.0f;
float anguloHeadcrabOffset = 0.4f;
float pasoHeadcrab = 0.0f;
float pasoHeadcrabOffset = 2.5f;
float limitePaso = 20.0f;
bool pasoAdelante = true;
//el bacteriofago tambien camina en circulo en la isla
Model Bacteriofago_M[NUM_PIEZAS_BACTERIOFAGO];
glm::vec3 rotBacteriofago[NUM_PIEZAS_BACTERIOFAGO];
glm::vec3 centroBacteriofago = glm::vec3(3.5f, 0.0f, 8.0f); //se mide desde posXen
float radioBacteriofago = 2.0f;
float anguloBacteriofago = 0.0f;
float anguloBacteriofagoOffset = 0.18f;
float escalaBacteriofago = 0.20f;

//planeta 2, que es Pop Star, el planeta de Kirby
Skybox skyboxPlaneta2Dia;
Skybox skyboxPlaneta2Noche;
Texture planeta2SueloTexture;
Model Planeta2Espacio_M;
glm::vec3 posPlaneta2Espacio = glm::vec3(-2730.0f, -375.0f, 1650.0f);
float giroPlaneta2 = 0.0f;
float giroPlaneta2Offset = 0.04f;
//objetos de la superficie
Model Planeta2Colina_M;
Model Planeta2Arbol_M;
Model Planeta2Nube1_M;
Model Planeta2Nube2_M;
Model WarpStar_M;
glm::vec3 posWarpStar = glm::vec3(4.0f, 0.8f, -4.0f);
Model WaddleDee_M[NUM_PIEZAS_WADDLE];
glm::vec3 rotWaddleDee[NUM_PIEZAS_WADDLE];
Model WaddleDoo_M[NUM_PIEZAS_WADDLE];
glm::vec3 rotWaddleDoo[NUM_PIEZAS_WADDLE];
Model BandanaDee_M[NUM_PIEZAS_BANDANA];
glm::vec3 rotBandana[NUM_PIEZAS_BANDANA];
Model KingDedede_M[NUM_PIEZAS_DEDEDE];
glm::vec3 rotDedede[NUM_PIEZAS_DEDEDE];
//el seno de anguloHabitantes usa multiplicadores enteros para que no brinque al pasar de 360
float anguloHabitantes = 0.0f;
float anguloHabitantesOffset = 2.0f;
//el centro se mide desde posPlaneta2
glm::vec3 centroWaddleDee = glm::vec3(-11.0f, 0.0f, -3.0f);
float radioWaddleDee = 3.0f;
float anguloWaddleDee = 0.0f;
float anguloWaddleDeeOffset = 0.5f;
float posBandana = 0.0f;
float posBandanaOffset = 0.015f;
float limiteBandana = 2.5f;
bool bandanaAdelante = true;
float rumboBandana = 90.0f;
float rumboBandanaOffset = 6.0f;
//estadoDedede vale 0 levantando, 1 azotando y 2 esperando; golpeDedede son los grados del brazo en X
int estadoDedede = 0;
float golpeDedede = 0.0f;
float subeDedede = 1.2f;
float bajaDedede = 9.0f;
float arribaDedede = -110.0f;
float pisoDedede = 8.0f; //con 8 grados la cara del martillo queda justo en el piso
float esperaDedede = 0.0f;
float tiempoEsperaDedede = 90.0f; //como deltaTime suma como 61 en un segundo, son como 1.5 segundos
Model Halberd_M;
glm::vec3 centroHalberd = glm::vec3(0.0f, 30.0f, -30.0f); //se mide desde posPlaneta2
float radioHalberd = 55.0f;
float anguloHalberd = 0.0f;
float anguloHalberdOffset = 0.08f;

//--- planeta 3: Verdania ---
Skybox skyboxVerdaniaDia;
Skybox skyboxVerdaniaNoche;
Texture verdaniaPastoTexture;
Model PlanetaVerdania_M;
glm::vec3 posPlanetaVerdania = glm::vec3(3330.0f, 250.0f, 1650.0f);
//centro de la cabeza; ahi se gira y se escala para que no gire alrededor de los pies
glm::vec3 centroCabezaPea = glm::vec3(0.0f, 1.15f, 0.05f);
float giroPlanetaVerdania = 0.0f;
float giroPlanetaVerdaniaOffset = 0.05f;

//anguloVerdania marca el tiempo de las animaciones de las plantas y solo avanza con la F prendida
float anguloVerdania = 0.0f;
float anguloVerdaniaOffset = 2.0f;

//peashooter
Model Peashooter_M[NUM_PIEZAS_PEASHOOTER];
glm::vec3 rotPeashooter[NUM_PIEZAS_PEASHOOTER];
glm::vec3 centroPeashooter = glm::vec3(0.0f, 0.0f, -2.0f); //el centro del circulo se mide desde posVerdania
float radioPeashooter = 2.5f;
float anguloPeashooter = 0.0f;
float anguloPeashooterOffset = 0.4f;

//crazy dave
Model CrazyDave_M[NUM_PIEZAS_CRAZYDAVE];
glm::vec3 rotCrazyDave[NUM_PIEZAS_CRAZYDAVE];
float posDave = 0.0f;
float posDaveOffset = 0.02f;
float limiteDave = 4.0f;
bool daveAdelante = true;
//hacia donde mira crazy dave; se gira poco a poco entre 90 y -90 para que la vuelta no sea de golpe
float rumboDave = 90.0f;
float rumboDaveOffset = 6.0f;

Model IcePea_M[NUM_PIEZAS_ICEPEA];
glm::vec3 rotIcePea[NUM_PIEZAS_ICEPEA];
Model GatlingPea_M[NUM_PIEZAS_GATLINGPEA];
glm::vec3 rotGatlingPea[NUM_PIEZAS_GATLINGPEA];
float giroCanones = 0.0f; //angulo de los canones de la gatling pea, que giran sin parar
float giroCanonesOffset = 8.0f;
Model FirePea_M[NUM_PIEZAS_FIREPEA];
glm::vec3 rotFirePea[NUM_PIEZAS_FIREPEA];
Model Cactus_M[NUM_PIEZAS_CACTUS];
glm::vec3 rotCactus[NUM_PIEZAS_CACTUS];

//plantas de adorno
Model Sunflower_M[NUM_PIEZAS_SUNFLOWER];
glm::vec3 rotSunflower[NUM_PIEZAS_SUNFLOWER];
Model Sunflower2_M[NUM_PIEZAS_SUNFLOWER2];
glm::vec3 rotSunflower2[NUM_PIEZAS_SUNFLOWER2];
Model StuffyFlower_M[NUM_PIEZAS_STUFFYFLOWER];
glm::vec3 rotStuffyFlower[NUM_PIEZAS_STUFFYFLOWER];
Model HypnoShroom_M[NUM_PIEZAS_HYPNOSHROOM];
glm::vec3 rotHypnoShroom[NUM_PIEZAS_HYPNOSHROOM];
Model LaserBean_M[NUM_PIEZAS_LASERBEAN];
glm::vec3 rotLaserBean[NUM_PIEZAS_LASERBEAN];
Model PotatoMine_M[NUM_PIEZAS_POTATOMINE];
glm::vec3 rotPotatoMine[NUM_PIEZAS_POTATOMINE];
Model Torchwood_M[NUM_PIEZAS_TORCHWOOD];
glm::vec3 rotTorchwood[NUM_PIEZAS_TORCHWOOD];
Model WallNut_M; //es una sola pieza, por eso es un Model normal

//jardin de crazy dave
Model JardinCasa_M;
Model JardinCalle_M;
Model JardinCalleFin_M;
Model JardinCerca_M;
Model JardinCercaPuerta_M;
Model JardinFarol_M;
Model JardinArbol1_M;
Model JardinArbol2_M;
Model JardinArbusto_M;
Model JardinArbustoFlores_M;
Model JardinFlores1_M;
Model JardinFlores2_M;
Model JardinRoca_M;
Model JardinCaminoPiedra_M;
Model JardinHongo_M;
Model JardinTrebol_M;
Model JardinHelecho_M;
//la casa mide 1.2 de alto y con escala 6.5 queda de casi 8; la calle, las cercas y el farol usan la misma escala
float escalaCasa = 6.5f;

//--- nave de Among Us y crewmates ---
//la nave mide como 12 de largo y tiene el frente hacia Z positivo y la puerta atras
//el cuerpo usa la textura en gris para pintarlo con su color, y el visor va en blanco para que no se pinte
Model AmongUsNave_M;
Model CrewmateCuerpo_M;
Model CrewmateVisor_M;
glm::vec3 posNaveAmongUs = glm::vec3(9.0f, 2.0f, -36.0f);
float giroNaveAmongUs = 160.0f;
//cada crewmate va desfasado 60 grados del anterior para que no se muevan todos juntos
float anguloAmongUs = 0.0f;
float anguloAmongUsOffset = 0.4f;
float avanceNave = 0.5f;  //lo mas que se mueve la nave hacia adelante o hacia atras
float avanceCrewmates = 0.4f; //lo mismo para los crewmates, que se acercan y se alejan de la puerta

//--- pista de carreras y naves ---
Model Pista_M;
Model Meta_M;
Model CubiertaMeta_M;
Model PuestoObservacion1_M;
Model PuestoObservacion2_M;
Model PuestoObservacion3_M;
Model PuestoObservacion4_M;
Model Almacen_M;
Model NavePista_M;
Model NaveNodriza_M; //el barco espacial
//naves de F-Zero que corren en la pista
const int NUM_NAVES_FZERO = 8;
Model NavesFZero_M[NUM_NAVES_FZERO];
const char* nombresNavesFZero[NUM_NAVES_FZERO] = {
	"blueFalcon", "fireStingray", "goldenFox", "wildGoose", "nightThunder", "rainbowPhoenix", "redGazzelle", "blackBull" };

//lugar de la pista en el espacio, cerca del centro de la escena
glm::vec3 posPista = glm::vec3(413.0f, 0.0f, -254.0f);

glm::mat4 modelauxPista(1.0);
glm::mat4 modelauxNaves(1.0);

//distAcumuladaPista guarda la distancia hasta cada punto, la llena InicializarRutaPista
float distAcumuladaPista[NUM_PUNTOS_PISTA + 1];
float longitudTotalPista = 0.0f;
float avanceCarrera = 0.0f; //distancia que lleva recorrida la primera nave
float velocidadCarrera = 2.5f;
const float ESCALA_INTERNA_PISTA = 0.45f; //los puntos se escalan igual que la pista, que lleva un segundo scale de 0.45

void InicializarRutaPista()
{
	longitudTotalPista = 0.0f;
	distAcumuladaPista[0] = 0.0f;
	for (int i = 0; i < NUM_PUNTOS_PISTA; i++)
	{
		//el ultimo punto se une con el primero porque la pista es un circuito cerrado
		glm::vec3 pActual = posRutaPista[i] * ESCALA_INTERNA_PISTA;
		glm::vec3 pSig = posRutaPista[(i + 1) % NUM_PUNTOS_PISTA] * ESCALA_INTERNA_PISTA;
		glm::vec3 tramo = pSig - pActual;
		longitudTotalPista += sqrt(tramo.x * tramo.x + tramo.y * tramo.y + tramo.z * tramo.z);
		distAcumuladaPista[i + 1] = longitudTotalPista;
	}
}

//matriz de la nave a cierta distancia del inicio; luego se multiplica por modelauxNaves
glm::mat4 ObtenerMatrizNavePista(float distancia, float carrilX, float alturaFlotacion)
{
	int n = NUM_PUNTOS_PISTA;

	//las naves de atras empiezan en negativo, por eso la distancia se pasa al rango de una vuelta
	float d = distancia;
	if (d < 0.0f)
		d += longitudTotalPista;
	if (d > longitudTotalPista)
		d -= longitudTotalPista;

	int idx = 0;
	for (int i = 0; i < n; i++)
	{
		if (d >= distAcumuladaPista[i] && d <= distAcumuladaPista[i + 1])
		{
			idx = i;
			break;
		}
	}

	int idxSig = (idx + 1) % n;
	int idxSig2 = (idx + 2) % n;

	float longSeg = distAcumuladaPista[idx + 1] - distAcumuladaPista[idx];
	float t = 0.0f;
	if (longSeg > 0.0001f)
		t = (d - distAcumuladaPista[idx]) / longSeg;

	glm::vec3 p0 = posRutaPista[idx] * ESCALA_INTERNA_PISTA;
	glm::vec3 p1 = posRutaPista[idxSig] * ESCALA_INTERNA_PISTA;
	glm::vec3 p2 = posRutaPista[idxSig2] * ESCALA_INTERNA_PISTA;

	glm::vec3 posCentro = p0 + (p1 - p0) * t;
	glm::vec3 normalPiso = glm::normalize(normalRutaPista[idx] + (normalRutaPista[idxSig] - normalRutaPista[idx]) * t);

	//se mezcla la direccion de este tramo con la del siguiente para que gire suave en las curvas
	glm::vec3 tangenteActual = glm::normalize(p1 - p0);
	glm::vec3 tangenteSig = glm::normalize(p2 - p1);
	glm::vec3 frente = glm::normalize(tangenteActual + (tangenteSig - tangenteActual) * t);

	//con el producto cruz salen los otros dos ejes de la nave, todos perpendiculares entre si
	glm::vec3 derecha = glm::normalize(glm::cross(frente, normalPiso));
	glm::vec3 arriba = glm::normalize(glm::cross(derecha, frente));

	glm::vec3 posFinal = posCentro + (derecha * carrilX) + (arriba * alturaFlotacion);

	//las columnas son los ejes y la posicion; el eje Z va al reves porque las naves ven hacia Z negativo
	glm::mat4 transform(1.0f);
	transform[0] = glm::vec4(-derecha, 0.0f);
	transform[1] = glm::vec4(arriba, 0.0f);
	transform[2] = glm::vec4(-frente, 0.0f);
	transform[3] = glm::vec4(posFinal, 1.0f);

	return transform;
}

//--- satelites y cuerpos espaciales ---
Model Satelite_M;
//la luna y los asteroides estan alrededor de Xen y giran con el
Model LunaXen_M;
Model Asteroides_M;
const int totalAsteroides = 16;
const float radioAsteroides = 1000.0f;
const float pasoAngulo = 360.0f / totalAsteroides;

//--- plataforma de lanzamiento y cohete ---
//su lugar se mide desde el centro de Pop Star; el cohete mide 4.6 y todo se escala con escalaPlataforma
Model Cohete_M[NUM_PIEZAS_COHETE];
glm::vec3 rotCohete[NUM_PIEZAS_COHETE]; //las globales empiezan en 0
Model PlataformaCohete_M;
Model TorreCohete_M;
glm::vec3 posPlataforma = glm::vec3(-42.0f, 0.0f, -32.0f);
float escalaPlataforma = 4.0f;
int pisosTorre = 5;
glm::mat4 modelauxPlataforma(1.0);
//estadoCohete vale 0 parado, 1 en la cuenta, 2 subiendo y 3 bajando; las alturas van en unidades del modelo
int estadoCohete = 0;
float tiempoCuentaCohete = 3.0f; //la cuenta regresiva dura 3 segundos
float cuentaCohete = 0.0f;  //segundos que le faltan a la cuenta
float alturaCohete = 0.0f;  //cuanto ha subido el cohete desde la plataforma
float velocidadCohete = 0.0f;
float aceleracionCohete = 0.002f; //lo que crece la velocidad cada frame, asi sube cada vez mas rapido
float alturaMaxCohete = 60.0f;  //al llegar aqui deja de subir y regresa
float velBajadaCohete = 0.15f;  //regresa a velocidad constante y despacio, como aterrizando
float fuegoCohete = 0.0f;  //tamano de la llama: 0 apagada y 1 completa
float fuegoCuentaOffset = 0.0027f; //en la cuenta la llama crece esto cada frame; en 3 segundos llega a la mitad
//animacion por keyframes: B la reproduce, R la detiene, I imprime los keyframes y O los guarda; los lee de un archivo de texto
AnimacionCohete animCohete;
const char* archivoKeyframesCohete = "keyframes_cohete.txt";
const char* archivoKeyframesGuardado = "keyframes_cohete_guardado.txt";
int iniciosKeyframesVistos = 0;
int reinicioKeyframesVistos = 0;
int imprimeKeyframesVistos = 0;
int guardaKeyframesVistos = 0;
//mientras la llama esta prendida sale una nube cada intervaloHumo; la nueva ocupa el lugar de la mas vieja
const int MAX_HUMO = 480;
glm::vec3 posHumo[MAX_HUMO];
float edadHumo[MAX_HUMO];  //segundos desde que salio cada nube; las que ya pasaron duracionHumo no se dibujan
int siguienteHumo = 0;  //lugar de los arreglos donde va la siguiente nube
float duracionHumo = 2.5f;
float intervaloHumo = 0.015f;
float esperaHumo = 0.0f;  //segundos desde la ultima nube
float tamanoHumo = 9.0f;  //lo que mide una nube en su punto mas grande
int idxHumo = 0;  //lugar de la malla del humo en meshListModel
Texture humoTexture;

void SueltaHumo(glm::vec3 pos)
{
	posHumo[siguienteHumo] = pos;
	edadHumo[siguienteHumo] = 0.0f;
	siguienteHumo = (siguienteHumo + 1) % MAX_HUMO;
}
//su lugar se mide desde el centro del planeta 2
glm::vec3 posGordonAstronauta = glm::vec3(-27.0f, 0.0f, -16.0f);
glm::vec3 posFalconAstronauta = glm::vec3(-23.0f, 0.0f, -17.0f);
float escalaGordonAstronauta = 1.5f; //igual que el avatar; Gordon mide 1.4 y queda de 2.1
float escalaFalconAstronauta = 1.8f; //el Capitan Falcon mide 0.95 y queda de 1.7, un poco mas bajo que Gordon
float anguloAstronautas = 0.0f;
float anguloAstronautasOffset = 1.0f;
//vistaCohete va de 0 a 1 y mezcla la pose de espera con la de ver el cohete
float vistaCohete = 0.0f;
float vistaCoheteOffset = 0.02f;
//rumbos en grados de la espera y del despegue
float rumboEspera = 40.0f;
float rumboDespegue = -50.0f;
//estadoReloj vale 0 esperando, 1 subiendo el brazo, 2 viendo la hora y 3 bajandolo
int estadoReloj = 0;
float brazoReloj = 0.0f;
float brazoRelojOffset = 0.03f;
float tiempoReloj = 0.0f;
float esperaReloj = 300.0f; //unos 5 segundos entre una vez y otra
float veReloj = 120.0f;  //y unos 2 segundos viendo la hora

//--- estacion espacial y naves ---
Model Estacion_M;
Model LandingPad_M;
Model NaveDorada_M;
Model NaveBlanca_M;
Model NaveCaza_M;
glm::vec3 posEstacion = glm::vec3(613.0f, -5.0f, -350.0f);
glm::mat4 modelauxEstacion(1.0);

//los estados del 0 al 7 son espera, elevarse, girar, salir, estar fuera, regresar, acercarse y posarse
float alturaNaveEstacion = 7.5f; //lo que sube sobre su pad antes de girar
float distanciaFrenado = 50.0f;  //a esta distancia de su pad deja la hipervelocidad y se acerca despacio
float distanciaSalida = 1800.0f; //a esta distancia de su pad ya no se alcanza a ver y deja de dibujarse
float velSubidaNave = 0.08f;
float velGiroNave = 2.0f;
float velHipervelocidad = 25.0f;
float velAcercamiento = 0.3f;
//las esperas van en segundos; a la minima se le suma una de esperasNaves
float tiempoMinEstacionada = 10.0f;
float tiempoMinFuera = 15.0f;

int estadoNavesEstacion[NUM_NAVES_ESTACION];  //en que estado va, del 0 al 7
float subidaNavesEstacion[NUM_NAVES_ESTACION];  //cuanto se ha elevado sobre su pad
float avanceNavesEstacion[NUM_NAVES_ESTACION];  //cuanto se ha alejado de su pad hacia afuera
float vueltaNavesEstacion[NUM_NAVES_ESTACION];  //lo que ha girado para salir, de 0 a 180
float esperaNavesEstacion[NUM_NAVES_ESTACION];  //segundos que le faltan para salir o para regresar
bool visibleNavesEstacion[NUM_NAVES_ESTACION];  //vale false cuando ya salio de la escena, y asi no se dibuja
int turnoEsperaNaves[NUM_NAVES_ESTACION];  //lugar de esperasNaves que le toca la siguiente vez

//regresa la espera que le toca a la nave y avanza a la siguiente
float SiguienteEspera(int nave)
{
	float espera = esperasNaves[turnoEsperaNaves[nave]];
	turnoEsperaNaves[nave] = (turnoEsperaNaves[nave] + 1) % NUM_ESPERAS_NAVES;
	return espera;
}

//pone cada nave en su pad; empiezan en esperas distintas para que no salgan juntas
void InicializarNavesEstacion()
{
	for (int i = 0; i < NUM_NAVES_ESTACION; i++)
	{
		estadoNavesEstacion[i] = 0;
		subidaNavesEstacion[i] = 0.0f;
		avanceNavesEstacion[i] = 0.0f;
		vueltaNavesEstacion[i] = 0.0f;
		visibleNavesEstacion[i] = true;
		turnoEsperaNaves[i] = i % NUM_ESPERAS_NAVES;
		esperaNavesEstacion[i] = tiempoMinEstacionada + SiguienteEspera(i);
	}
}

//--- avatar ---
//el avatar es Kirby o la carnivora, y la T cambia entre los dos
glm::vec3 rotAvatar[NUM_PIEZAS_KIRBY];
//lugar del avatar desde el centro del piso del planeta; rumbo 0 ve hacia Z positivo y 180 hacia Z negativo
//llega adelante y a la izquierda de la camara libre para no taparle la vista
glm::vec3 posLlegadaAvatar = glm::vec3(-3.5f, 0.0f, 3.5f);
glm::vec3 posAvatar = posLlegadaAvatar;
float rumboAvatar = 180.0f;
float escalaAvatar = 1.5f;
float velocidadAvatar = 0.08f;
float giroAvatarOffset = 2.5f;
//el piso llega a 100 y el avatar se detiene antes de la orilla
float limitePisoAvatar = 95.0f;
//en Xen el limite es un circulo de radio 32 para que el avatar no se salga de la isla
float limiteIslaXen = 32.0f;
float pasoAvatar = 0.0f;
float pasoAvatarOffset = 3.0f;
float limitePasoAvatar = 25.0f;
bool pasoAdelanteAvatar = true;
glm::vec3 rotAvatarCarnivora[NUM_PIEZAS_CARNIVORA];
//la carnivora mide cerca de 1.4, asi que no usa la escala de Kirby para no tapar la camara 2
float escalaAvatarCarnivora = 1.0f;
//se comparan con la zona y el reinicio de Window para volver al punto de llegada
int zonaAvatar = 0;
int reinicioAvatar = 0;

//--- camaras y recorridos ---
//zona de la camara, 0 espacio, 1 Xen, 2 planeta 2 y 3 Verdania; se compara con la de Window para saber cuando cambia
int zonaActual = 0;
//contador de Window para saber si se volvio a presionar la tecla del planeta
int reinicioPlanetaActual = 0;
//camara del frame anterior, para saber cuando cambia
int modoCamaraActual = 1;
//contador de Window para saber cuando se presiono la Z en el espacio
int puntoEspacioActual = 0;
//posicion y direccion de la camara activa; las usan la luz especular, la linterna y el sol
glm::vec3 posVista = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 dirVista = glm::vec3(0.0f, 0.0f, -1.0f);
//camara 2, detras y arriba del avatar
float distanciaCamaraAvatar = 4.0f;
float alturaCamaraAvatar = 2.2f;
float alturaMiraAvatar = 0.8f;
//camara 3, detras de una nave de la carrera; los puntos se miden desde la nave, que ve hacia Z negativo
glm::vec3 camaraNaveAtras = glm::vec3(0.0f, 3.5f, 10.0f);
glm::vec3 camaraNaveMira = glm::vec3(0.0f, 1.5f, -8.0f);
//camara 4, viaje de la estacion a un planeta; G va a Xen, H a Pop Star y J a Verdania
//sale del pad que da hacia X negativo, el de abajo
//etapaViaje, de 0 a 6, es estacionada en la estacion, elevandose, girando, volando, llegando, bajando y estacionada en el planeta
Model NaveViaje_M;
Model EstelaViaje_M;
int etapaViaje = 0;
//contador de Window para saber cuando se presiono G, H o J
int reinicioViajeActual = 0;
//el modelo mide 160 de largo con la nariz hacia Z negativo; se recorre su centro para que quede en el origen
float escalaNaveViaje = 0.15f;
float centroNaveViaje = 76.5f;
//lugar de la nave en el pad, medido desde el centro de la estacion, hacia la orilla para que al subir no le pegue a la de arriba
glm::vec3 padViaje = glm::vec3(-212.5f, -3.05f, 0.0f);
float rumboPadViaje = -90.0f; //asi la nariz ve hacia X positivo, hacia la estacion
//segundos que se queda estacionada antes de salir y despues de llegar
float esperaViaje = 2.0f;
float tiempoViaje = 0.0f;
//lo que ha subido; al mismo tiempo se aleja lo mismo, asi sube en diagonal
float subidaViaje = 0.0f;
float velSubidaViaje = 0.5f;
//rumbo de la nave en grados; en la etapa 2 cambia poco a poco hasta llegar al del viaje
float rumboViaje = -90.0f;
float velGiroViaje = 1.5f;
//lo que ha volado desde que termino de girar
float avanceViaje = 0.0f;
float velVueloViaje = 8.0f;
//en el planeta llega derecha desde distanciaLlegadaViaje; llegadaViaje y bajadaViaje son lo que le falta para quedar arriba y para posarse
float alturaPosadoViaje = 25.0f;
float distanciaLlegadaViaje = 150.0f;
float llegadaViaje = 0.0f;
float velLlegadaViaje = 1.0f;
float bajadaViaje = 0.0f;
float velBajadaViaje = 0.15f;
//tamano de la estela: 0 apagada y 1 completa; mientras esta prendida lleva una luz azul adentro
float fuegoViaje = 0.0f;
PointLight luzEstelaViaje;
glm::mat4 matrizNaveViaje(1.0);
//camara 5, el recorrido del cohete; etapaRecorrido va de 0 a 3, del despegue a la vuelta por el espacio
int etapaRecorrido = 0;
//en Pop Star la camara se mide desde la plataforma
glm::vec3 camaraPlataforma = glm::vec3(30.0f, 6.0f, 45.0f);
//en el espacio la camara va atras, a un lado y arriba del cohete, para verlo de lado
float distanciaCamaraCohete = 30.0f;
float ladoCamaraCohete = 30.0f;
float alturaCamaraCohete = 12.0f;
//avance del cohete en el espacio; deja de dibujarse en avanceMaxCohete, cuando ya esta muy lejos
float avanceCoheteEspacio = 0.0f;
float velCoheteEspacio = 4.0f;
float avanceMaxCohete = 4190.0f;
//segundos que la camara se queda quieta viendo alejarse al cohete
float tiempoAlejamiento = 3.5f;
float tiempoEtapa = 0.0f;
//la camara siempre mira al centro de la vuelta
glm::vec3 centroVuelta = glm::vec3(500.0f, 100.0f, -170.0f);
float radioVuelta = 1000.0f;
float alturaVuelta = 350.0f;
float anguloVuelta = 0.0f;
//empieza del lado de X positivo y Z negativo, que es hacia donde se fue el cohete
float anguloInicioVuelta = 324.0f;
//grados por cada unidad de deltaTime, unos 15 segundos por vuelta
float velVuelta = 0.4f;

//pone el cohete en la plataforma y empieza la cuenta, para que el recorrido lo vea despegar
void ReiniciaRecorrido()
{
	animCohete.Detiene();  //los keyframes y el recorrido no pueden usar el cohete al mismo tiempo
	etapaRecorrido = 0;
	alturaCohete = 0.0f;
	velocidadCohete = 0.0f;
	fuegoCohete = 0.0f;
	rotCohete[0].z = 0.0f;
	cuentaCohete = tiempoCuentaCohete;
	estadoCohete = 1;
}

void ReiniciaViaje()
{
	etapaViaje = 0;
	tiempoViaje = 0.0f;
	subidaViaje = 0.0f;
	rumboViaje = rumboPadViaje;
	avanceViaje = 0.0f;
	llegadaViaje = distanciaLlegadaViaje;
	bajadaViaje = alturaPosadoViaje;
	fuegoViaje = 0.0f;
}

//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---
//en los planetas el sol da una vuelta en unos 2 minutos
float anguloSol = 60.0f;
float anguloSolOffset = 0.05f;
bool esDeNoche = false;
//en el espacio el sol no se mueve; esta 45 grados arriba del centro, viendo hacia Z negativo
glm::vec3 haciaSolEspacio = glm::vec3(0.0f, 0.707f, -0.707f);
//direccion desde donde llega la luz en la zona actual
glm::vec3 haciaSol = haciaSolEspacio;
glm::vec3 posSol = glm::vec3(0.0f, 0.0f, 0.0f);
//menor que el plano lejano, que esta en 7000, para que se vea, y mayor que la distancia a los planetas para que quede detras
float distanciaSol = 6000.0f;
//la N salta a la manana si es de noche o a la noche si es de dia; el ciclo sigue corriendo desde ahi
int saltoDiaNocheActual = 0;
float anguloManana = 45.0f;
float anguloNoche = 225.0f;
//el sol y la luna son esferas lejanas, por eso se escalan mucho
Model Sol_M;
Model Luna_M;
float tamanoSol = 336.0f;
float tamanoLuna = 187.0f;

//la linterna se guarda aparte y solo se copia al arreglo de spotlights cuando esta prendida
SpotLight linternaLuz;

//luces de cada zona; en el while solo se mandan las de la zona donde se esta, para no pasar del maximo
//las luminarias de los planetas se prenden de noche; las del espacio siempre estan prendidas porque ahi no hay ciclo
const int NUM_LUMINARIAS_ESPACIO = 5;
PointLight luminariasEspacio[NUM_LUMINARIAS_ESPACIO];
const int NUM_LUMINARIAS_XEN = 3;
PointLight luminariasXen[NUM_LUMINARIAS_XEN];
//luz verde del vortigaunt; se arma otra vez en cada frame
PointLight luzVortigaunt;
PointLight luminariaPlaneta2;
PointLight luminariaVerdania;
//spotlights de cada zona, se prenden y apagan con la K
const int NUM_SPOTS_ESPACIO = 4;
SpotLight spotsEspacio[NUM_SPOTS_ESPACIO];
SpotLight spotXen;
SpotLight spotPlaneta2;
SpotLight spotVerdania;


// ================== 2. GEOMETRIA HECHA A MANO ==================

//las normales apuntan hacia adentro, asi que los triangulos van en sentido horario visto desde afuera
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	//las normales van hacia abajo, o sea hacia adentro del piso, por como el shader calcula la luz
	//las UV llegan a 10 para que la textura se repita 10 veces
	GLfloat floorVertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	//dos planos cruzados, sirve para plantas o cosas con transparencia
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4, 5, 6,
	   4, 6, 7
	};

	//mismas normales que el piso para que se ilumine igual de los dos lados
	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,
	};

	//CreateMeshModel(vertices, indices, cantidad de floats del arreglo, cantidad de indices)
	MeshModel* piso = new MeshModel();
	piso->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(piso); //meshListModel[0]

	MeshModel* vegetacion = new MeshModel();
	vegetacion->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(vegetacion); //meshListModel[1]
}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1); //shaderList[0]
}


//dibuja un modelo con jerarquia; la matriz de cada pieza sale de su padre, y para girar el pivote va al origen
void DibujaJerarquia(Model piezas[], int padres[], glm::vec3 pivotes[], glm::vec3 rotaciones[], int numPiezas,
	glm::mat4 raiz, GLuint uniformModel)
{
	glm::mat4 matrices[20]; //una por pieza, para que los hijos usen la de su padre
	for (int i = 0; i < numPiezas; i++)
	{
		glm::mat4 model;
		if (padres[i] == -1)
			model = raiz;
		else
			model = matrices[padres[i]];

		model = glm::translate(model, pivotes[i]);
		model = glm::rotate(model, rotaciones[i].x * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotaciones[i].y * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotaciones[i].z * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, -pivotes[i]);
		matrices[i] = model;

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		piezas[i].RenderModel();
	}
}


//dibuja el cohete con los keyframes; cada pieza se mueve y gira sola alrededor de su centro, y se guarda la matriz de cada una para poner la llama
void DibujaCoheteKeyframes(Model piezas[], AnimacionCohete& anim, glm::mat4 raiz, GLuint uniformModel, glm::mat4 matrices[])
{
	for (int i = 0; i < NUM_PIEZAS_COHETE; i++)
	{
		glm::mat4 model = raiz;
		model = glm::translate(model, centrosCohete[i] + anim.pos[i]);
		model = glm::rotate(model, anim.rot[i].x * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, anim.rot[i].y * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, anim.rot[i].z * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, -centrosCohete[i]);
		matrices[i] = model;

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		piezas[i].RenderModel();
	}
}


//el indice de la malla se guarda antes del push_back y se dibuja con meshListModel[idx]

//--- personajes ---

//--- planetas: Xen y planeta 2 ---

//--- pista de carreras y naves ---

//--- satelites y cuerpos espaciales ---

//--- plataforma de lanzamiento y cohete ---
//tres planos cruzados de 1 por 1 para que la nube se vea desde cualquier lado, con normales hacia abajo
void CrearHumoCohete()
{
	unsigned int humoIndices[] = {
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7,
		8, 9, 10,
		8, 10, 11
	};

	GLfloat humoVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

		-0.5f, 0.0f, -0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, 0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f
	};

	MeshModel* humo = new MeshModel();
	humo->CreateMeshModel(humoVertices, humoIndices, 96, 18);
	idxHumo = meshListModel.size();
	meshListModel.push_back(humo);
}

//--- estacion espacial y naves ---

//--- avatar ---

//--- camaras y recorridos ---

//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---


int main()
{
	mainWindow = Window(1366, 768); //tambien jala con 1280x1024 o 1024x768
	mainWindow.Initialise();
	CreateObjects();
	CreateShaders();

	//Camera(posicion inicial, vector up, yaw, pitch, velocidad de movimiento, velocidad de giro)
	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	// ================== 3. CARGA DE RECURSOS ==================

	//en las texturas primero se asigna la ruta y luego se carga
	//LoadTextureA es para imagenes con canal alfa, como png o tga, y LoadTexture para las que no tienen, como jpg

	//las 6 caras del skybox van en este orden: rt, lf, dn, up, bk, ft
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/sBRight.png");//derecha, hacia X positivo
	skyboxFaces.push_back("Textures/Skybox/Left.png");//izquierda, hacia X negativo
	skyboxFaces.push_back("Textures/Skybox/DownF.png");//abajo, hacia Y negativo
	skyboxFaces.push_back("Textures/Skybox/Top.png");//arriba, hacia Y positivo
	skyboxFaces.push_back("Textures/Skybox/FrontF.png");//atras, hacia Z positivo
	skyboxFaces.push_back("Textures/Skybox/BackF.png");//enfrente, hacia Z negativo

	skybox = Skybox(skyboxFaces);

	//los skyboxes de los planetas usan el mismo orden de caras
	std::string carasSkybox[6] = { "rt", "lf", "dn", "up", "bk", "ft" };

	//materiales con Material(intensidad especular, shininess)
	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);
	//el metal brilla mas que el opaco; lo usan las naves
	Material_metal = Material(2.0f, 64);
	//brillo debil y extendido, como de goma, para que el avatar no se vea metalico
	Material_avatar = Material(0.4f, 8);

	//--- personajes ---
	for (int i = 0; i < NUM_PIEZAS_FALCON; i++)
	{
		std::string ruta = std::string("Models/capitanFalcon/capitanFalcon_") + piezasFalcon[i] + ".obj";
		CapitanFalcon_M[i] = Model();
		CapitanFalcon_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_CARNIVORA; i++)
	{
		std::string ruta = std::string("Models/Carnivora/carnivora_") + piezasCarnivora[i] + ".obj";
		Carnivora_M[i] = Model();
		Carnivora_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_GORDON; i++)
	{
		std::string ruta = std::string("Models/GordonParts/gordon_") + piezasGordon[i] + ".obj";
		Gordon_M[i] = Model();
		Gordon_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_KIRBY; i++)
	{
		std::string ruta = std::string("Models/Kirby/kirby_") + piezasKirby[i] + ".obj";
		Kirby_M[i] = Model();
		Kirby_M[i].LoadModel(ruta);
	}

	//--- planetas: Xen y planeta 2 ---
	//Xen
	std::vector<std::string> carasXenDia;
	std::vector<std::string> carasXenNoche;
	for (int i = 0; i < 6; i++)
	{
		carasXenDia.push_back("Textures/Skybox/xen_dia_" + carasSkybox[i] + ".png");
		carasXenNoche.push_back("Textures/Skybox/xen_" + carasSkybox[i] + ".png");
	}
	skyboxXenDia = Skybox(carasXenDia);
	skyboxXenNoche = Skybox(carasXenNoche);

	XenPlaneta_M = Model();
	XenPlaneta_M.LoadModel("Models/Xen/xen_planeta.obj");
	XenSuelo_M = Model();
	XenSuelo_M.LoadModel("Models/Xen/xen_suelo_isla.obj");
	XenCharco_M = Model();
	XenCharco_M.LoadModel("Models/Xen/xen_charco.obj");
	Nihilanth_M = Model();
	Nihilanth_M.LoadModel("Models/Nihilanth/nihilanth.obj");
	XenCristal_M = Model();
	XenCristal_M.LoadModel("Models/Xen/xen_cristal.obj");
	XenIsla_M = Model();
	XenIsla_M.LoadModel("Models/Xen/xen_isla.obj");
	XenTallo_M = Model();
	XenTallo_M.LoadModel("Models/Xen/xen_planta_tallo.obj");
	XenFoco_M = Model();
	XenFoco_M.LoadModel("Models/Xen/xen_planta_foco.obj");

	for (int i = 0; i < NUM_PIEZAS_HOUNDEYE; i++)
	{
		Houndeye_M[i] = Model();
		Houndeye_M[i].LoadModel(std::string("Models/Houndeye/houndeye_") + piezasHoundeye[i] + ".obj");
	}
	for (int i = 0; i < NUM_PIEZAS_BULLSQUID; i++)
	{
		Bullsquid_M[i] = Model();
		Bullsquid_M[i].LoadModel(std::string("Models/Bullsquid/bullsquid_") + piezasBullsquid[i] + ".obj");
	}
	for (int i = 0; i < NUM_PIEZAS_VORTIGAUNT; i++)
	{
		Vortigaunt_M[i] = Model();
		Vortigaunt_M[i].LoadModel(std::string("Models/Vortigaunt/vortigaunt_") + piezasVortigaunt[i] + ".obj");
	}
	for (int i = 0; i < NUM_PIEZAS_GRUNT; i++)
	{
		AlienGrunt_M[i] = Model();
		AlienGrunt_M[i].LoadModel(std::string("Models/AlienGrunt/aliengrunt_") + piezasGrunt[i] + ".obj");
	}
	for (int i = 0; i < NUM_PIEZAS_BOID; i++)
	{
		Boid_M[i] = Model();
		Boid_M[i].LoadModel(std::string("Models/Boid/boid_") + piezasBoid[i] + ".obj");
	}
	for (int i = 0; i < NUM_PIEZAS_ARBOL_XEN; i++)
	{
		ArbolXen_M[i] = Model();
		ArbolXen_M[i].LoadModel(std::string("Models/XenArbol/xen_arbol_") + piezasArbolXen[i] + ".obj");
	}

	for (int i = 0; i < NUM_PIEZAS_HEADCRAB; i++)
	{
		std::string ruta = std::string("Models/Headcrab/headcrab_") + piezasHeadcrab[i] + ".obj";
		Headcrab_M[i] = Model();
		Headcrab_M[i].LoadModel(ruta);
	}
	for (int i = 0; i < NUM_PIEZAS_BACTERIOFAGO; i++)
	{
		std::string ruta = std::string("Models/bacteriofago/") + piezasBacteriofago[i] + ".obj";
		Bacteriofago_M[i] = Model();
		Bacteriofago_M[i].LoadModel(ruta);
	}

	//planeta 2
	std::vector<std::string> carasPlaneta2Dia;
	std::vector<std::string> carasPlaneta2Noche;
	for (int i = 0; i < 6; i++)
	{
		carasPlaneta2Dia.push_back("Textures/Skybox/planeta2_dia_" + carasSkybox[i] + ".png");
		carasPlaneta2Noche.push_back("Textures/Skybox/planeta2_" + carasSkybox[i] + ".png");
	}
	skyboxPlaneta2Dia = Skybox(carasPlaneta2Dia);
	skyboxPlaneta2Noche = Skybox(carasPlaneta2Noche);

	planeta2SueloTexture = Texture("Textures/planeta2_suelo.png");
	planeta2SueloTexture.LoadTextureA();

	Planeta2Espacio_M = Model();
	Planeta2Espacio_M.LoadModel("Models/Planeta2/planeta2_planeta.obj");
	Planeta2Arbol_M = Model();
	Planeta2Arbol_M.LoadModel("Models/Planeta2/planeta2_arbol.obj");
	Planeta2Nube1_M = Model();
	Planeta2Nube1_M.LoadModel("Models/Planeta2/planeta2_nube1.obj");
	Planeta2Nube2_M = Model();
	Planeta2Nube2_M.LoadModel("Models/Planeta2/planeta2_nube2.obj");
	WarpStar_M = Model();
	WarpStar_M.LoadModel("Models/Planeta2/planeta2_warpstar.obj");
	Planeta2Colina_M = Model();
	Planeta2Colina_M.LoadModel("Models/Planeta2/planeta2_colina.obj");

	for (int i = 0; i < NUM_PIEZAS_WADDLE; i++)
	{
		WaddleDee_M[i] = Model();
		WaddleDee_M[i].LoadModel(std::string("Models/WaddleDee/waddledee_") + piezasWaddle[i] + ".obj");
		WaddleDoo_M[i] = Model();
		WaddleDoo_M[i].LoadModel(std::string("Models/WaddleDoo/waddledoo_") + piezasWaddle[i] + ".obj");
	}
	for (int i = 0; i < NUM_PIEZAS_BANDANA; i++)
	{
		BandanaDee_M[i] = Model();
		BandanaDee_M[i].LoadModel(std::string("Models/BandanaDee/bandanadee_") + piezasBandana[i] + ".obj");
	}
	for (int i = 0; i < NUM_PIEZAS_DEDEDE; i++)
	{
		KingDedede_M[i] = Model();
		KingDedede_M[i].LoadModel(std::string("Models/KingDedede/kingdedede_") + piezasDedede[i] + ".obj");
	}
	Halberd_M = Model();
	Halberd_M.LoadModel("Models/Halberd/halberd.obj");

	//--- planeta 3: Verdania ---
	std::vector<std::string> carasVerdaniaDia;
	std::vector<std::string> carasVerdaniaNoche;
	for (int i = 0; i < 6; i++)
	{
		carasVerdaniaDia.push_back("Textures/Skybox/verdania_" + carasSkybox[i] + ".png");
		carasVerdaniaNoche.push_back("Textures/Skybox/verdania_noche_" + carasSkybox[i] + ".png");
	}
	skyboxVerdaniaDia = Skybox(carasVerdaniaDia);
	skyboxVerdaniaNoche = Skybox(carasVerdaniaNoche);

	verdaniaPastoTexture = Texture("Textures/verdania_pasto.png");
	verdaniaPastoTexture.LoadTextureA();

	PlanetaVerdania_M = Model();
	PlanetaVerdania_M.LoadModel("Models/Peashooter/peashooter_planeta.obj");

	for (int i = 0; i < NUM_PIEZAS_PEASHOOTER; i++)
	{
		std::string ruta = std::string("Models/Peashooter/peashooter_") + piezasPeashooter[i] + ".obj";
		Peashooter_M[i] = Model();
		Peashooter_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_CRAZYDAVE; i++)
	{
		std::string ruta = std::string("Models/CrazyDave/crazydave_") + piezasCrazyDave[i] + ".obj";
		CrazyDave_M[i] = Model();
		CrazyDave_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_ICEPEA; i++)
	{
		std::string ruta = std::string("Models/IcePea/icepea_") + piezasIcePea[i] + ".obj";
		IcePea_M[i] = Model();
		IcePea_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_GATLINGPEA; i++)
	{
		std::string ruta = std::string("Models/GatlingPea/gatlingpea_") + piezasGatlingPea[i] + ".obj";
		GatlingPea_M[i] = Model();
		GatlingPea_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_FIREPEA; i++)
	{
		std::string ruta = std::string("Models/FirePea/firepea_") + piezasFirePea[i] + ".obj";
		FirePea_M[i] = Model();
		FirePea_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_CACTUS; i++)
	{
		std::string ruta = std::string("Models/Cactus/cactus_") + piezasCactus[i] + ".obj";
		Cactus_M[i] = Model();
		Cactus_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_SUNFLOWER; i++)
	{
		std::string ruta = std::string("Models/Sunflower/sunflower_") + piezasSunflower[i] + ".obj";
		Sunflower_M[i] = Model();
		Sunflower_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_SUNFLOWER2; i++)
	{
		std::string ruta = std::string("Models/Sunflower2/sunflower2_") + piezasSunflower2[i] + ".obj";
		Sunflower2_M[i] = Model();
		Sunflower2_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_STUFFYFLOWER; i++)
	{
		std::string ruta = std::string("Models/StuffyFlower/stuffyflower_") + piezasStuffyFlower[i] + ".obj";
		StuffyFlower_M[i] = Model();
		StuffyFlower_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_HYPNOSHROOM; i++)
	{
		std::string ruta = std::string("Models/HypnoShroom/hypnoshroom_") + piezasHypnoShroom[i] + ".obj";
		HypnoShroom_M[i] = Model();
		HypnoShroom_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_LASERBEAN; i++)
	{
		std::string ruta = std::string("Models/LaserBean/laserbean_") + piezasLaserBean[i] + ".obj";
		LaserBean_M[i] = Model();
		LaserBean_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_POTATOMINE; i++)
	{
		std::string ruta = std::string("Models/PotatoMine/potatomine_") + piezasPotatoMine[i] + ".obj";
		PotatoMine_M[i] = Model();
		PotatoMine_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_TORCHWOOD; i++)
	{
		std::string ruta = std::string("Models/Torchwood/torchwood_") + piezasTorchwood[i] + ".obj";
		Torchwood_M[i] = Model();
		Torchwood_M[i].LoadModel(ruta);
	}

	WallNut_M = Model();
	WallNut_M.LoadModel("Models/WallNut/wallnut_cuerpo.obj");

	//jardin de crazy dave
	JardinCasa_M = Model();
	JardinCasa_M.LoadModel("Models/JardinDave/jardin_casa.obj");
	JardinCalle_M = Model();
	JardinCalle_M.LoadModel("Models/JardinDave/jardin_calle.obj");
	JardinCalleFin_M = Model();
	JardinCalleFin_M.LoadModel("Models/JardinDave/jardin_calle_fin.obj");
	JardinCerca_M = Model();
	JardinCerca_M.LoadModel("Models/JardinDave/jardin_cerca.obj");
	JardinCercaPuerta_M = Model();
	JardinCercaPuerta_M.LoadModel("Models/JardinDave/jardin_cerca_puerta.obj");
	JardinFarol_M = Model();
	JardinFarol_M.LoadModel("Models/JardinDave/jardin_farol.obj");
	JardinArbol1_M = Model();
	JardinArbol1_M.LoadModel("Models/JardinDave/jardin_arbol1.obj");
	JardinArbol2_M = Model();
	JardinArbol2_M.LoadModel("Models/JardinDave/jardin_arbol2.obj");
	JardinArbusto_M = Model();
	JardinArbusto_M.LoadModel("Models/JardinDave/jardin_arbusto.obj");
	JardinArbustoFlores_M = Model();
	JardinArbustoFlores_M.LoadModel("Models/JardinDave/jardin_arbusto_flores.obj");
	JardinFlores1_M = Model();
	JardinFlores1_M.LoadModel("Models/JardinDave/jardin_flores1.obj");
	JardinFlores2_M = Model();
	JardinFlores2_M.LoadModel("Models/JardinDave/jardin_flores2.obj");
	JardinRoca_M = Model();
	JardinRoca_M.LoadModel("Models/JardinDave/jardin_roca.obj");
	JardinCaminoPiedra_M = Model();
	JardinCaminoPiedra_M.LoadModel("Models/JardinDave/jardin_camino_piedra1.obj");
	JardinHongo_M = Model();
	JardinHongo_M.LoadModel("Models/JardinDave/jardin_hongo.obj");
	JardinTrebol_M = Model();
	JardinTrebol_M.LoadModel("Models/JardinDave/jardin_trebol.obj");
	JardinHelecho_M = Model();
	JardinHelecho_M.LoadModel("Models/JardinDave/jardin_helecho.obj");

	//--- nave de Among Us y crewmates ---
	AmongUsNave_M = Model();
	AmongUsNave_M.LoadModel("Models/AmongUs/amongus_nave.obj");
	CrewmateCuerpo_M = Model();
	CrewmateCuerpo_M.LoadModel("Models/AmongUs/amongus_crewmate_cuerpo.obj");
	CrewmateVisor_M = Model();
	CrewmateVisor_M.LoadModel("Models/AmongUs/amongus_crewmate_visor.obj");

	//--- pista de carreras y naves ---
	Pista_M = Model();
	Pista_M.LoadModel("Models/pista/pista.obj");
	NavePista_M = Model();
	NavePista_M.LoadModel("Models/nave1Race/nave1Race.obj");
	Meta_M = Model();
	Meta_M.LoadModel("Models/meta/meta.obj");
	CubiertaMeta_M = Model();
	CubiertaMeta_M.LoadModel("Models/cubiertaMeta/cubiertaMeta.obj");
	PuestoObservacion1_M = Model();
	PuestoObservacion1_M.LoadModel("Models/puestoObservacion1/puestoObservacion1.obj");
	PuestoObservacion2_M = Model();
	PuestoObservacion2_M.LoadModel("Models/puestoObservacion2/puestoObservacion2.obj");
	PuestoObservacion3_M = Model();
	PuestoObservacion3_M.LoadModel("Models/puestoObservacion3/puestoObservacion3.obj");
	PuestoObservacion4_M = Model();
	PuestoObservacion4_M.LoadModel("Models/puestoObservacion4/puestoObservacion4.obj");
	Almacen_M = Model();
	Almacen_M.LoadModel("Models/warehousePort/warehousePort.obj");
	NaveNodriza_M = Model();
	NaveNodriza_M.LoadModel("Models/barcoEspacial/barcoEspacial.obj");

	InicializarRutaPista();

	//cada nave esta en su propia carpeta, con el mismo nombre que su .obj
	for (int i = 0; i < NUM_NAVES_FZERO; i++)
	{
		std::string ruta = std::string("Models/") + nombresNavesFZero[i] + "/" + nombresNavesFZero[i] + ".obj";
		NavesFZero_M[i] = Model();
		NavesFZero_M[i].LoadModel(ruta);
	}

	//--- satelites y cuerpos espaciales ---
	Satelite_M = Model();
	Satelite_M.LoadModel("Models/sateliteRace/sateliteRace.obj");
	Asteroides_M = Model();
	Asteroides_M.LoadModel("Models/asteroides/asteroides.obj");
	LunaXen_M = Model();
	LunaXen_M.LoadModel("Models/luna/luna.obj");

	//--- plataforma de lanzamiento y cohete ---
	for (int i = 0; i < NUM_PIEZAS_COHETE; i++)
	{
		std::string ruta = std::string("Models/Cohete/cohete_") + piezasCohete[i] + ".obj";
		Cohete_M[i] = Model();
		Cohete_M[i].LoadModel(ruta);
	}
	PlataformaCohete_M = Model();
	PlataformaCohete_M.LoadModel("Models/Cohete/plataforma.obj");
	TorreCohete_M = Model();
	TorreCohete_M.LoadModel("Models/Cohete/torre_estructura.obj");
	CrearHumoCohete();
	animCohete.Carga(archivoKeyframesCohete);
	humoTexture = Texture("Textures/humo.png");
	humoTexture.LoadTextureA();
	//las nubes empiezan con la edad en la que ya desaparecieron, para que no se dibujen hasta que el cohete las suelte
	for (int i = 0; i < MAX_HUMO; i++)
		edadHumo[i] = duracionHumo;

	//--- estacion espacial y naves ---
	Estacion_M = Model();
	Estacion_M.LoadModel("Models/estacionEspacial/estacionEspacial.obj");
	LandingPad_M = Model();
	LandingPad_M.LoadModel("Models/landingPad/landingPad.obj");
	NaveDorada_M = Model();
	NaveDorada_M.LoadModel("Models/naveDorada/naveDorada.obj");
	NaveBlanca_M = Model();
	NaveBlanca_M.LoadModel("Models/naveBlanca/naveBlanca.obj");
	NaveCaza_M = Model();
	NaveCaza_M.LoadModel("Models/naveCaza/naveCaza.obj");

	InicializarNavesEstacion();

	//--- avatar ---

	//--- camaras y recorridos ---
	//la nave y su estela son dos modelos para poder dibujar la estela solo cuando el motor esta prendido
	NaveViaje_M = Model();
	NaveViaje_M.LoadModel("Models/naveRecorrido/naveRecorrido.obj");
	EstelaViaje_M = Model();
	EstelaViaje_M.LoadModel("Models/naveRecorrido/estelaNaveR.obj");

	// ---------- luces ----------
	//los parametros van en este orden:
	//DirectionalLight(r, g, b, intensidad ambiental, intensidad difusa, direccion x, y, z)
	//PointLight(r, g, b, ambiental, difusa, posicion x, y, z, constante, lineal, exponente)
	//SpotLight(r, g, b, ambiental, difusa, posicion x, y, z, direccion x, y, z, constante, lineal, exponente, angulo del cono)

	//solo hay una luz direccional y siempre tiene que existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.2f, 0.4f,
		0.0f, -1.0f, -1.0f);

	//contadores de luces, se vuelven a 0 en cada frame
	unsigned int pointLightCount = 0;
	unsigned int spotLightCount = 0;

	//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---
	Sol_M = Model();
	Sol_M.LoadModel("Models/Sol/sol.obj");
	Luna_M = Model();
	Luna_M.LoadModel("Models/Sol/luna.obj");

	linternaLuz = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		10.0f);

	//los faros van antes de la cubierta de la meta, que llega hasta 52 en Z
	luminariasEspacio[0] = PointLight(1.0f, 0.15f, 0.1f,
		0.1f, 2.5f,
		posPista.x - 12.0f, posPista.y + 8.0f, posPista.z + 58.0f,
		1.0f, 0.02f, 0.002f);
	luminariasEspacio[1] = PointLight(1.0f, 0.15f, 0.1f,
		0.1f, 2.5f,
		posPista.x + 12.0f, posPista.y + 8.0f, posPista.z + 58.0f,
		1.0f, 0.02f, 0.002f);
	luminariasEspacio[2] = PointLight(0.4f, 0.7f, 1.0f,
		0.1f, 1.5f,
		posNaveAmongUs.x, posNaveAmongUs.y + 5.0f, posNaveAmongUs.z,
		1.0f, 0.05f, 0.01f);

	//en Xen cada planta de luz alumbra desde su foco, que esta en la punta del tallo
	for (int i = 0; i < NUM_LUMINARIAS_XEN; i++)
	{
		glm::vec3 pos = posXen + posPlantas[i] + glm::vec3(0.0f, altoTallo, 0.0f);
		luminariasXen[i] = PointLight(1.0f, 0.85f, 0.4f,
			0.1f, 2.5f,
			pos.x, pos.y, pos.z,
			1.0f, 0.1f, 0.02f);
	}

	luminariaPlaneta2 = PointLight(1.0f, 0.9f, 0.3f,
		0.1f, 1.5f,
		posPlaneta2.x + posWarpStar.x, posPlaneta2.y + posWarpStar.y + 1.5f, posPlaneta2.z + posWarpStar.z,
		1.0f, 0.1f, 0.02f);

	luminariaVerdania = PointLight(1.0f, 0.85f, 0.6f,
		0.1f, 3.0f,
		posVerdania.x + posCalleX[0] - 6.3f, posVerdania.y + 4.2f, posVerdania.z + posCalleZ - 4.0f,
		1.0f, 0.05f, 0.01f);

	spotsEspacio[0] = SpotLight(1.0f, 1.0f, 0.9f,
		0.0f, 3.0f,
		posPista.x - 5.0f, posPista.y + 20.0f, posPista.z + 75.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.01f, 0.001f,
		30.0f);
	spotsEspacio[1] = SpotLight(1.0f, 1.0f, 0.9f,
		0.0f, 3.0f,
		posPista.x + 5.0f, posPista.y + 20.0f, posPista.z + 75.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.01f, 0.001f,
		30.0f);

	//la posicion de cada luz sale de multiplicar un punto del edificio por su matriz
	glm::mat4 matrizPista = glm::mat4(1.0f);
	matrizPista = glm::translate(matrizPista, posPista);
	matrizPista = glm::scale(matrizPista, glm::vec3(0.45f, 0.45f, 0.45f));
	matrizPista = glm::scale(matrizPista, glm::vec3(-1.0f, 1.0f, -1.0f));

	//el faro va a 76 de altura en el edificio redondo, un poco arriba de su esfera para que no quede tapado
	glm::mat4 edificio = glm::translate(matrizPista, glm::vec3(175.0f, 80.0f, 500.0f));
	edificio = glm::rotate(edificio, -45.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 posLuz = glm::vec3(edificio * glm::vec4(0.0f, 76.0f, 0.0f, 1.0f));
	luminariasEspacio[3] = PointLight(1.0f, 0.12f, 0.08f,
		0.15f, 4.0f,
		posLuz.x, posLuz.y, posLuz.z,
		1.0f, 0.008f, 0.0002f);

	edificio = glm::translate(matrizPista, glm::vec3(175.0f, 80.0f, -375.0f));
	edificio = glm::rotate(edificio, -115.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	posLuz = glm::vec3(edificio * glm::vec4(0.0f, 76.0f, 0.0f, 1.0f));
	luminariasEspacio[4] = PointLight(1.0f, 0.12f, 0.08f,
		0.15f, 4.0f,
		posLuz.x, posLuz.y, posLuz.z,
		1.0f, 0.008f, 0.0002f);

	//la direccion no tiene que medir 1 porque SpotLight la normaliza
	edificio = glm::translate(matrizPista, glm::vec3(0.0f, 20.0f, -600.0f));
	edificio = glm::rotate(edificio, 35.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	edificio = glm::scale(edificio, glm::vec3(0.5f, 0.5f, 0.5f));
	posLuz = glm::vec3(edificio * glm::vec4(0.0f, 374.0f, 8.0f, 1.0f));
	spotsEspacio[2] = SpotLight(1.0f, 1.0f, 0.95f,
		0.0f, 6.0f,
		posLuz.x, posLuz.y, posLuz.z,
		0.10f, -0.65f, -0.75f,
		1.0f, 0.003f, 0.00005f,
		28.0f);

	edificio = glm::translate(matrizPista, glm::vec3(300.0f, 100.0f, -600.0f));
	edificio = glm::rotate(edificio, -35.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	edificio = glm::scale(edificio, glm::vec3(0.5f, 0.5f, 0.5f));
	posLuz = glm::vec3(edificio * glm::vec4(0.0f, 374.0f, 8.0f, 1.0f));
	spotsEspacio[3] = SpotLight(1.0f, 1.0f, 0.95f,
		0.0f, 6.0f,
		posLuz.x, posLuz.y, posLuz.z,
		-0.18f, -0.60f, -0.78f,
		1.0f, 0.003f, 0.00005f,
		28.0f);

	spotXen = SpotLight(0.7f, 0.3f, 1.0f,
		0.0f, 2.0f,
		posXen.x - 10.0f, posXen.y + 8.0f, posXen.z - 3.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.05f, 0.005f,
		25.0f);

	spotPlaneta2 = SpotLight(1.0f, 0.5f, 0.8f,
		0.0f, 2.0f,
		posPlaneta2.x + posWarpStar.x, posPlaneta2.y + 8.0f, posPlaneta2.z + posWarpStar.z,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.05f, 0.005f,
		25.0f);

	spotVerdania = SpotLight(1.0f, 0.9f, 0.7f,
		0.0f, 3.0f,
		posVerdania.x, posVerdania.y + 12.0f, posVerdania.z - 8.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.02f, 0.002f,
		35.0f);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	GLuint uniformTextureOffset = 0;
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);
	//el angulo de glm::perspective va en radianes, por eso 45.0f da una vista de unos 58 grados y no de 45
	//el plano lejano en 7000 para que se vean los planetas desde cualquier parte del espacio
	//la precision del depth buffer depende sobre todo del plano cercano, y mientras mas cerca de 0 este, menos precision hay
	//con 0.5 de plano cercano las capas muy pegadas, como los ojos, ya no parpadean
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.5f, 7000.0f);

	//la hora se toma justo antes del while para que el primer frame no tenga un deltaTime enorme
	lastTime = glfwGetTime();

	// ================== 4. CICLO PRINCIPAL ==================
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		//el mouse se lee siempre, pero solo mueve la camara libre, para que no brinque al regresar a ella
		if (mainWindow.getmodoCamara() == 1)
		{
			camera.keyControl(mainWindow.getsKeys(), deltaTime);
			camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
		}
		else
		{
			mainWindow.getXChange();
			mainWindow.getYChange();
		}

		glm::mat4 view = camera.calculateViewMatrix();

		//con P se pausan las animaciones poniendo deltaTime en 0; deltaReal sigue para que el avatar se mueva
		GLfloat deltaReal = deltaTime;
		if (mainWindow.getpausaAnimaciones())
			deltaTime = 0.0f;

		//--- camaras y recorridos ---
		//al regresar a la camara libre aparece donde estaba la otra y viendo hacia lo mismo
		//el yaw y el pitch salen de la direccion con atan2 y asin, al reves de como Camera::update saca la direccion a partir de ellos
		if (mainWindow.getmodoCamara() != modoCamaraActual)
		{
			if (mainWindow.getmodoCamara() == 4)
				ReiniciaViaje();
			if (mainWindow.getmodoCamara() == 5)
				ReiniciaRecorrido();
			if (mainWindow.getmodoCamara() == 1 && modoCamaraActual != 1)
			{
				float yaw = atan2(dirVista.z, dirVista.x) / toRadians;
				float pitch = asin(dirVista.y) / toRadians;
				camera = Camera(posVista, glm::vec3(0.0f, 1.0f, 0.0f), yaw, pitch, 0.3f, 0.5f);
				view = camera.calculateViewMatrix();
			}
			modoCamaraActual = mainWindow.getmodoCamara();
		}

		//al cambiar de zona, o volver a presionar su tecla, la camara libre se crea de nuevo en el punto de llegada
		if (mainWindow.getzonaPlaneta() != zonaActual || mainWindow.getreinicioPlaneta() != reinicioPlanetaActual)
		{
			zonaActual = mainWindow.getzonaPlaneta();
			reinicioPlanetaActual = mainWindow.getreinicioPlaneta();
			if (zonaActual == 1)
				camera = Camera(posXen + glm::vec3(0.0f, 1.5f, 8.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.3f, 0.5f);
			else if (zonaActual == 2)
				camera = Camera(posPlaneta2 + glm::vec3(0.0f, 1.5f, 8.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.3f, 0.5f);
			else if (zonaActual == 3)
				camera = Camera(posVerdania + glm::vec3(0.0f, 1.5f, 8.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.3f, 0.5f);
			else
				camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);
			view = camera.calculateViewMatrix();
		}

		posVista = camera.getCameraPosition();
		dirVista = camera.getCameraDirection();

		//en el espacio la Z recorre el inicio, la nave nodriza y la pista, y luego regresa al inicio
		if (mainWindow.getpuntoEspacio() != puntoEspacioActual)
		{
			puntoEspacioActual = mainWindow.getpuntoEspacio();
			int punto = puntoEspacioActual % 3;
			if (punto == 1)
			{
				//del lado de la nave nodriza que da hacia X negativo, para que salga completa y atras la estacion
				camera = Camera(glm::vec3(-250.0f, 120.0f, -40.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, -5.0f, 0.3f, 0.5f);
			}
			else if (punto == 2)
			{
				//atras de la meta, viendo hacia Z negativo
				camera = Camera(posPista + glm::vec3(0.0f, 40.0f, 120.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -15.0f, 0.3f, 0.5f);
			}
			else
			{
				//el mismo lugar donde empieza el programa
				camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);
			}
			view = camera.calculateViewMatrix();
			posVista = camera.getCameraPosition();
			dirVista = camera.getCameraDirection();
		}

		//el avatar se mueve aqui para que la camara 2 use su lugar de este frame
		if (mainWindow.getzonaPlaneta() != zonaAvatar || mainWindow.getreinicioPlaneta() != reinicioAvatar)
		{
			zonaAvatar = mainWindow.getzonaPlaneta();
			reinicioAvatar = mainWindow.getreinicioPlaneta();
			posAvatar = posLlegadaAvatar;
			rumboAvatar = 180.0f;
		}
		//el piso del planeta donde esta el avatar; en el espacio no se dibuja, asi que ahi no importa
		glm::vec3 centroAvatar = posXen;
		if (zonaAvatar == 2)
			centroAvatar = posPlaneta2;
		else if (zonaAvatar == 3)
			centroAvatar = posVerdania;

		//con la camara 2, W y S lo mueven hacia enfrente y hacia atras, y A y D lo giran; Shift lo hace ir al doble de rapido
		//como nunca cambia su altura, siempre camina sobre el piso
		bool avatarCamina = false;
		if (mainWindow.getmodoCamara() == 2)
		{
			bool* teclas = mainWindow.getsKeys();
			float velocidad = velocidadAvatar * deltaReal;
			if (teclas[GLFW_KEY_LEFT_SHIFT])
				velocidad = velocidad * 2.0f;

			if (teclas[GLFW_KEY_A])
				rumboAvatar += giroAvatarOffset * deltaReal;
			if (teclas[GLFW_KEY_D])
				rumboAvatar -= giroAvatarOffset * deltaReal;
			glm::vec3 frenteAvatar = glm::vec3(sin(rumboAvatar * toRadians), 0.0f, cos(rumboAvatar * toRadians));

			//se guarda donde estaba por si el paso lo saca de la isla de Xen
			glm::vec3 posAnteriorAvatar = posAvatar;
			if (teclas[GLFW_KEY_W])
			{
				posAvatar += frenteAvatar * velocidad;
				avatarCamina = true;
			}
			if (teclas[GLFW_KEY_S])
			{
				posAvatar -= frenteAvatar * velocidad;
				avatarCamina = true;
			}

			//no puede salirse del piso
			if (posAvatar.x > limitePisoAvatar)
				posAvatar.x = limitePisoAvatar;
			if (posAvatar.x < -limitePisoAvatar)
				posAvatar.x = -limitePisoAvatar;
			if (posAvatar.z > limitePisoAvatar)
				posAvatar.z = limitePisoAvatar;
			if (posAvatar.z < -limitePisoAvatar)
				posAvatar.z = -limitePisoAvatar;
			//en Xen se compara el cuadrado de la distancia con el de limiteIslaXen, sin sacar raiz
			if (zonaAvatar == 1 && posAvatar.x * posAvatar.x + posAvatar.z * posAvatar.z > limiteIslaXen * limiteIslaXen)
				posAvatar = posAnteriorAvatar;

			glm::vec3 posAvatarMundo = centroAvatar + posAvatar;
			glm::vec3 puntoMira = posAvatarMundo + glm::vec3(0.0f, alturaMiraAvatar, 0.0f);
			posVista = posAvatarMundo - frenteAvatar * distanciaCamaraAvatar + glm::vec3(0.0f, alturaCamaraAvatar, 0.0f);
			dirVista = glm::normalize(puntoMira - posVista);
			view = glm::lookAt(posVista, puntoMira, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		//camara 3, detras de una nave de la carrera; Q y E cambian de nave
		//se le suma lo que la carrera avanza en este frame para que la camara no se quede atras
		if (mainWindow.getmodoCamara() == 3)
		{
			//el resto de dividir entre el numero de naves sale negativo si objetivoCamara es negativo, por eso se le suma una vuelta
			int naveSeguida = mainWindow.getobjetivoCamara() % NUM_NAVES_FZERO;
			if (naveSeguida < 0)
				naveSeguida += NUM_NAVES_FZERO;
			float carrilX = 2.15f;
			if (naveSeguida % 2 == 0)
				carrilX = -5.15f;
			float desfaseAtras = -10.0f * (naveSeguida + 1);
			glm::mat4 matrizNave = modelauxNaves * ObtenerMatrizNavePista(avanceCarrera + velocidadCarrera * deltaTime + desfaseAtras, carrilX, 1.0f);

			//la segunda columna de la matriz es el eje Y de la nave, asi la camara se inclina con la pista
			posVista = glm::vec3(matrizNave * glm::vec4(camaraNaveAtras, 1.0f));
			glm::vec3 puntoMira = glm::vec3(matrizNave * glm::vec4(camaraNaveMira, 1.0f));
			glm::vec3 arribaNave = glm::normalize(glm::vec3(matrizNave[1]));
			dirVista = glm::normalize(puntoMira - posVista);
			view = glm::lookAt(posVista, puntoMira, arribaNave);
		}

		if (mainWindow.getmodoCamara() == 4)
		{
			//G, H y J escogen el viaje; si se presiona otra vez, empieza de nuevo
			int viaje = mainWindow.getviajeCamara();
			if (mainWindow.getreinicioViaje() != reinicioViajeActual)
			{
				reinicioViajeActual = mainWindow.getreinicioViaje();
				ReiniciaViaje();
			}

			//centro del piso del planeta a donde va
			glm::vec3 centroDestino = posXen;
			if (zonaViajes[viaje] == 2)
				centroDestino = posPlaneta2;
			else if (zonaViajes[viaje] == 3)
				centroDestino = posVerdania;

			if (etapaViaje == 0)
			{
				//estacionada en la estacion
				tiempoViaje += deltaTime / 61.0f;
				if (tiempoViaje >= esperaViaje)
					etapaViaje = 1;
			}
			else if (etapaViaje == 1)
			{
				//sube en diagonal
				subidaViaje += velSubidaViaje * deltaTime;
				if (subidaViaje >= subidaViajes[viaje])
				{
					subidaViaje = subidaViajes[viaje];
					etapaViaje = 2;
				}
			}
			else if (etapaViaje == 2)
			{
				//gira hacia el planeta por el lado que le toque
				if (rumboViajes[viaje] > rumboPadViaje)
				{
					rumboViaje += velGiroViaje * deltaTime;
					if (rumboViaje >= rumboViajes[viaje])
					{
						rumboViaje = rumboViajes[viaje];
						etapaViaje = 3;
					}
				}
				else
				{
					rumboViaje -= velGiroViaje * deltaTime;
					if (rumboViaje <= rumboViajes[viaje])
					{
						rumboViaje = rumboViajes[viaje];
						etapaViaje = 3;
					}
				}
			}
			else if (etapaViaje == 3)
			{
				//se prende la estela y vuela; zonaActual cambia aqui para que el skybox sea el del planeta en este frame
				fuegoViaje += 0.03f * deltaTime;
				if (fuegoViaje > 1.0f)
					fuegoViaje = 1.0f;
				avanceViaje += velVueloViaje * deltaTime;
				if (avanceViaje >= distanciaViajes[viaje])
				{
					rumboViaje = rumboLlegadaViajes[viaje];
					etapaViaje = 4;
					mainWindow.setzonaPlaneta(zonaViajes[viaje]);
					zonaActual = zonaViajes[viaje];
				}
			}
			else if (etapaViaje == 4)
			{
				//se acerca derecho hasta quedar arriba de su lugar
				llegadaViaje -= velLlegadaViaje * deltaTime;
				if (llegadaViaje <= 0.0f)
				{
					llegadaViaje = 0.0f;
					etapaViaje = 5;
				}
			}
			else if (etapaViaje == 5)
			{
				//baja derecho y la estela se va apagando conforme se acerca al piso
				bajadaViaje -= velBajadaViaje * deltaTime;
				if (bajadaViaje <= 0.0f)
				{
					bajadaViaje = 0.0f;
					tiempoViaje = 0.0f;
					etapaViaje = 6;
				}
				fuegoViaje = bajadaViaje / alturaPosadoViaje;
			}
			else
			{
				//estacionada en el planeta; al acabar su tiempo regresa a la estacion
				tiempoViaje += deltaTime / 61.0f;
				if (tiempoViaje >= esperaViaje)
				{
					ReiniciaViaje();
					mainWindow.setzonaPlaneta(0);
					zonaActual = 0;
				}
			}

			//la nariz ve hacia Z negativo, asi que el frente sale del seno y el coseno del rumbo
			glm::vec3 frenteNave = glm::vec3(-sin(rumboViaje * toRadians), 0.0f, -cos(rumboViaje * toRadians));

			glm::vec3 posNave;
			if (etapaViaje <= 3)
			{
				//en el espacio se aleja hacia X negativo lo mismo que sube, y luego vuela hacia enfrente
				posNave = posEstacion + padViaje + glm::vec3(-subidaViaje, subidaViaje, 0.0f) + frenteNave * avanceViaje;
				posVista = posEstacion + camaraEstacionViajes[viaje];
			}
			else
			{
				//en el planeta queda atras de su lugar lo que le falta por llegar y arriba lo que le falta por bajar
				posNave = centroDestino + lugarLlegadaViajes[viaje] - frenteNave * llegadaViaje + glm::vec3(0.0f, bajadaViaje, 0.0f);
				posVista = centroDestino + camaraLlegadaViajes[viaje];
			}

			matrizNaveViaje = glm::mat4(1.0f);
			matrizNaveViaje = glm::translate(matrizNaveViaje, posNave);
			matrizNaveViaje = glm::rotate(matrizNaveViaje, rumboViaje * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			matrizNaveViaje = glm::scale(matrizNaveViaje, glm::vec3(escalaNaveViaje, escalaNaveViaje, escalaNaveViaje));
			matrizNaveViaje = glm::translate(matrizNaveViaje, glm::vec3(0.0f, 0.0f, centroNaveViaje));

			//la luz de la estela va atras del motor y brilla mas entre mas grande es la estela
			glm::vec3 posLuz = glm::vec3(matrizNaveViaje * glm::vec4(0.0f, 0.0f, 30.0f, 1.0f));
			luzEstelaViaje = PointLight(0.3f, 0.5f, 1.0f,
				0.0f, 3.0f * fuegoViaje,
				posLuz.x, posLuz.y, posLuz.z,
				1.0f, 0.05f, 0.01f);

			dirVista = glm::normalize(posNave - posVista);
			view = glm::lookAt(posVista, posNave, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		//la camara 5 hace el recorrido del lanzamiento: ve despegar el cohete en Pop Star, lo sigue en el espacio hasta la estacion
		//y luego da una vuelta por la estacion y el satelite; al terminar regresa a Pop Star y empieza otra vez
		//como todo avanza con deltaTime, la P tambien pausa el recorrido
		if (mainWindow.getmodoCamara() == 5)
		{
			//primero cambia la etapa y despues se pone la camara, asi el frame del cambio ya usa la camara nueva
			if (etapaRecorrido >= 1)
				avanceCoheteEspacio += velCoheteEspacio * deltaTime;

			if (etapaRecorrido == 0 && estadoCohete == 3)
			{
				//cuando el cohete deja de subir pasa al espacio; zonaActual cambia aqui para que el skybox sea el del espacio
				estadoCohete = 0;
				alturaCohete = 0.0f;
				fuegoCohete = 0.0f;
				etapaRecorrido = 1;
				avanceCoheteEspacio = 0.0f;
				mainWindow.setzonaPlaneta(0);
				zonaActual = 0;
			}
			else if (etapaRecorrido == 1 && avanceCoheteEspacio >= largoCaminoCohete)
			{
				etapaRecorrido = 2;
				tiempoEtapa = 0.0f;
			}
			else if (etapaRecorrido == 2)
			{
				tiempoEtapa += deltaTime / 61.0f;
				if (tiempoEtapa >= tiempoAlejamiento)
				{
					etapaRecorrido = 3;
					anguloVuelta = anguloInicioVuelta;
				}
			}
			else if (etapaRecorrido == 3)
			{
				anguloVuelta += velVuelta * deltaTime;
				if (anguloVuelta >= anguloInicioVuelta + 360.0f)
				{
					ReiniciaRecorrido();
					mainWindow.setzonaPlaneta(2);
					zonaActual = 2;
				}
			}

			glm::vec3 puntoMira = posVista;
			if (etapaRecorrido == 0)
			{
				//la camara sube con el cohete a la mitad de su velocidad para no perderlo
				glm::vec3 posPlat = posPlaneta2 + posPlataforma;
				puntoMira = posPlat + glm::vec3(0.0f, (2.4f + alturaCohete) * escalaPlataforma, 0.0f);
				posVista = posPlat + camaraPlataforma + glm::vec3(0.0f, alturaCohete * escalaPlataforma * 0.5f, 0.0f);
			}
			else if (etapaRecorrido == 1 || etapaRecorrido == 2)
			{
				//en la etapa 2 la camara se queda en finCoheteEspacio y solo mira al cohete
				glm::vec3 posCoheteEspacio = inicioCoheteEspacio + frenteCoheteEspacio * avanceCoheteEspacio;
				glm::vec3 seguirCohete = posCoheteEspacio;
				if (etapaRecorrido == 2)
					seguirCohete = finCoheteEspacio;
				posVista = seguirCohete - frenteCoheteEspacio * distanciaCamaraCohete + ladoCoheteEspacio * ladoCamaraCohete + glm::vec3(0.0f, alturaCamaraCohete, 0.0f);
				puntoMira = posCoheteEspacio + frenteCoheteEspacio * 9.0f;
			}
			else
			{
				posVista = centroVuelta + glm::vec3(radioVuelta * cos(anguloVuelta * toRadians), alturaVuelta, radioVuelta * sin(anguloVuelta * toRadians));
				puntoMira = centroVuelta;
			}
			dirVista = glm::normalize(puntoMira - posVista);
			view = glm::lookAt(posVista, puntoMira, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		//limpiar la pantalla
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//el skybox va primero; zonaActual cambia con Z, X, C y V
		//el del espacio gira en los tres ejes; los de los planetas solo sobre Y para que el horizonte no se incline
		if (zonaActual == 1 && esDeNoche)
			skyboxXenNoche.DrawSkybox(view, projection, deltaTime, false);
		else if (zonaActual == 1)
			skyboxXenDia.DrawSkybox(view, projection, deltaTime, false);
		else if (zonaActual == 2 && esDeNoche)
			skyboxPlaneta2Noche.DrawSkybox(view, projection, deltaTime, false);
		else if (zonaActual == 2)
			skyboxPlaneta2Dia.DrawSkybox(view, projection, deltaTime, false);
		else if (zonaActual == 3 && esDeNoche)
			skyboxVerdaniaNoche.DrawSkybox(view, projection, deltaTime, false);
		else if (zonaActual == 3)
			skyboxVerdaniaDia.DrawSkybox(view, projection, deltaTime, false);
		else
			skybox.DrawSkybox(view, projection, deltaTime, true);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation();
		//las texturas van quietas; solo el charco de Xen cambia el offset y al terminar lo regresa a 0
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
		glUniform3f(uniformEyePosition, posVista.x, posVista.y, posVista.z);

		//linterna pegada a la camara que se este usando, un poco abajo de los ojos
		glm::vec3 lowerLight = posVista;
		lowerLight.y -= 0.3f;
		linternaLuz.SetFlash(lowerLight, dirVista);

		//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---
		int zonaLuces = mainWindow.getzonaPlaneta();
		if (zonaLuces == 0)
		{
			haciaSol = haciaSolEspacio;
			esDeNoche = false;
			mainLight = DirectionalLight(1.0f, 0.95f, 0.85f,
				0.3f, 0.7f,
				-haciaSol.x, -haciaSol.y, -haciaSol.z);
		}
		else
		{
			if (mainWindow.getsaltoDiaNoche() != saltoDiaNocheActual)
			{
				saltoDiaNocheActual = mainWindow.getsaltoDiaNoche();
				if (esDeNoche)
				{
					anguloSol = anguloManana;
					printf("ahora es de dia\n");
				}
				else
				{
					anguloSol = anguloNoche;
					printf("ahora es de noche\n");
				}
			}
			//en los planetas el sol sale por X positivo y se mete por X negativo
			anguloSol += anguloSolOffset * deltaTime;
			if (anguloSol > 360.0f)
				anguloSol -= 360.0f;
			float alturaSol = sin(anguloSol * toRadians);
			haciaSol = glm::normalize(glm::vec3(cos(anguloSol * toRadians), alturaSol, -0.3f));
			esDeNoche = alturaSol < 0.0f;
			if (!esDeNoche)
			{
				//de dia la luz baja del sol; entre mas alto esta, mas fuerte alumbra
				//cerca del horizonte se ve naranja porque el verde y el azul dependen de la altura
				mainLight = DirectionalLight(1.0f, 0.6f + 0.4f * alturaSol, 0.4f + 0.5f * alturaSol,
					0.25f + 0.15f * alturaSol, 0.2f + 0.6f * alturaSol,
					-haciaSol.x, -haciaSol.y, -haciaSol.z);
			}
			else
			{
				//de noche alumbra la luna, que esta del lado contrario al sol, con poca luz y azulada
				mainLight = DirectionalLight(0.65f, 0.75f, 1.0f,
					0.15f, 0.2f,
					haciaSol.x, haciaSol.y, haciaSol.z);
			}
		}
		//el sol se mueve con la camara para que parezca estar en el infinito
		posSol = posVista + haciaSol * distanciaSol;

		pointLightCount = 0;
		spotLightCount = 0;
		if (mainWindow.getlinterna())
		{
			spotLights[spotLightCount] = linternaLuz;
			spotLightCount++;
		}
		if (zonaLuces == 0)
		{
			for (int i = 0; i < NUM_LUMINARIAS_ESPACIO; i++)
			{
				pointLights[pointLightCount] = luminariasEspacio[i];
				pointLightCount++;
			}
			if (mainWindow.getlucesEscenario())
			{
				for (int i = 0; i < NUM_SPOTS_ESPACIO; i++)
				{
					spotLights[spotLightCount] = spotsEspacio[i];
					spotLightCount++;
				}
			}
		}
		else if (zonaLuces == 1)
		{
			if (esDeNoche)
			{
				for (int i = 0; i < NUM_LUMINARIAS_XEN; i++)
				{
					pointLights[pointLightCount] = luminariasXen[i];
					pointLightCount++;
				}
			}
			//la luz del vortigaunt va enfrente de sus manos y su intensidad sube con la carga, de dia y de noche
			glm::vec3 manosVortigaunt = posXen + posVortigaunt + glm::vec3(0.0f, 0.8f, 0.8f);
			luzVortigaunt = PointLight(0.3f, 1.0f, 0.4f,
				0.0f, 3.0f * cargaVortigaunt,
				manosVortigaunt.x, manosVortigaunt.y, manosVortigaunt.z,
				1.0f, 0.3f, 0.15f);
			pointLights[pointLightCount] = luzVortigaunt;
			pointLightCount++;
			if (mainWindow.getlucesEscenario())
			{
				spotLights[spotLightCount] = spotXen;
				spotLightCount++;
			}
		}
		else if (zonaLuces == 2)
		{
			if (esDeNoche)
			{
				pointLights[pointLightCount] = luminariaPlaneta2;
				pointLightCount++;
			}
			if (mainWindow.getlucesEscenario())
			{
				spotLights[spotLightCount] = spotPlaneta2;
				spotLightCount++;
			}
		}
		else if (zonaLuces == 3)
		{
			if (esDeNoche)
			{
				pointLights[pointLightCount] = luminariaVerdania;
				pointLightCount++;
			}
			if (mainWindow.getlucesEscenario())
			{
				spotLights[spotLightCount] = spotVerdania;
				spotLightCount++;
			}
		}
		//la estela de la nave de la camara 4 lleva su luz en cualquier zona mientras esta prendida
		if (mainWindow.getmodoCamara() == 4 && fuegoViaje > 0.0f)
		{
			pointLights[pointLightCount] = luzEstelaViaje;
			pointLightCount++;
		}

		//mandar todas las luces al shader
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		glm::mat4 model(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); //con blanco la textura se ve con sus colores normales

		// ---------- objetos opacos ----------
		//todo empieza con color blanco y material opaco, y lo que usa otro material regresa el opaco al terminar
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//--- personajes ---

		//--- planetas: Xen y planeta 2 ---
		//Xen
		//la esfera de radio 1 con escala 660 queda de radio 660
		giroPlanetaXen += giroPlanetaXenOffset * deltaTime;
		if (giroPlanetaXen > 360.0f)
			giroPlanetaXen -= 360.0f;
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlanetaXen);
		model = glm::rotate(model, giroPlanetaXen * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(660.0f, 660.0f, 660.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		XenPlaneta_M.RenderModel();

		//la cara de arriba del suelo queda en y 0 y su orilla entre 36 y 50 del centro
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		XenSuelo_M.RenderModel();

		//criaturas de Xen
		anguloXen += anguloXenOffset * deltaTime;
		if (anguloXen > 360.0f)
			anguloXen -= 360.0f;

		if (estadoHoundeye == 0)
		{
			anguloHoundeyes += anguloHoundeyesOffset * deltaTime;
			if (anguloHoundeyes > 360.0f)
				anguloHoundeyes -= 360.0f;
			tiempoHoundeye += deltaTime;
			if (tiempoHoundeye > tiempoCaminaHoundeye)
			{
				tiempoHoundeye = 0.0f;
				estadoHoundeye = 1;
			}
		}
		else if (estadoHoundeye == 1)
		{
			agacheHoundeye += bajaHoundeye * deltaTime;
			if (agacheHoundeye > 1.0f)
			{
				agacheHoundeye = 1.0f;
				estadoHoundeye = 2;
			}
		}
		else
		{
			agacheHoundeye -= subeHoundeye * deltaTime;
			if (agacheHoundeye < 0.0f)
			{
				agacheHoundeye = 0.0f;
				estadoHoundeye = 0;
			}
		}
		for (int i = 0; i < NUM_HOUNDEYES; i++)
		{
			//las patas de adelante van juntas y al reves de la de atras; cada houndeye va desfasado
			float pasoHoundeye = 0.0f;
			if (estadoHoundeye == 0)
				pasoHoundeye = sin((anguloXen * 5.0f + i * 120.0f) * toRadians);
			rotHoundeye[1].x = 20.0f * pasoHoundeye;
			rotHoundeye[2].x = 20.0f * pasoHoundeye;
			rotHoundeye[3].x = -20.0f * pasoHoundeye;
			rotHoundeye[1].z = -35.0f * agacheHoundeye; //pata_del_der
			rotHoundeye[2].z = 35.0f * agacheHoundeye;  //pata_del_izq
			rotHoundeye[3].x += 35.0f * agacheHoundeye; //pata_tras
			//con el valor absoluto del seno el cuerpo da saltitos sin meterse al piso; agachado baja lo que suben las patas al abrirse
			float saltoHoundeye = 0.06f * fabs(pasoHoundeye) - 0.05f * agacheHoundeye;
			float angulo = anguloHoundeyes + i * 120.0f;
			model = glm::mat4(1.0);
			model = glm::translate(model, posXen + centroHoundeyes +
				glm::vec3(radioHoundeyes * sin(angulo * toRadians), saltoHoundeye, radioHoundeyes * cos(angulo * toRadians)));
			model = glm::rotate(model, (angulo + 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			DibujaJerarquia(Houndeye_M, padresHoundeye, pivotesHoundeye, rotHoundeye, NUM_PIEZAS_HOUNDEYE, model, uniformModel);
		}

		if (estadoBullsquid == 0)
		{
			esperaBullsquid += deltaTime;
			if (esperaBullsquid > tiempoEsperaBullsquid)
			{
				esperaBullsquid = 0.0f;
				estadoBullsquid = 1;
			}
		}
		else if (estadoBullsquid == 1)
		{
			inclinaBullsquid -= 0.3f * deltaTime;
			if (inclinaBullsquid < atrasBullsquid)
				estadoBullsquid = 2;
		}
		else if (estadoBullsquid == 2)
		{
			inclinaBullsquid += 2.5f * deltaTime;
			if (inclinaBullsquid > adelanteBullsquid)
			{
				inclinaBullsquid = adelanteBullsquid;
				estadoBullsquid = 3;
			}
		}
		else
		{
			inclinaBullsquid -= 0.3f * deltaTime;
			if (inclinaBullsquid < 0.0f)
			{
				inclinaBullsquid = 0.0f;
				estadoBullsquid = 0;
			}
		}
		//las patas giran lo contrario que el cuerpo sobre la misma linea de la cadera, asi se quedan como estaban
		rotBullsquid[0].x = inclinaBullsquid;  //cuerpo
		rotBullsquid[3].x = -inclinaBullsquid;  //pata_der
		rotBullsquid[4].x = -inclinaBullsquid;  //pata_izq
		rotBullsquid[2].y = 20.0f * sin(anguloXen * 2.0f * toRadians); //cola
		rotBullsquid[2].x = 5.0f * sin(anguloXen * 4.0f * toRadians);
		//los tentaculos se mueven mas hacia arriba que hacia abajo para que no toquen el piso
		rotBullsquid[1].x = -4.0f + 6.0f * sin(anguloXen * 6.0f * toRadians); //tentaculos
		rotBullsquid[1].y = 6.0f * sin(anguloXen * 3.0f * toRadians);
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen + glm::vec3(-6.0f, 0.0f, -12.0f));
		DibujaJerarquia(Bullsquid_M, padresBullsquid, pivotesBullsquid, rotBullsquid, NUM_PIEZAS_BULLSQUID, model, uniformModel);

		if (estadoVortigaunt == 0)
		{
			cargaVortigaunt += 0.006f * deltaTime;
			if (cargaVortigaunt > 1.0f)
			{
				cargaVortigaunt = 1.0f;
				estadoVortigaunt = 1;
			}
		}
		else if (estadoVortigaunt == 1)
		{
			cargaVortigaunt -= 0.08f * deltaTime;
			if (cargaVortigaunt < 0.0f)
			{
				cargaVortigaunt = 0.0f;
				esperaVortigaunt = 0.0f;
				estadoVortigaunt = 2;
			}
		}
		else
		{
			esperaVortigaunt += deltaTime;
			if (esperaVortigaunt > tiempoEsperaVortigaunt)
				estadoVortigaunt = 0;
		}
		if (estadoVortigaunt == 2)
		{
			if (cabezaVortigauntIzq)
			{
				cabezaVortigaunt += cabezaVortigauntOffset * deltaTime;
				if (cabezaVortigaunt > limiteCabezaVortigaunt)
					cabezaVortigauntIzq = false;
			}
			else
			{
				cabezaVortigaunt -= cabezaVortigauntOffset * deltaTime;
				if (cabezaVortigaunt < -limiteCabezaVortigaunt)
					cabezaVortigauntIzq = true;
			}
		}
		else if (cabezaVortigaunt > 0.0f)
		{
			cabezaVortigaunt -= cabezaVortigauntOffset * deltaTime;
			if (cabezaVortigaunt < 0.0f)
				cabezaVortigaunt = 0.0f;
		}
		else if (cabezaVortigaunt < 0.0f)
		{
			cabezaVortigaunt += cabezaVortigauntOffset * deltaTime;
			if (cabezaVortigaunt > 0.0f)
				cabezaVortigaunt = 0.0f;
		}
		float meceVortigaunt = 3.0f * sin(anguloXen * 2.0f * toRadians);
		rotVortigaunt[2].x = -45.0f * cargaVortigaunt + meceVortigaunt; //brazo_der
		rotVortigaunt[2].z = -20.0f * cargaVortigaunt;
		rotVortigaunt[3].x = -45.0f * cargaVortigaunt - meceVortigaunt; //brazo_izq
		rotVortigaunt[3].z = 20.0f * cargaVortigaunt;
		rotVortigaunt[1].y = cabezaVortigaunt; //cabeza
		rotVortigaunt[1].x = -15.0f * cargaVortigaunt;
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen + posVortigaunt);
		DibujaJerarquia(Vortigaunt_M, padresVortigaunt, pivotesVortigaunt, rotVortigaunt, NUM_PIEZAS_VORTIGAUNT, model, uniformModel);

		if (estadoGrunt == 0)
		{
			if (gruntAdelante)
			{
				posGrunt += posGruntOffset * deltaTime;
				if (posGrunt > limiteGrunt)
				{
					posGrunt = limiteGrunt;
					estadoGrunt = 1;
				}
			}
			else
			{
				posGrunt -= posGruntOffset * deltaTime;
				if (posGrunt < -limiteGrunt)
				{
					posGrunt = -limiteGrunt;
					estadoGrunt = 1;
				}
			}
		}
		else if (estadoGrunt == 1)
		{
			apuntaGrunt += 0.03f * deltaTime;
			if (apuntaGrunt > 1.0f)
			{
				apuntaGrunt = 1.0f;
				esperaGrunt = 0.0f;
				estadoGrunt = 2;
			}
		}
		else if (estadoGrunt == 2)
		{
			esperaGrunt += deltaTime;
			if (esperaGrunt > tiempoApuntaGrunt)
				estadoGrunt = 3;
		}
		else
		{
			apuntaGrunt -= 0.03f * deltaTime;
			if (apuntaGrunt < 0.0f)
			{
				apuntaGrunt = 0.0f;
				gruntAdelante = !gruntAdelante;
				estadoGrunt = 0;
			}
		}
		//el rumbo cambia poco a poco, como el del Bandana Dee
		float rumboDeseado = 0.0f;
		if (estadoGrunt == 0)
		{
			if (gruntAdelante)
				rumboDeseado = 90.0f;
			else
				rumboDeseado = -90.0f;
		}
		if (rumboGrunt < rumboDeseado)
		{
			rumboGrunt += rumboGruntOffset * deltaTime;
			if (rumboGrunt > rumboDeseado)
				rumboGrunt = rumboDeseado;
		}
		if (rumboGrunt > rumboDeseado)
		{
			rumboGrunt -= rumboGruntOffset * deltaTime;
			if (rumboGrunt < rumboDeseado)
				rumboGrunt = rumboDeseado;
		}
		float pasoGrunt = 0.0f;
		if (estadoGrunt == 0)
			pasoGrunt = sin(anguloXen * 4.0f * toRadians);
		rotGrunt[4].x = 25.0f * pasoGrunt;  //pierna_der
		rotGrunt[5].x = -25.0f * pasoGrunt;  //pierna_izq
		rotGrunt[2].x = -15.0f * pasoGrunt;  //brazo_izq
		rotGrunt[1].x = 10.0f * pasoGrunt - 35.0f * apuntaGrunt;  //arma
		rotGrunt[3].x = 8.0f * sin(anguloXen * 3.0f * toRadians);  //brazo_chico
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen + centroGrunt + glm::vec3(posGrunt, 0.0f, 0.0f));
		model = glm::rotate(model, rumboGrunt * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(AlienGrunt_M, padresGrunt, pivotesGrunt, rotGrunt, NUM_PIEZAS_GRUNT, model, uniformModel);

		//el offset de UV del charco avanza cada frame; al pasar de 1 se le resta 1 porque la textura se repite
		charcoU += 0.0015f * deltaTime;
		charcoV += 0.0008f * deltaTime;
		if (charcoU > 1.0f)
			charcoU -= 1.0f;
		if (charcoV > 1.0f)
			charcoV -= 1.0f;
		toffset = glm::vec2(charcoU, charcoV);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen + posCharco);
		model = glm::scale(model, glm::vec3(2.5f, 1.0f, 1.8f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		XenCharco_M.RenderModel();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		//cristales, todos con el mismo modelo
		for (int i = 0; i < NUM_CRISTALES; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posXen + posCristales[i]);
			model = glm::rotate(model, giroCristales[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaCristales[i], escalaCristales[i], escalaCristales[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			XenCristal_M.RenderModel();
		}

		//modelaux guarda la matriz antes de escalar el tallo, para que el foco cuelgue de la punta
		tiempoPlantas += deltaTime;
		if (plantasEscondidas && tiempoPlantas > tiempoEscondidas)
		{
			plantasEscondidas = false;
			tiempoPlantas = 0.0f;
		}
		else if (!plantasEscondidas && tiempoPlantas > tiempoParadas)
		{
			plantasEscondidas = true;
			tiempoPlantas = 0.0f;
		}
		for (int i = 0; i < NUM_PLANTAS; i++)
		{
			glm::vec3 pos = posXen + posPlantas[i];
			if (plantasEscondidas)
			{
				if (alturaPlantas[i] > 0.1f)
					alturaPlantas[i] -= 0.05f * deltaTime;
			}
			else if (alturaPlantas[i] < 1.0f)
			{
				alturaPlantas[i] += 0.01f * deltaTime;
			}
			//se deja entre 0.1 y 1 porque con una escala negativa el tallo se voltearia
			if (alturaPlantas[i] < 0.1f)
				alturaPlantas[i] = 0.1f;
			if (alturaPlantas[i] > 1.0f)
				alturaPlantas[i] = 1.0f;
			model = glm::mat4(1.0);
			model = glm::translate(model, pos);
			glm::mat4 modelaux = model;
			model = glm::scale(model, glm::vec3(1.0f, alturaPlantas[i], 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			XenTallo_M.RenderModel();
			//el foco baja lo mismo que la punta del tallo
			model = modelaux;
			model = glm::translate(model, glm::vec3(0.0f, altoTallo * (alturaPlantas[i] - 1.0f), 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			XenFoco_M.RenderModel();
		}

		//cada isla sube y baja desfasada 120 grados; lo que lleva encima usa la matriz sin escala
		anguloIslas += anguloIslasOffset * deltaTime;
		if (anguloIslas > 360.0f)
			anguloIslas -= 360.0f;
		for (int i = 0; i < NUM_ISLAS; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posXen + posIslas[i] +
				glm::vec3(0.0f, 0.8f * sin((anguloIslas + i * 120.0f) * toRadians), 0.0f));
			model = glm::rotate(model, i * 50.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 modelaux = model;
			model = glm::scale(model, glm::vec3(escalaIslas[i], escalaIslas[i], escalaIslas[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			XenIsla_M.RenderModel();
			if (cosaIsla[i] != 0)
			{
				model = glm::translate(modelaux, glm::vec3(0.0f, 0.12f * escalaIslas[i], 0.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				if (cosaIsla[i] == 1)
				{
					model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
					XenCristal_M.RenderModel();
				}
				else
				{
					XenTallo_M.RenderModel();
					XenFoco_M.RenderModel();
				}
			}
		}

		for (int i = 0; i < NUM_ARBOLES_XEN; i++)
		{
			if (estadoArbol[i] == 0)
			{
				esperaArbol[i] += deltaTime;
				if (esperaArbol[i] > tiempoEsperaArbol)
					estadoArbol[i] = 1;
			}
			else if (estadoArbol[i] == 1)
			{
				golpeArbol[i] -= subeArbol * deltaTime;
				if (golpeArbol[i] < arribaArbol)
					estadoArbol[i] = 2;
			}
			else
			{
				golpeArbol[i] += bajaArbol * deltaTime;
				if (golpeArbol[i] > pisoArbol)
				{
					golpeArbol[i] = pisoArbol;
					esperaArbol[i] = 0.0f;
					estadoArbol[i] = 0;
				}
			}
			rotArbolXen[1].x = golpeArbol[i]; //brazo
			model = glm::mat4(1.0);
			model = glm::translate(model, posXen + posArbolesXen[i]);
			model = glm::rotate(model, giroArbolesXen[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaArbolesXen[i], escalaArbolesXen[i], escalaArbolesXen[i]));
			DibujaJerarquia(ArbolXen_M, padresArbolXen, pivotesArbolXen, rotArbolXen, NUM_PIEZAS_ARBOL_XEN, model, uniformModel);
		}

		//la punta usa el mismo seno 40 grados atrasado para que el ala se doble al aletear
		anguloBoids += anguloBoidsOffset * deltaTime;
		if (anguloBoids > 360.0f)
			anguloBoids -= 360.0f;
		for (int i = 0; i < NUM_BOIDS; i++)
		{
			float aleteo = sin((anguloXen * 4.0f + i * 70.0f) * toRadians);
			float aleteoPunta = sin((anguloXen * 4.0f + i * 70.0f - 40.0f) * toRadians);
			rotBoid[0].z = -30.0f * aleteo;  //ala_der
			rotBoid[1].z = -20.0f * aleteoPunta; //punta_der
			rotBoid[2].z = 30.0f * aleteo;  //ala_izq
			rotBoid[3].z = 20.0f * aleteoPunta;  //punta_izq
			//van separados 72 grados en el circulo y los nones vuelan 4 unidades mas alto
			float angulo = anguloBoids + i * 72.0f;
			model = glm::mat4(1.0);
			model = glm::translate(model, posXen + centroBoids +
				glm::vec3(radioBoids * sin(angulo * toRadians), (i % 2) * 4.0f, radioBoids * cos(angulo * toRadians)));
			model = glm::rotate(model, (angulo + 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.35f, 0.35f, 0.35f));
			DibujaJerarquia(Boid_M, padresBoid, pivotesBoid, rotBoid, NUM_PIEZAS_BOID, model, uniformModel);
		}

		//el Nihilanth mide 1 de alto, y con escala 45 queda gigante
		giroNihilanth += giroNihilanthOffset * deltaTime;
		if (giroNihilanth > 360.0f)
			giroNihilanth -= 360.0f;
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen + posNihilanth + glm::vec3(0.0f, 6.0f * sin(anguloXen * toRadians), 0.0f));
		model = glm::rotate(model, 25.0f * sin(giroNihilanth * toRadians) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(45.0f, 45.0f, 45.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nihilanth_M.RenderModel();

		if (mainWindow.getcaminaHeadcrab())
		{
			anguloHeadcrab += anguloHeadcrabOffset * deltaTime;
			if (anguloHeadcrab > 360.0f)
				anguloHeadcrab -= 360.0f;
			if (pasoAdelante)
			{
				pasoHeadcrab += pasoHeadcrabOffset * deltaTime;
				if (pasoHeadcrab > limitePaso)
					pasoAdelante = false;
			}
			else
			{
				pasoHeadcrab -= pasoHeadcrabOffset * deltaTime;
				if (pasoHeadcrab < -limitePaso)
					pasoAdelante = true;
			}
		}
		//las patas se mueven en diagonal, la delantera derecha con la trasera izquierda
		rotHeadcrab[2].y = pasoHeadcrab;
		rotHeadcrab[8].y = pasoHeadcrab;
		rotHeadcrab[4].y = -pasoHeadcrab;
		rotHeadcrab[6].y = -pasoHeadcrab;
		//la garra se levanta un poco mientras su pata va hacia adelante
		rotHeadcrab[3].x = -pasoHeadcrab * 0.5f;
		rotHeadcrab[9].x = -pasoHeadcrab * 0.5f;
		rotHeadcrab[5].x = pasoHeadcrab * 0.5f;
		rotHeadcrab[7].x = pasoHeadcrab * 0.5f;
		//la posicion en el circulo sale del seno y el coseno, y se gira 90 grados mas para que mire hacia donde avanza
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen + centroHeadcrab +
			glm::vec3(radioHeadcrab * sin(anguloHeadcrab * toRadians), 0.0f, radioHeadcrab * cos(anguloHeadcrab * toRadians)));
		model = glm::rotate(model, (anguloHeadcrab + 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(Headcrab_M, padresHeadcrab, pivotesHeadcrab, rotHeadcrab, NUM_PIEZAS_HEADCRAB, model, uniformModel);

		//bacteriofago
		anguloBacteriofago += anguloBacteriofagoOffset * deltaTime;
		if (anguloBacteriofago > 360.0f)
			anguloBacteriofago -= 360.0f;
		//cada pata va desfasada 90 grados de la anterior; se balancea hacia adelante y hacia atras y se levanta al avanzar
		for (int i = 1; i < NUM_PIEZAS_BACTERIOFAGO; i++)
		{
			float fasePata = (anguloXen * 2.0f + i * 90.0f) * toRadians;
			float avancePata = sin(fasePata);
			float levantaPata = 0.5f * (1.0f + cos(fasePata));
			rotBacteriofago[i].x = 12.0f * avancePata;
			rotBacteriofago[i].y = 8.0f * avancePata;
			if (i % 2 == 0)
				rotBacteriofago[i].z = 6.0f * (levantaPata - 0.5f);
			else
				rotBacteriofago[i].z = -6.0f * (levantaPata - 0.5f);
		}
		//el cuerpo sube un poco y se ladea mientras camina
		model = glm::mat4(1.0);
		model = glm::translate(model, posXen + centroBacteriofago +
			glm::vec3(radioBacteriofago * sin(anguloBacteriofago * toRadians), 0.025f * sin(anguloXen * 2.0f * toRadians),
				radioBacteriofago * cos(anguloBacteriofago * toRadians)));
		model = glm::rotate(model, (anguloBacteriofago + 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 2.0f * cos(anguloXen * 2.0f * toRadians) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(escalaBacteriofago, escalaBacteriofago, escalaBacteriofago));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		DibujaJerarquia(Bacteriofago_M, padresBacteriofago, pivotesBacteriofago, rotBacteriofago, NUM_PIEZAS_BACTERIOFAGO, model, uniformModel);
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//planeta 2
		//Pop Star mide como 1.8 de la punta al centro, asi que con escala 540 cada punta sale unas 970 unidades
		//se gira 120 grados en Y para que vea al centro de la escena, y despues gira en Z como rehilete
		giroPlaneta2 += giroPlaneta2Offset * deltaTime;
		if (giroPlaneta2 > 360.0f)
			giroPlaneta2 -= 360.0f;
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2Espacio);
		model = glm::rotate(model, 120.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, giroPlaneta2 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(540.0f, 540.0f, 540.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Planeta2Espacio_M.RenderModel();

		//el piso base mide 20 por 20 y su textura se repite 10 veces, asi que con escala 10 el piso del planeta 2 queda de 200 por 200
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2);
		model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		planeta2SueloTexture.UseTexture();
		meshListModel[0]->RenderMeshModel();

		for (int i = 0; i < NUM_COLINAS; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posPlaneta2 + posColinas[i]);
			model = glm::rotate(model, giroColinas[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaColinas[i], escalaColinas[i], escalaColinas[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Planeta2Colina_M.RenderModel();
		}

		for (int i = 0; i < NUM_ARBOLES; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posPlaneta2 + posArboles[i]);
			model = glm::rotate(model, giroArboles[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaArboles[i], escalaArboles[i], escalaArboles[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Planeta2Arbol_M.RenderModel();
		}

		for (int i = 0; i < NUM_NUBES; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posPlaneta2 + posNubes[i]);
			model = glm::scale(model, glm::vec3(escalaNubes[i], escalaNubes[i], escalaNubes[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (tipoNubes[i] == 1)
				Planeta2Nube1_M.RenderModel();
			else
				Planeta2Nube2_M.RenderModel();
		}

		//la punta de la warp star ve hacia Z positivo, se gira un poco para que se vean los propulsores
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2 + posWarpStar);
		model = glm::rotate(model, -20.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		WarpStar_M.RenderModel();

		//habitantes de Pop Star, del mas simple al mas evolucionado
		anguloHabitantes += anguloHabitantesOffset * deltaTime;
		if (anguloHabitantes > 360.0f)
			anguloHabitantes -= 360.0f;

		//el Waddle Dee camina con los pies en X al reves uno del otro, y cada mano va al reves del pie de su lado
		anguloWaddleDee += anguloWaddleDeeOffset * deltaTime;
		if (anguloWaddleDee > 360.0f)
			anguloWaddleDee -= 360.0f;
		float pasoDee = sin(anguloHabitantes * 4.0f * toRadians);
		rotWaddleDee[3].x = 30.0f * pasoDee;  //pie_der
		rotWaddleDee[4].x = -30.0f * pasoDee; //pie_izq
		rotWaddleDee[1].x = -25.0f * pasoDee; //mano_der
		rotWaddleDee[2].x = 25.0f * pasoDee;  //mano_izq
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2 + centroWaddleDee +
			glm::vec3(radioWaddleDee * sin(anguloWaddleDee * toRadians), 0.0f, radioWaddleDee * cos(anguloWaddleDee * toRadians)));
		model = glm::rotate(model, (anguloWaddleDee + 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(WaddleDee_M, padresWaddle, pivotesWaddleDee, rotWaddleDee, NUM_PIEZAS_WADDLE, model, uniformModel);

		//las manos del Waddle Doo van al reves con el mismo valor en Z, porque estan de lados contrarios
		rotWaddleDoo[1].z = 30.0f * sin(anguloHabitantes * 3.0f * toRadians);
		rotWaddleDoo[2].z = 30.0f * sin(anguloHabitantes * 3.0f * toRadians);
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2 + glm::vec3(-6.5f, 0.0f, -8.5f));
		model = glm::rotate(model, (10.0f + 35.0f * sin(anguloHabitantes * 0.5f * toRadians)) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(WaddleDoo_M, padresWaddle, pivotesWaddleDoo, rotWaddleDoo, NUM_PIEZAS_WADDLE, model, uniformModel);

		if (bandanaAdelante)
		{
			posBandana += posBandanaOffset * deltaTime;
			if (posBandana > limiteBandana)
				bandanaAdelante = false;
		}
		else
		{
			posBandana -= posBandanaOffset * deltaTime;
			if (posBandana < -limiteBandana)
				bandanaAdelante = true;
		}
		if (bandanaAdelante && rumboBandana < 90.0f)
		{
			rumboBandana += rumboBandanaOffset * deltaTime;
			if (rumboBandana > 90.0f)
				rumboBandana = 90.0f;
		}
		if (!bandanaAdelante && rumboBandana > -90.0f)
		{
			rumboBandana -= rumboBandanaOffset * deltaTime;
			if (rumboBandana < -90.0f)
				rumboBandana = -90.0f;
		}
		//la lanza viene acostada y se gira en X para dejarla parada; la mano no se mueve para no despegarla
		float pasoBandana = sin(anguloHabitantes * 3.0f * toRadians);
		rotBandana[2].x = -90.0f;  //lanza
		rotBandana[4].x = 30.0f * pasoBandana;  //pie_der
		rotBandana[5].x = -30.0f * pasoBandana;  //pie_izq
		rotBandana[3].x = 25.0f * pasoBandana;  //mano_izq
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2 + glm::vec3(-3.0f + posBandana, 0.0f, -6.0f));
		model = glm::rotate(model, rumboBandana * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(BandanaDee_M, padresBandana, pivotesBandana, rotBandana, NUM_PIEZAS_BANDANA, model, uniformModel);

		if (estadoDedede == 0)
		{
			golpeDedede -= subeDedede * deltaTime;
			if (golpeDedede < arribaDedede)
				estadoDedede = 1;
		}
		else if (estadoDedede == 1)
		{
			golpeDedede += bajaDedede * deltaTime;
			if (golpeDedede > pisoDedede)
			{
				golpeDedede = pisoDedede;
				esperaDedede = 0.0f;
				estadoDedede = 2;
			}
		}
		else
		{
			esperaDedede += deltaTime;
			if (esperaDedede > tiempoEsperaDedede)
				estadoDedede = 0;
		}
		rotDedede[1].x = golpeDedede; //brazo_der
		//el martillo viene con el barril acostado; girado 90 grados en Z pega en el piso con una cara
		rotDedede[3].z = 90.0f;  //martillo
		//el brazo izquierdo viene extendido; se baja 40 grados en Z y se mece un poco
		rotDedede[4].z = -40.0f + 5.0f * sin(anguloHabitantes * toRadians); //brazo_izq
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2 + glm::vec3(1.0f, 0.0f, -12.0f));
		model = glm::rotate(model, -15.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(KingDedede_M, padresDedede, pivotesDedede, rotDedede, NUM_PIEZAS_DEDEDE, model, uniformModel);

		//se le suman 90 grados al giro para que la nariz del Halberd vea hacia donde avanza
		anguloHalberd += anguloHalberdOffset * deltaTime;
		if (anguloHalberd > 360.0f)
			anguloHalberd -= 360.0f;
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlaneta2 + centroHalberd +
			glm::vec3(radioHalberd * sin(anguloHalberd * toRadians), 0.0f, radioHalberd * cos(anguloHalberd * toRadians)));
		model = glm::rotate(model, (anguloHalberd + 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Halberd_M.RenderModel();

		//--- planeta 3: Verdania ---
		//el seno de anguloVerdania usa multiplicadores enteros para que no brinque al pasar de 360 a 0
		if (mainWindow.getanimaVerdania())
		{
			anguloVerdania += anguloVerdaniaOffset * deltaTime;
			if (anguloVerdania > 360.0f)
				anguloVerdania -= 360.0f;
			anguloPeashooter += anguloPeashooterOffset * deltaTime;
			if (anguloPeashooter > 360.0f)
				anguloPeashooter -= 360.0f;
			if (daveAdelante)
			{
				posDave += posDaveOffset * deltaTime;
				if (posDave > limiteDave)
					daveAdelante = false;
			}
			else
			{
				posDave -= posDaveOffset * deltaTime;
				if (posDave < -limiteDave)
					daveAdelante = true;
			}
			if (daveAdelante && rumboDave < 90.0f)
			{
				rumboDave += rumboDaveOffset * deltaTime;
				if (rumboDave > 90.0f)
					rumboDave = 90.0f;
			}
			if (!daveAdelante && rumboDave > -90.0f)
			{
				rumboDave -= rumboDaveOffset * deltaTime;
				if (rumboDave < -90.0f)
					rumboDave = -90.0f;
			}
			giroCanones += giroCanonesOffset * deltaTime;
			if (giroCanones > 360.0f)
				giroCanones -= 360.0f;
		}

		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//piso de pasto de 200 por 200
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania);
		model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		verdaniaPastoTexture.UseTexture();
		meshListModel[0]->RenderMeshModel();

		//las hojas de abajo son sus patas y se mueven cruzadas
		float pasoPea = sin(anguloVerdania * 3.0f * toRadians);
		rotPeashooter[8].x = 25.0f * pasoPea;  //hoja_del_izq
		rotPeashooter[11].x = 25.0f * pasoPea;  //hoja_tras_der
		rotPeashooter[9].x = -25.0f * pasoPea;  //hoja_del_der
		rotPeashooter[10].x = -25.0f * pasoPea; //hoja_tras_izq
		//los brazos vienen en T y se bajan 70 grados
		rotPeashooter[6].z = -70.0f;  //brazo_izq abajo
		rotPeashooter[7].z = 70.0f;  //brazo_der abajo
		//su lugar en el circulo sale del seno y coseno, y se gira 90 grados mas para que mire hacia donde avanza
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + centroPeashooter +
			glm::vec3(radioPeashooter * sin(anguloPeashooter * toRadians), 0.0f, radioPeashooter * cos(anguloPeashooter * toRadians)));
		model = glm::rotate(model, (anguloPeashooter + 90.0f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(Peashooter_M, padresPeashooter, pivotesPeashooter, rotPeashooter, NUM_PIEZAS_PEASHOOTER, model, uniformModel);

		//las piernas van al reves una de la otra; la pantorrilla y el pie siguen al muslo porque son sus hijos
		float pasoDave = sin(anguloVerdania * 4.0f * toRadians);
		rotCrazyDave[11].x = 25.0f * pasoDave;  //muslo_der
		rotCrazyDave[14].x = -25.0f * pasoDave; //muslo_izq
		//los brazos vienen abiertos y se bajan 45 grados
		rotCrazyDave[5].z = 45.0f;  //brazo_der abajo
		rotCrazyDave[8].z = -45.0f;  //brazo_izq abajo
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(posDave, 0.0f, -8.0f));
		model = glm::rotate(model, rumboDave * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(CrazyDave_M, padresCrazyDave, pivotesCrazyDave, rotCrazyDave, NUM_PIEZAS_CRAZYDAVE, model, uniformModel);

		//ice pea y fire pea voltean la cabeza hasta 35 grados; el fire pea va desfasado para que no volteen juntos
		rotIcePea[2].y = 35.0f * sin(anguloVerdania * toRadians);  //cabeza
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(-6.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 40.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(IcePea_M, padresIcePea, pivotesIcePea, rotIcePea, NUM_PIEZAS_ICEPEA, model, uniformModel);

		//el gatling pea solo gira sus canones
		rotGatlingPea[4].z = giroCanones;
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(-7.0f, 0.0f, -3.0f));
		model = glm::rotate(model, 55.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(GatlingPea_M, padresGatlingPea, pivotesGatlingPea, rotGatlingPea, NUM_PIEZAS_GATLINGPEA, model, uniformModel);

		rotFirePea[2].y = 35.0f * sin((anguloVerdania + 240.0f) * toRadians);  //cabeza
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(-6.5f, 0.0f, -6.5f));
		model = glm::rotate(model, 70.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(FirePea_M, padresFirePea, pivotesFirePea, rotFirePea, NUM_PIEZAS_FIREPEA, model, uniformModel);

		//con el mismo valor en los dos brazos del cactus uno sube y el otro baja, porque estan de lados contrarios
		rotCactus[4].z = 10.0f * sin(anguloVerdania * 2.0f * toRadians); //brazo_izq
		rotCactus[5].z = 10.0f * sin(anguloVerdania * 2.0f * toRadians); //brazo_der
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(6.5f, 0.0f, -1.0f));
		model = glm::rotate(model, -40.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(Cactus_M, padresCactus, pivotesCactus, rotCactus, NUM_PIEZAS_CACTUS, model, uniformModel);

		//los girasoles mecen la cabeza de lado, cada uno desfasado 120 grados
		rotSunflower[6].z = -60.0f; //los brazos tambien vienen en T y se bajan 60 grados
		rotSunflower[7].z = 60.0f;
		rotSunflower[3].z = 8.0f * sin(anguloVerdania * toRadians); //cabeza
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(-3.0f, 0.0f, -12.0f));
		model = glm::rotate(model, 15.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(Sunflower_M, padresSunflower, pivotesSunflower, rotSunflower, NUM_PIEZAS_SUNFLOWER, model, uniformModel);

		rotSunflower2[6].z = -60.0f;
		rotSunflower2[7].z = 60.0f;
		rotSunflower2[3].z = 8.0f * sin((anguloVerdania + 120.0f) * toRadians); //cabeza
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(0.0f, 0.0f, -13.0f));
		DibujaJerarquia(Sunflower2_M, padresSunflower2, pivotesSunflower2, rotSunflower2, NUM_PIEZAS_SUNFLOWER2, model, uniformModel);

		rotStuffyFlower[6].z = -60.0f;
		rotStuffyFlower[7].z = 60.0f;
		rotStuffyFlower[3].z = 8.0f * sin((anguloVerdania + 240.0f) * toRadians); //cabeza
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(3.0f, 0.0f, -12.0f));
		model = glm::rotate(model, -15.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(StuffyFlower_M, padresStuffyFlower, pivotesStuffyFlower, rotStuffyFlower, NUM_PIEZAS_STUFFYFLOWER, model, uniformModel);

		rotHypnoShroom[2].z = 6.0f * sin(anguloVerdania * 2.0f * toRadians); //sombrero
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(7.0f, 0.0f, -6.0f));
		model = glm::rotate(model, -50.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(HypnoShroom_M, padresHypnoShroom, pivotesHypnoShroom, rotHypnoShroom, NUM_PIEZAS_HYPNOSHROOM, model, uniformModel);

		//el cuerpo del laser bean se balancea y el pelo lo sigue porque es su hijo
		rotLaserBean[1].z = 8.0f * sin(anguloVerdania * toRadians);  //cuerpo
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(5.0f, 0.0f, 3.0f));
		model = glm::rotate(model, -30.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(LaserBean_M, padresLaserBean, pivotesLaserBean, rotLaserBean, NUM_PIEZAS_LASERBEAN, model, uniformModel);

		rotPotatoMine[2].z = 15.0f * sin(anguloVerdania * 3.0f * toRadians); //antena
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(-4.0f, 0.0f, 4.0f));
		model = glm::rotate(model, 30.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		DibujaJerarquia(PotatoMine_M, padresPotatoMine, pivotesPotatoMine, rotPotatoMine, NUM_PIEZAS_POTATOMINE, model, uniformModel);

		//la wall-nut es una sola pieza, asi que se balancea todo el modelo apoyado en su base
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(8.5f, 0.0f, -9.0f));
		model = glm::rotate(model, -40.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 6.0f * sin(anguloVerdania * toRadians) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		WallNut_M.RenderModel();

		//la mandibula del torchwood se abre entre 0 y 12 grados; como a 6 se le suma 6 por el seno, nunca baja de 0
		rotTorchwood[4].x = 6.0f + 6.0f * sin(anguloVerdania * 2.0f * toRadians); //mandibula
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(0.0f, 0.0f, -18.0f));
		DibujaJerarquia(Torchwood_M, padresTorchwood, pivotesTorchwood, rotTorchwood, NUM_PIEZAS_TORCHWOOD, model, uniformModel);

		//jardin de crazy dave
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//casa al fondo, girada 180 grados porque su puerta ve hacia Z negativo
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(0.0f, 0.0f, -34.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		JardinCasa_M.RenderModel();

		//los tramos de la calle se giran 90 grados para que corran sobre X, y en la punta va el final
		for (int i = 0; i < NUM_TRAMOS_CALLE; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posVerdania + glm::vec3(posCalleX[i], 0.01f, posCalleZ));
			model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			JardinCalle_M.RenderModel();
		}
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(posCalleX[0] - 6.5f, 0.01f, posCalleZ));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		JardinCalleFin_M.RenderModel();

		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(posCalleX[0] - 5.0f, 0.0f, posCalleZ - 4.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		JardinFarol_M.RenderModel();

		//camino de piedra; cada piedra va girada distinto para que no se vea repetido
		for (int i = 0; i < NUM_PIEDRAS_CAMINO; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posVerdania + posCamino[i]);
			model = glm::rotate(model, i * 47.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			JardinCaminoPiedra_M.RenderModel();
		}

		//cerca; en indicePuerta va la pieza con la puerta
		for (int i = 0; i < NUM_CERCAS_JARDIN; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posVerdania + glm::vec3(posCercaX[i], 0.0f, posCercaZ));
			if (i == indicePuerta)
			{
				model = glm::scale(model, glm::vec3(escalaCasa * estiraPuerta, escalaCasa, escalaCasa));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				JardinCercaPuerta_M.RenderModel();
			}
			else
			{
				model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				JardinCerca_M.RenderModel();
			}
		}

		for (int i = 0; i < NUM_ARBOLES_JARDIN; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posVerdania + posArbolesJardin[i]);
			model = glm::rotate(model, giroArbolesJardin[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaArbolesJardin[i], escalaArbolesJardin[i], escalaArbolesJardin[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (i % 2 == 0)
				JardinArbol1_M.RenderModel();
			else
				JardinArbol2_M.RenderModel();
		}

		for (int i = 0; i < NUM_ARBUSTOS_JARDIN; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posVerdania + posArbustosJardin[i]);
			model = glm::rotate(model, giroArbustosJardin[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaArbustosJardin[i], escalaArbustosJardin[i], escalaArbustosJardin[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (i % 2 == 0)
				JardinArbusto_M.RenderModel();
			else
				JardinArbustoFlores_M.RenderModel();
		}

		for (int i = 0; i < NUM_FLORES_JARDIN; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posVerdania + posFloresJardin[i]);
			model = glm::rotate(model, i * 63.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaFloresJardin[i], escalaFloresJardin[i], escalaFloresJardin[i]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (i % 2 == 0)
				JardinFlores1_M.RenderModel();
			else
				JardinFlores2_M.RenderModel();
		}

		//en los detalles del pasto el residuo de i entre 3 dice si va trebol, helecho u hongo
		for (int i = 0; i < NUM_DETALLES_JARDIN; i++)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, posVerdania + posDetallesJardin[i]);
			model = glm::rotate(model, i * 71.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(escalaDetallesJardin[i % 3], escalaDetallesJardin[i % 3], escalaDetallesJardin[i % 3]));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			if (i % 3 == 0)
				JardinTrebol_M.RenderModel();
			else if (i % 3 == 1)
				JardinHelecho_M.RenderModel();
			else
				JardinHongo_M.RenderModel();
		}

		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(-17.0f, 0.0f, -13.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		JardinRoca_M.RenderModel();
		model = glm::mat4(1.0);
		model = glm::translate(model, posVerdania + glm::vec3(15.0f, 0.0f, -24.0f));
		model = glm::rotate(model, 120.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		JardinRoca_M.RenderModel();

		//Verdania en el espacio, gira despacio sobre Y; con escala 1350 mide unas 990 unidades
		giroPlanetaVerdania += giroPlanetaVerdaniaOffset * deltaTime;
		if (giroPlanetaVerdania > 360.0f)
			giroPlanetaVerdania -= 360.0f;
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlanetaVerdania);
		model = glm::rotate(model, giroPlanetaVerdania * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1350.0f, 1350.0f, 1350.0f));
		model = glm::translate(model, -centroCabezaPea);
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PlanetaVerdania_M.RenderModel();

		//--- nave de Among Us y crewmates ---
		anguloAmongUs += anguloAmongUsOffset * deltaTime;
		if (anguloAmongUs > 360.0f)
			anguloAmongUs -= 360.0f;

		glm::mat4 modelauxAmongUs = glm::mat4(1.0);
		modelauxAmongUs = glm::translate(modelauxAmongUs, posNaveAmongUs);
		modelauxAmongUs = glm::rotate(modelauxAmongUs, giroNaveAmongUs * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		model = modelauxAmongUs;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, avanceNave * sin(anguloAmongUs * toRadians)));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		AmongUsNave_M.RenderModel();

		//se ladea desde la mitad del cuerpo, por eso se sube 0.5 antes de girar y se baja despues
		for (int i = 0; i < NUM_CREWMATES_NAVE; i++)
		{
			float onda = sin((anguloAmongUs + i * 60.0f) * toRadians);
			model = modelauxAmongUs;
			model = glm::translate(model, posCrewmates[i] + glm::vec3(0.0f, 0.0f, -avanceCrewmates * onda));
			model = glm::rotate(model, giroYCrewmates[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
			model = glm::rotate(model, (ladeoCrewmates[i] + 10.0f * onda) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = colorCrewmates[i];
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			CrewmateCuerpo_M.RenderModel();
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			CrewmateVisor_M.RenderModel();
		}

		//el de Xen sube y baja con su isla, sin desfase
		glm::vec3 superficies[NUM_CREWMATES_ESCONDIDOS] = { posXen, posPlaneta2, posVerdania };
		for (int i = 0; i < NUM_CREWMATES_ESCONDIDOS; i++)
		{
			glm::vec3 pos = superficies[i] + posEscondidos[i];
			if (i == 0)
				pos.y += 0.8f * sin(anguloIslas * toRadians);
			model = glm::mat4(1.0);
			model = glm::translate(model, pos);
			model = glm::rotate(model, giroEscondidos[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = colorEscondidos[i];
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			CrewmateCuerpo_M.RenderModel();
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			CrewmateVisor_M.RenderModel();
		}
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//--- pista de carreras y naves ---
		//modelauxNaves es la pista sin voltear y modelauxPista la volteada en X y Z; la meta y los edificios van sobre ella
		model = glm::mat4(1.0f);
		model = glm::translate(model, posPista);
		model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
		modelauxNaves = model;
		model = glm::scale(model, glm::vec3(-1.0f, 1.0f, -1.0));
		modelauxPista = model;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::scale(model, glm::vec3(0.45f, 0.45f, 0.45f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Pista_M.RenderModel();

		//meta
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(0.0f, 0.05f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Meta_M.RenderModel();

		//cubierta de la meta
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		CubiertaMeta_M.RenderModel();

		avanceCarrera += velocidadCarrera * deltaTime;
		if (avanceCarrera > longitudTotalPista)
			avanceCarrera -= longitudTotalPista;

		//naves de F-Zero corriendo en la pista
		//son de metal, por eso usan Material_metal
		Material_metal.UseMaterial(uniformSpecularIntensity, uniformShininess);
		for (int i = 0; i < NUM_NAVES_FZERO; i++)
		{
			//las naves pares van en el carril izquierdo y las impares en el derecho
			float carrilX = 2.15f;
			if (i % 2 == 0)
				carrilX = -5.15f;

			//cada nave va 10 unidades detras de la anterior
			float desfaseAtras = -10.0f * (i + 1);

			float flotacion = 1.00f + 0.07f * sin((avanceCarrera * 0.1f) + i);

			model = modelauxNaves * ObtenerMatrizNavePista(avanceCarrera + desfaseAtras, carrilX, flotacion);
			model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			NavesFZero_M[i].RenderModel();
		}
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//puesto de observacion 1, son los edificios del centro
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(175.0f, 80.00f, 500.0f));
		model = glm::rotate(model, -45.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion1_M.RenderModel();

		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(175.0f, 80.00f, -375.0f));
		model = glm::rotate(model, -115.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion1_M.RenderModel();

		//puesto de observacion 2, son los edificios altos
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(0.0f, 20.00f, 700.0f));
		model = glm::rotate(model, -25.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion2_M.RenderModel();

		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(300.0f, 200.00f, 700.0f));
		model = glm::rotate(model, 45.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion2_M.RenderModel();

		//puesto de observacion 3, son los edificios del centro que estan en la orilla
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(150.0f, 180.00f, 800.0f));
		model = glm::scale(model, glm::vec3(0.75f, 0.75f, 0.75f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion3_M.RenderModel();

		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(150.0f, 140.00f, -680.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.75f, 0.75f, 0.75f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion3_M.RenderModel();

		//puesto de observacion 4
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(0.0f, 20.00f, -600.0f));
		model = glm::rotate(model, 35.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion4_M.RenderModel();

		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(300.0f, 100.00f, -600.0f));
		model = glm::rotate(model, -35.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PuestoObservacion4_M.RenderModel();

		//barco espacial, la nave nodriza; con escala 0.315 mide unos 540 de largo
		model = glm::mat4(1.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(200.0f, 30.0f, -40.0f));
		model = glm::rotate(model, 265.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.315f, 0.315f, 0.315f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		NaveNodriza_M.RenderModel();

		//almacen del puerto
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(700.f, -350.00f, 50.0f));
		model = glm::rotate(model, 60.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Almacen_M.RenderModel();

		//nave 1 de la pista
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(70.0f, 140.0f, 30.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 30.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		NavePista_M.RenderModel();

		//nave 2 de la pista
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(-70.0f, 140.0f, 30.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -30.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		NavePista_M.RenderModel();

		//--- satelites y cuerpos espaciales ---
		//satelite
		model = modelauxPista;
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(-125.0f, 50.0f, -750.0f));
		model = glm::rotate(model, 240.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Satelite_M.RenderModel();

		//luna de Xen
		model = glm::mat4(1.0);
		model = glm::translate(model, posPlanetaXen + glm::vec3(1300.0f, 1300.0f, 0.0f));
		model = glm::rotate(model, giroPlanetaXen * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LunaXen_M.RenderModel();

		//anillo de asteroides alrededor de Xen; cada uno se gira distinto para que no se vean iguales
		for (int i = 0; i < totalAsteroides; i++)
		{
			float anguloAsteroide = i * pasoAngulo * toRadians;
			model = glm::mat4(1.0);
			model = glm::translate(model, posPlanetaXen + glm::vec3(radioAsteroides * cos(anguloAsteroide), 0.0f, radioAsteroides * sin(anguloAsteroide)));
			model = glm::rotate(model, (giroPlanetaXen + i * 22.5f) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Asteroides_M.RenderModel();
		}

		//--- plataforma de lanzamiento y cohete ---
		//keyframes: B empieza la animacion y quita el despegue normal para que no haya dos cohetes; R regresa el cohete a la plataforma
		if (mainWindow.getiniciosKeyframes() != iniciosKeyframesVistos)
		{
			iniciosKeyframesVistos = mainWindow.getiniciosKeyframes();
			estadoCohete = 0;
			alturaCohete = 0.0f;
			velocidadCohete = 0.0f;
			fuegoCohete = 0.0f;
			rotCohete[0].z = 0.0f;
			animCohete.Inicia();
		}
		if (mainWindow.getreinicioKeyframes() != reinicioKeyframesVistos)
		{
			reinicioKeyframesVistos = mainWindow.getreinicioKeyframes();
			animCohete.Detiene();
		}
		if (mainWindow.getimprimeKeyframes() != imprimeKeyframesVistos)
		{
			imprimeKeyframesVistos = mainWindow.getimprimeKeyframes();
			animCohete.ImprimeTodos();
		}
		if (mainWindow.getguardaKeyframes() != guardaKeyframesVistos)
		{
			guardaKeyframesVistos = mainWindow.getguardaKeyframes();
			animCohete.Guarda(archivoKeyframesGuardado);
		}
		//los keyframes van en segundos, por eso deltaTime se divide entre 61 (con P se pausan porque deltaTime vale 0)
		animCohete.Actualiza(deltaTime / 61.0f);

		//los tiempos del despegue van en segundos, por eso deltaTime se divide entre 61
		if (estadoCohete == 0)
		{
			//parado; si la F tiene prendidos los despegues empieza la cuenta, salvo que los keyframes tengan el cohete
			if (mainWindow.getdespegaCohete() && !animCohete.activa)
			{
				cuentaCohete = tiempoCuentaCohete;
				estadoCohete = 1;
			}
		}
		else if (estadoCohete == 1)
		{
			//en la cuenta la llama crece y el cohete tiembla desde la base
			cuentaCohete -= deltaTime / 61.0f;
			fuegoCohete += fuegoCuentaOffset * deltaTime;
			if (fuegoCohete > 0.5f)
				fuegoCohete = 0.5f;
			rotCohete[0].z = 3.0f * fuegoCohete * sin(cuentaCohete * 2000.0f * toRadians);
			if (cuentaCohete <= 0.0f)
			{
				rotCohete[0].z = 0.0f;
				velocidadCohete = 0.0f;
				estadoCohete = 2;
			}
		}
		else if (estadoCohete == 2)
		{
			fuegoCohete = 1.0f;
			velocidadCohete += aceleracionCohete * deltaTime;
			alturaCohete += velocidadCohete * deltaTime;
			if (alturaCohete >= alturaMaxCohete)
			{
				alturaCohete = alturaMaxCohete;
				estadoCohete = 3;
			}
		}
		else if (estadoCohete == 3)
		{
			fuegoCohete = 0.6f;
			alturaCohete -= velBajadaCohete * deltaTime;
			if (alturaCohete <= 0.0f)
			{
				alturaCohete = 0.0f;
				fuegoCohete = 0.0f;
				estadoCohete = 0;
			}
		}

		model = glm::mat4(1.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, posPlaneta2 + posPlataforma);
		model = glm::scale(model, glm::vec3(escalaPlataforma, escalaPlataforma, escalaPlataforma));
		modelauxPlataforma = model;
		//piso de 3 por 3 placas; miden 0.1 de grueso, por eso la torre y el cohete se suben 0.1
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				model = modelauxPlataforma;
				model = glm::translate(model, glm::vec3(i * 2.0f, 0.0f, j * 2.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				PlataformaCohete_M.RenderModel();
			}
		}
		Material_metal.UseMaterial(uniformSpecularIntensity, uniformShininess);
		//torre de 5 cubos de vigas a un lado del cohete
		for (int i = 0; i < pisosTorre; i++)
		{
			model = modelauxPlataforma;
			model = glm::translate(model, glm::vec3(1.6f, 0.1f + i * 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			TorreCohete_M.RenderModel();
		}
		//cohete por piezas; raizCohete se guarda para poner la llama abajo del motor
		model = modelauxPlataforma;
		model = glm::translate(model, glm::vec3(0.0f, 0.1f + alturaCohete, 0.0f));
		glm::mat4 raizCohete = model;
		//la llama y el humo salen de la pieza que queda abajo: en el despegue normal es la raiz, y con keyframes la que diga el keyframe
		glm::mat4 matFuegoCohete = raizCohete;
		float tamanoFuego = fuegoCohete;
		float fondoFuego = 0.0f;
		float anchoFuego = 1.0f;
		if (animCohete.activa)
		{
			glm::mat4 matPiezas[NUM_PIEZAS_COHETE];
			DibujaCoheteKeyframes(Cohete_M, animCohete, raizCohete, uniformModel, matPiezas);
			matFuegoCohete = matPiezas[animCohete.piezaFuego];
			tamanoFuego = animCohete.fuego;
			fondoFuego = fondoCohete[animCohete.piezaFuego];
			anchoFuego = anchoFuegoCohete[animCohete.piezaFuego];
		}
		else
		{
			DibujaJerarquia(Cohete_M, padresCohete, pivotesCohete, rotCohete, NUM_PIEZAS_COHETE, model, uniformModel);
		}
		//la llama es la pieza del fuego del torchwood puesta de cabeza abajo del motor; crece con el tamano de la llama
		if (tamanoFuego > 0.0f)
		{
			model = matFuegoCohete;
			model = glm::translate(model, glm::vec3(0.0f, fondoFuego + 0.2f, 0.0f));
			model = glm::rotate(model, 180.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.6f * anchoFuego, 1.2f * tamanoFuego, 0.6f * anchoFuego));
			//se baja el fuego para que su base quede en el origen
			model = glm::translate(model, glm::vec3(0.0f, -1.9f, 0.23f));
			//con color arriba de 1 brilla aunque no le llegue luz
			color = glm::vec3(2.0f, 2.0f, 2.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Torchwood_M[3].RenderModel();
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		}
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//las nubes del humo envejecen con deltaTime, y mientras haya llama sale una nueva de abajo del motor
		for (int i = 0; i < MAX_HUMO; i++)
		{
			if (edadHumo[i] < duracionHumo)
				edadHumo[i] += deltaTime / 61.0f;
		}
		esperaHumo += deltaTime / 61.0f;
		//cuando el cohete todavia esta en la plataforma ese punto queda abajo del piso, asi que la nube se sube a la altura de las placas
		if (tamanoFuego > 0.0f && esperaHumo >= intervaloHumo)
		{
			glm::vec3 salidaHumo = glm::vec3(matFuegoCohete * glm::vec4(0.0f, fondoFuego - 0.6f, 0.0f, 1.0f));
			if (salidaHumo.y < 1.0f)
				salidaHumo.y = 1.0f;
			SueltaHumo(salidaHumo);
			esperaHumo = 0.0f;
		}

		//astronautas: Gordon y el Capitan Falcon junto a la plataforma
		anguloAstronautas += anguloAstronautasOffset * deltaTime;
		if (anguloAstronautas > 360.0f)
			anguloAstronautas -= 360.0f;
		if (estadoCohete == 1 || estadoCohete == 2 || animCohete.activa)
		{
			vistaCohete += vistaCoheteOffset * deltaTime;
			if (vistaCohete > 1.0f)
				vistaCohete = 1.0f;
		}
		else
		{
			vistaCohete -= vistaCoheteOffset * deltaTime;
			if (vistaCohete < 0.0f)
				vistaCohete = 0.0f;
		}
		//la pose se mezcla de espera a despegue con vistaCohete
		float rumboAstronautas = rumboEspera + (rumboDespegue - rumboEspera) * vistaCohete;
		//la cabeza sube conforme sube el cohete, hasta 35 grados
		float subeMirada = alturaCohete;
		if (animCohete.activa)
			subeMirada = animCohete.pos[4].y; //con keyframes mira a la capsula
		if (subeMirada > 35.0f)
			subeMirada = 35.0f;

		//reloj de Gordon; si el cohete despega deja de verlo y baja el brazo
		if (estadoReloj == 0)
		{
			tiempoReloj += deltaTime;
			if (tiempoReloj > esperaReloj && vistaCohete == 0.0f)
				estadoReloj = 1;
		}
		else if (estadoReloj == 1)
		{
			brazoReloj += brazoRelojOffset * deltaTime;
			if (brazoReloj > 1.0f)
			{
				brazoReloj = 1.0f;
				tiempoReloj = 0.0f;
				estadoReloj = 2;
			}
		}
		else if (estadoReloj == 2)
		{
			tiempoReloj += deltaTime;
			if (tiempoReloj > veReloj || vistaCohete > 0.0f)
				estadoReloj = 3;
		}
		else
		{
			brazoReloj -= brazoRelojOffset * deltaTime;
			if (brazoReloj < 0.0f)
			{
				brazoReloj = 0.0f;
				tiempoReloj = 0.0f;
				estadoReloj = 0;
			}
		}
		//el antebrazo izquierdo se dobla en X para ver el reloj, y la cabeza baja hacia la muneca
		float esperaCabezaX = 30.0f * brazoReloj;
		float esperaCabezaY = 20.0f * sin(anguloAstronautas * toRadians) * (1.0f - brazoReloj) + 15.0f * brazoReloj;
		rotGordon[5].z = -35.0f;  //brazo_izq
		rotGordon[5].x = -25.0f * brazoReloj;
		rotGordon[5].y = -55.0f * brazoReloj;
		rotGordon[6].x = -80.0f * brazoReloj;  //antebrazo_izq
		//el brazo derecho sube para taparse el sol, y el antebrazo se dobla 150 grados en Z hasta la frente
		rotGordon[2].z = 35.0f + (-50.0f - 35.0f) * vistaCohete; //brazo_der
		rotGordon[2].y = 35.0f * vistaCohete;
		rotGordon[3].z = -150.0f * vistaCohete; //antebrazo_der
		rotGordon[1].x = esperaCabezaX + (-subeMirada - esperaCabezaX) * vistaCohete; //cabeza
		rotGordon[1].y = esperaCabezaY + (-50.0f - esperaCabezaY) * vistaCohete;
		model = glm::mat4(1.0f);
		model = glm::translate(model, posPlaneta2 + posGordonAstronauta);
		model = glm::rotate(model, rumboAstronautas * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaGordonAstronauta, escalaGordonAstronauta, escalaGordonAstronauta));
		DibujaJerarquia(Gordon_M, padresGordon, pivotesGordon, rotGordon, NUM_PIEZAS_GORDON, model, uniformModel);

		//el Capitan Falcon viene en T; las manos quedan en la cintura y al despegar saluda con la derecha
		float esperaFalconY = 30.0f * cos(anguloAstronautas * toRadians);
		float saludo = 30.0f * sin(anguloAstronautas * 8.0f * toRadians);
		rotFalcon[2].z = 45.0f + (-40.0f - 45.0f) * vistaCohete;  //brazo_der
		rotFalcon[3].z = 80.0f + (-50.0f + saludo - 80.0f) * vistaCohete;  //antebrazo_der
		rotFalcon[4].z = -45.0f;  //brazo_izq
		rotFalcon[5].z = -80.0f;  //antebrazo_izq
		rotFalcon[1].y = esperaFalconY * (1.0f - vistaCohete);  //cabeza
		//sus pies estan en -0.627 de y, por eso se sube eso despues de escalar
		model = glm::mat4(1.0f);
		model = glm::translate(model, posPlaneta2 + posFalconAstronauta);
		model = glm::rotate(model, rumboAstronautas * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaFalconAstronauta, escalaFalconAstronauta, escalaFalconAstronauta));
		model = glm::translate(model, glm::vec3(0.0f, 0.627f, 0.0f));
		DibujaJerarquia(CapitanFalcon_M, padresFalcon, pivotesFalcon, rotFalcon, NUM_PIEZAS_FALCON, model, uniformModel);

		//--- estacion espacial y naves ---
		//modelauxEstacion se guarda antes de escalar la estacion, asi los pads y las naves no se hacen chicos con ella
		model = glm::mat4(1.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, posEstacion);
		modelauxEstacion = model;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Estacion_M.RenderModel();

		//landing pads; el lado donde se posan las naves queda hacia afuera al girar cada pad
		model = modelauxEstacion;
		model = glm::translate(model, glm::vec3(-150.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LandingPad_M.RenderModel();

		model = modelauxEstacion;
		model = glm::translate(model, glm::vec3(150.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LandingPad_M.RenderModel();

		model = modelauxEstacion;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 150.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LandingPad_M.RenderModel();

		model = modelauxEstacion;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -150.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LandingPad_M.RenderModel();

		float segundosFrame = deltaTime / 61.0f;
		Material_metal.UseMaterial(uniformSpecularIntensity, uniformShininess);
		for (int i = 0; i < NUM_NAVES_ESTACION; i++)
		{
			if (estadoNavesEstacion[i] == 0)
			{
				//estacionada, esperando a que se acabe su tiempo
				esperaNavesEstacion[i] -= segundosFrame;
				if (esperaNavesEstacion[i] <= 0.0f)
					estadoNavesEstacion[i] = 1;
			}
			else if (estadoNavesEstacion[i] == 1)
			{
				//se eleva sobre su pad
				subidaNavesEstacion[i] += velSubidaNave * deltaTime;
				if (subidaNavesEstacion[i] >= alturaNaveEstacion)
				{
					subidaNavesEstacion[i] = alturaNaveEstacion;
					estadoNavesEstacion[i] = 2;
				}
			}
			else if (estadoNavesEstacion[i] == 2)
			{
				//gira 180 grados en Y para quedar viendo hacia afuera
				vueltaNavesEstacion[i] += velGiroNave * deltaTime;
				if (vueltaNavesEstacion[i] >= 180.0f)
				{
					vueltaNavesEstacion[i] = 180.0f;
					estadoNavesEstacion[i] = 3;
				}
			}
			else if (estadoNavesEstacion[i] == 3)
			{
				//sale a hipervelocidad; al llegar a distanciaSalida deja de dibujarse y toma cuanto tarda en regresar
				avanceNavesEstacion[i] += velHipervelocidad * deltaTime;
				if (avanceNavesEstacion[i] >= distanciaSalida)
				{
					avanceNavesEstacion[i] = distanciaSalida;
					visibleNavesEstacion[i] = false;
					esperaNavesEstacion[i] = tiempoMinFuera + SiguienteEspera(i);
					estadoNavesEstacion[i] = 4;
				}
			}
			else if (estadoNavesEstacion[i] == 4)
			{
				//fuera de la escena; cuando se acaba su tiempo vuelve a aparecer lejos, ya viendo hacia la estacion
				esperaNavesEstacion[i] -= segundosFrame;
				if (esperaNavesEstacion[i] <= 0.0f)
				{
					vueltaNavesEstacion[i] = 0.0f;
					subidaNavesEstacion[i] = alturaNaveEstacion;
					avanceNavesEstacion[i] = distanciaSalida;
					visibleNavesEstacion[i] = true;
					estadoNavesEstacion[i] = 5;
				}
			}
			else if (estadoNavesEstacion[i] == 5)
			{
				//regresa a hipervelocidad hasta que le faltan distanciaFrenado para llegar
				avanceNavesEstacion[i] -= velHipervelocidad * deltaTime;
				if (avanceNavesEstacion[i] <= distanciaFrenado)
				{
					avanceNavesEstacion[i] = distanciaFrenado;
					estadoNavesEstacion[i] = 6;
				}
			}
			else if (estadoNavesEstacion[i] == 6)
			{
				//se acerca despacio hasta quedar arriba de su pad
				avanceNavesEstacion[i] -= velAcercamiento * deltaTime;
				if (avanceNavesEstacion[i] <= 0.0f)
				{
					avanceNavesEstacion[i] = 0.0f;
					estadoNavesEstacion[i] = 7;
				}
			}
			else
			{
				//baja hasta posarse y toma cuanto tiempo se queda estacionada
				subidaNavesEstacion[i] -= velSubidaNave * deltaTime;
				if (subidaNavesEstacion[i] <= 0.0f)
				{
					subidaNavesEstacion[i] = 0.0f;
					esperaNavesEstacion[i] = tiempoMinEstacionada + SiguienteEspera(i);
					estadoNavesEstacion[i] = 0;
				}
			}

			//su lugar es el de su pad mas lo que ha subido y lo que ha avanzado hacia afuera
			if (visibleNavesEstacion[i])
			{
				glm::vec3 posNave = posNavesEstacion[i] + glm::vec3(0.0f, subidaNavesEstacion[i], 0.0f) + salidaNavesEstacion[i] * avanceNavesEstacion[i];
				model = modelauxEstacion;
				model = glm::translate(model, posNave);
				model = glm::rotate(model, (giroNavesEstacion[i] + vueltaNavesEstacion[i]) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				if (modeloNavesEstacion[i] == 0)
					NaveDorada_M.RenderModel();
				else if (modeloNavesEstacion[i] == 1)
					NaveBlanca_M.RenderModel();
				else
					NaveCaza_M.RenderModel();
			}
		}
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//--- avatar ---
		//usa deltaReal para que el paso siga aunque las demas animaciones esten en pausa
		if (avatarCamina)
		{
			if (pasoAdelanteAvatar)
			{
				pasoAvatar += pasoAvatarOffset * deltaReal;
				if (pasoAvatar > limitePasoAvatar)
					pasoAdelanteAvatar = false;
			}
			else
			{
				pasoAvatar -= pasoAvatarOffset * deltaReal;
				if (pasoAvatar < -limitePasoAvatar)
					pasoAdelanteAvatar = true;
			}
		}
		else
		{
			if (pasoAvatar > 0.0f)
			{
				pasoAvatar -= pasoAvatarOffset * deltaReal;
				if (pasoAvatar < 0.0f)
					pasoAvatar = 0.0f;
			}
			else if (pasoAvatar < 0.0f)
			{
				pasoAvatar += pasoAvatarOffset * deltaReal;
				if (pasoAvatar > 0.0f)
					pasoAvatar = 0.0f;
			}
		}
		//los pies se balancean en X desde la cadera, cada uno al reves del otro
		rotAvatar[3].x = pasoAvatar;
		rotAvatar[4].x = -pasoAvatar;
		//las manos giran en Y, cada una al reves del pie de su lado
		rotAvatar[1].y = pasoAvatar * 1.2f;
		rotAvatar[2].y = pasoAvatar * 1.2f;
		//en la carnivora las hojas de abajo son las patas, cruzadas, la delantera izquierda con la trasera derecha
		rotAvatarCarnivora[8].x = pasoAvatar;  //hoja_del_izq
		rotAvatarCarnivora[9].x = pasoAvatar;  //hoja_tras_der
		rotAvatarCarnivora[7].x = -pasoAvatar;  //hoja_del_der
		rotAvatarCarnivora[10].x = -pasoAvatar; //hoja_tras_izq
		//el avatar da un pequeno salto en cada paso, mas alto mientras mas separados van los pies
		float saltoAvatar = 0.03f * fabs(pasoAvatar) / limitePasoAvatar;

		//el avatar solo esta en los planetas y se queda donde se dejo aunque se cambie a la camara libre
		if (zonaActual != 0)
		{
			model = glm::mat4(1.0);
			model = glm::translate(model, centroAvatar + posAvatar + glm::vec3(0.0f, saltoAvatar, 0.0f));
			model = glm::rotate(model, rumboAvatar * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_avatar.UseMaterial(uniformSpecularIntensity, uniformShininess);
			//la T cambia cual modelo se dibuja
			if (mainWindow.getpersonajeAvatar() == 0)
			{
				model = glm::scale(model, glm::vec3(escalaAvatar, escalaAvatar, escalaAvatar));
				DibujaJerarquia(Kirby_M, padresKirby, pivotesAvatar, rotAvatar, NUM_PIEZAS_KIRBY, model, uniformModel);
			}
			else
			{
				model = glm::scale(model, glm::vec3(escalaAvatarCarnivora, escalaAvatarCarnivora, escalaAvatarCarnivora));
				DibujaJerarquia(Carnivora_M, padresCarnivora, pivotesCarnivora, rotAvatarCarnivora, NUM_PIEZAS_CARNIVORA, model, uniformModel);
			}
		}

		//--- camaras y recorridos ---
		//nave del recorrido de la camara 4
		if (mainWindow.getmodoCamara() == 4)
		{
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_metal.UseMaterial(uniformSpecularIntensity, uniformShininess);
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matrizNaveViaje));
			NaveViaje_M.RenderModel();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		}

		//en la camara 5 el cohete sigue en el espacio con las mismas piezas y escala que en la plataforma
		if (mainWindow.getmodoCamara() == 5 && etapaRecorrido >= 1 && avanceCoheteEspacio < avanceMaxCohete)
		{
			//se inclina sobre ejeGiroCoheteEspacio para que la punta vea hacia donde va
			model = glm::mat4(1.0f);
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			model = glm::translate(model, inicioCoheteEspacio + frenteCoheteEspacio * avanceCoheteEspacio);
			model = glm::rotate(model, inclinaCoheteEspacio * toRadians, ejeGiroCoheteEspacio);
			model = glm::scale(model, glm::vec3(escalaPlataforma, escalaPlataforma, escalaPlataforma));
			glm::mat4 raizCoheteEspacio = model;
			Material_metal.UseMaterial(uniformSpecularIntensity, uniformShininess);
			DibujaJerarquia(Cohete_M, padresCohete, pivotesCohete, rotCohete, NUM_PIEZAS_COHETE, model, uniformModel);
			model = raizCoheteEspacio;
			model = glm::translate(model, glm::vec3(0.0f, 0.2f, 0.0f));
			model = glm::rotate(model, 180.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.6f, 1.2f, 0.6f));
			model = glm::translate(model, glm::vec3(0.0f, -1.9f, 0.23f));
			color = glm::vec3(2.0f, 2.0f, 2.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Torchwood_M[3].RenderModel();
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			//como va muy rapido, se sueltan tres nubes repartidas en lo que avanzo desde la ultima vez
			if (esperaHumo >= intervaloHumo)
			{
				glm::vec3 salidaHumo = glm::vec3(raizCoheteEspacio * glm::vec4(0.0f, -0.6f, 0.0f, 1.0f));
				float recorridoHumo = velCoheteEspacio * 61.0f * esperaHumo;
				for (int k = 0; k < 3; k++)
					SueltaHumo(salidaHumo - frenteCoheteEspacio * (recorridoHumo * k / 3.0f));
				esperaHumo = 0.0f;
			}
		}

		//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---
		//de noche sale la luna del lado contrario al sol; el color va arriba de 1 para que brillen aunque la luz les pegue por atras
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		model = glm::mat4(1.0);
		if (mainWindow.getzonaPlaneta() == 0 || !esDeNoche)
		{
			model = glm::translate(model, posSol);
			model = glm::scale(model, glm::vec3(tamanoSol, tamanoSol, tamanoSol));
			color = glm::vec3(3.0f, 3.0f, 3.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Sol_M.RenderModel();
		}
		else
		{
			model = glm::translate(model, posVista - haciaSol * distanciaSol);
			model = glm::scale(model, glm::vec3(tamanoLuna, tamanoLuna, tamanoLuna));
			color = glm::vec3(5.0f, 5.0f, 5.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Luna_M.RenderModel();
		}
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		// ---------- objetos con transparencia ----------
		//van al final, despues de todos los opacos, si no se ven mal
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//--- personajes ---

		//--- planetas: Xen y planeta 2 ---

		//--- pista de carreras y naves ---

		//--- satelites y cuerpos espaciales ---

		//--- plataforma de lanzamiento y cohete ---
		//con glDepthMask apagado las nubes no escriben profundidad, asi una orilla transparente no tapa a las de atras
		//color arriba de 1, como la llama, para que se vea claro aunque el sol no le pegue
		glDepthMask(GL_FALSE);
		color = glm::vec3(1.5f, 1.5f, 1.5f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		humoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		for (int i = 0; i < MAX_HUMO; i++)
		{
			if (edadHumo[i] < duracionHumo)
			{
				//con el seno de la edad la nube crece y luego se encoge; el angulo va de 30 a 180 grados
				float tamano = tamanoHumo * sin((30.0f + edadHumo[i] / duracionHumo * 150.0f) * toRadians);
				model = glm::mat4(1.0f);
				model = glm::translate(model, posHumo[i]);
				model = glm::scale(model, glm::vec3(tamano, tamano, tamano));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				meshListModel[idxHumo]->RenderMeshModel();
			}
		}
		glDepthMask(GL_TRUE);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		//--- estacion espacial y naves ---

		//--- avatar ---

		//--- camaras y recorridos ---
		//la estela sale hacia Z positivo y se alarga con fuegoViaje; no escribe profundidad y su color va arriba de 1
		if (mainWindow.getmodoCamara() == 4 && fuegoViaje > 0.0f)
		{
			glDepthMask(GL_FALSE);
			color = glm::vec3(2.0f, 2.0f, 2.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			model = glm::scale(matrizNaveViaje, glm::vec3(1.0f, 1.0f, fuegoViaje));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			EstelaViaje_M.RenderModel();
			glDepthMask(GL_TRUE);
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		}

		//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---

		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
