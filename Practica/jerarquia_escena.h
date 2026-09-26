// jerarquia_escena.h
// Tablas de jerarquia del Holocron y del Satelite en un solo archivo.
//
// Los dos .h que venian por separado declaraban cada uno su propia copia de
// struct ParteModelo. Si se incluyen los dos en el mismo .cpp el compilador
// marca "redefinicion de ParteModelo", asi que aqui la estructura se declara
// UNA sola vez y despues van las dos tablas.
//
// Sistema de coordenadas de los dos modelos: Y arriba, mano derecha.
// Cada OBJ ya trae su origen puesto en su pivote, de modo que la matriz de
// cualquier pieza es siempre:
//
//     M_pieza = M_padre * Traslacion(offset) * Rotacion(eje, angulo)
//
#pragma once

struct ParteModelo {
	const char* nombre;
	const char* archivo;
	int         padre;     // indice del padre, -1 si es raiz
	float       offset[3]; // pivote respecto al pivote del padre
	float       eje[3];    // eje de giro normalizado; {0,0,0} = no gira
};

//==================================================================
//  HOLOCRON
//  Cubo de 10.4 por lado, ya recentrado: el centro del cubo es (0,0,0).
//  Las 8 esquinas tienen offset (0,0,0) porque su origen YA es el centro
//  del cubo. Por eso al rotarlas giran alrededor del centro sin moverse
//  de lugar, que es justo lo que pide el punto 2.
//  Cada esquina gira sobre la diagonal que va del centro hacia ella,
//  o sea uno de los ocho vectores (+-1, +-1, +-1)/raiz(3).
//==================================================================
static const ParteModelo kPartesHolocron[] = {
	{ "Nucleo",     "Holocron_Nucleo.obj",            -1, { 0.0f, 0.0f, 0.0f }, {  0.000000f,  0.000000f,  0.000000f } },
	{ "Sup Der Fre","Holocron_Esquina_SupDerFre.obj",  0, { 0.0f, 0.0f, 0.0f }, {  0.577350f,  0.577350f,  0.577350f } },
	{ "Sup Der Tra","Holocron_Esquina_SupDerTra.obj",  0, { 0.0f, 0.0f, 0.0f }, {  0.577350f,  0.577350f, -0.577350f } },
	{ "Sup Izq Fre","Holocron_Esquina_SupIzqFre.obj",  0, { 0.0f, 0.0f, 0.0f }, { -0.577350f,  0.577350f,  0.577350f } },
	{ "Sup Izq Tra","Holocron_Esquina_SupIzqTra.obj",  0, { 0.0f, 0.0f, 0.0f }, { -0.577350f,  0.577350f, -0.577350f } },
	{ "Inf Der Fre","Holocron_Esquina_InfDerFre.obj",  0, { 0.0f, 0.0f, 0.0f }, {  0.577350f, -0.577350f,  0.577350f } },
	{ "Inf Der Tra","Holocron_Esquina_InfDerTra.obj",  0, { 0.0f, 0.0f, 0.0f }, {  0.577350f, -0.577350f, -0.577350f } },
	{ "Inf Izq Fre","Holocron_Esquina_InfIzqFre.obj",  0, { 0.0f, 0.0f, 0.0f }, { -0.577350f, -0.577350f,  0.577350f } },
	{ "Inf Izq Tra","Holocron_Esquina_InfIzqTra.obj",  0, { 0.0f, 0.0f, 0.0f }, { -0.577350f, -0.577350f, -0.577350f } },
};
static const int kNumPartesHolocron = 9;

// Medidas del Holocron ensamblado (sacadas de los .obj):
//   X: -5.205 .. 5.205   Y: -5.272 .. 5.272   Z: -5.254 .. 5.254
static const float kHolocronMitad = 5.272f;   // del centro a la cara

//==================================================================
//  SATELITE
//  +X = frente (lado de la antena), +Z = ala izquierda. Ya venia centrado.
//  El Cuerpo es la raiz: moverlo con el teclado arrastra todo lo demas,
//  que es el punto 4. Las otras cuatro piezas giran en su union con el
//  cuerpo, no alrededor del centro del satelite (punto 5).
//==================================================================
static const ParteModelo kPartesSatelite[] = {
	{ "Cuerpo",         "Satelite_Cuerpo.obj",          -1, {  0.00000f, 0.0f,  0.00000f }, { 0.0f, 0.0f, 0.0f } },
	{ "Ala Izquierda",  "Satelite_Ala_Izq.obj",          0, { -3.15580f, 0.0f,  0.53363f }, { 0.0f, 0.0f, 1.0f } },
	{ "Ala Derecha",    "Satelite_Ala_Der.obj",          0, { -3.15580f, 0.0f, -0.53363f }, { 0.0f, 0.0f, 1.0f } },
	{ "Antena",         "Satelite_Antena.obj",           0, {  2.16316f, 0.0f, -0.10100f }, { 0.0f, 1.0f, 0.0f } },
	{ "Modulo Trasero", "Satelite_Modulo_Trasero.obj",   0, { -4.75249f, 0.0f,  0.00000f }, { 1.0f, 0.0f, 0.0f } },
};
static const int kNumPartesSatelite = 5;

// Indices utiles del satelite
static const int kSatCuerpo = 0;
static const int kSatAlaIzq = 1;
static const int kSatAlaDer = 2;
static const int kSatAntena = 3;
static const int kSatModulo = 4;

// Medidas del Satelite ensamblado:
//   X: -6.621 .. 7.713   Y: -3.973 .. 3.973   Z: -11.789 .. 11.789
static const float kSateliteBajo = -3.973f;   // punto mas bajo respecto al origen
