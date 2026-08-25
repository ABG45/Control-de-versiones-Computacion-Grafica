/*
   PROGRAMA: Iniciales AGA con cambio de color de fondo aleatorio
   DESCRIPCION: Muestra las iniciales "AGA" formadas por triángulos
                en el centro de la pantalla. El fondo cambia
                aleatoriamente entre colores RGB cada 2 segundos.

   NOTA DE LA CORRECCION:
   Las letras se generan a partir de una matriz de puntos (bitmap) de
   5 columnas x 7 filas, igual que las fuentes de pantallas LED clásicas.
   Cada celda "encendida" del bitmap se dibuja como un rectángulo
   (2 triángulos). Esto evita el problema anterior, donde las "patas"
   de la A terminaban en un solo punto y el triángulo del "techo"
   tenía una base más ancha que ese punto, dejando huecos sin conectar.
*/

#include <stdio.h>
#include <string.h>
#include <vector>
#include <glew.h>
#include <glfw3.h>
#include <cstdlib>
#include <ctime>

/*
   CONFIGURACION DE LA VENTANA
*/
const int WIDTH = 1024, HEIGHT = 768;

/*
   VARIABLES GLOBALES
*/
GLuint VAO[3], VBO[3], shader;
GLsizei vertexCount[3];          // Cantidad de vertices de cada letra (varia segun el bitmap)
float colorTimer = 0.0f;
float bgR = 0.0f, bgG = 0.0f, bgB = 0.0f;

/*
   FUENTE DE MATRIZ 5x7
   1 = pixel encendido, 0 = pixel apagado
*/
const int FILAS = 7, COLUMNAS = 5;

int letraA[FILAS][COLUMNAS] = {
    {0,1,1,1,0},
    {1,0,0,0,1},
    {1,0,0,0,1},
    {1,1,1,1,1},
    {1,0,0,0,1},
    {1,0,0,0,1},
    {1,0,0,0,1}
};

int letraG[FILAS][COLUMNAS] = {
    {0,1,1,1,0},
    {1,0,0,0,1},
    {1,0,0,0,0},
    {1,0,1,1,1},
    {1,0,0,0,1},
    {1,0,0,0,1},
    {0,1,1,1,0}
};

/*
   SHADERS (Programas que corren en la GPU)
*/
static const char* vShader = "						\n\
#version 330										\n\
layout (location = 0) in vec3 pos;					\n\
void main()											\n\
{													\n\
	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f); \n\
}";

static const char* fShader = "						\n\
#version 330										\n\
uniform vec3 uniColor;								\n\
out vec4 color;										\n\
void main()											\n\
{													\n\
	color = vec4(uniColor, 1.0f);	 				\n\
}";

/*
   FUNCION: Generar los vertices de una letra a partir de su bitmap 5x7
   gx0, gy0  -> esquina inferior izquierda donde se dibuja la letra
   cellW/H   -> tamaño de cada "pixel" de la letra
*/
std::vector<GLfloat> GenerarVerticesLetra(int bitmap[FILAS][COLUMNAS], float gx0, float gy0, float cellW, float cellH)
{
    std::vector<GLfloat> verts;

    for (int fila = 0; fila < FILAS; fila++)
    {
        for (int col = 0; col < COLUMNAS; col++)
        {
            if (bitmap[fila][col] == 1)
            {
                float x0 = gx0 + col * cellW;
                float x1 = x0 + cellW;

                // La fila 0 del bitmap es la de ARRIBA, por eso se invierte con (FILAS - fila)
                float yTop = gy0 + (FILAS - fila) * cellH;
                float yBot = yTop - cellH;

                // Triangulo 1 (inferior izquierdo)
                verts.push_back(x0); verts.push_back(yBot); verts.push_back(0.0f);
                verts.push_back(x1); verts.push_back(yBot); verts.push_back(0.0f);
                verts.push_back(x0); verts.push_back(yTop); verts.push_back(0.0f);

                // Triangulo 2 (superior derecho)
                verts.push_back(x1); verts.push_back(yBot); verts.push_back(0.0f);
                verts.push_back(x1); verts.push_back(yTop); verts.push_back(0.0f);
                verts.push_back(x0); verts.push_back(yTop); verts.push_back(0.0f);
            }
        }
    }

    return verts;
}

/*
   FUNCION: Sube los vertices de una letra a su VAO/VBO correspondiente
*/
void CrearLetraVAO(int indice, const std::vector<GLfloat>& verts)
{
    glGenVertexArrays(1, &VAO[indice]);
    glBindVertexArray(VAO[indice]);

    glGenBuffers(1, &VBO[indice]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[indice]);

    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vertexCount[indice] = (GLsizei)(verts.size() / 3);
}

/*
   FUNCION: Agregar Shader al Programa
*/
void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
    GLuint theShader = glCreateShader(shaderType);

    const GLchar* theCode[1];
    theCode[0] = shaderCode;

    GLint codeLength[1];
    codeLength[0] = strlen(shaderCode);

    glShaderSource(theShader, 1, theCode, codeLength);
    glCompileShader(theShader);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
        printf("Error al compilar el shader %d: %s \n", shaderType, eLog);
        return;
    }

    glAttachShader(theProgram, theShader);
}

/*
   FUNCION: Compilar y Linkear Shaders
*/
void CompileShaders()
{
    shader = glCreateProgram();
    if (!shader)
    {
        printf("Error creando el shader");
        return;
    }

    AddShader(shader, vShader, GL_VERTEX_SHADER);
    AddShader(shader, fShader, GL_FRAGMENT_SHADER);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glLinkProgram(shader);

    glGetProgramiv(shader, GL_LINK_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        printf("Error al linkear: %s \n", eLog);
        return;
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        printf("Error al validar: %s \n", eLog);
        return;
    }
}

/*
   FUNCION: Generar Color Aleatorio
*/
void GenerarColorAleatorio()
{
    bgR = (float)(rand() % 256) / 255.0f;
    bgG = (float)(rand() % 256) / 255.0f;
    bgB = (float)(rand() % 256) / 255.0f;
}

/*
   FUNCION: Actualizar Color de Fondo
*/
void ActualizarColorFondo(float deltaTime)
{
    colorTimer += deltaTime;

    if (colorTimer >= 2.0f)
    {
        colorTimer = 0.0f;
        GenerarColorAleatorio();
    }
}

/*
   FUNCION PRINCIPAL: main() uwu
*/
int main()
{
    srand((unsigned int)time(NULL));
    GenerarColorAleatorio();

    if (!glfwInit())
    {
        printf("Fallo inicializar GLFW");
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT,
        "Iniciales AGA",
        NULL, NULL);

    if (!mainWindow)
    {
        printf("Fallo en crearse la ventana con GLFW");
        glfwTerminate();
        return 1;
    }

    int BufferWidth, BufferHeight;
    glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);

    glfwMakeContextCurrent(mainWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Fallo inicializacion de GLEW");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    glViewport(0, 0, BufferWidth, BufferHeight);
    glDisable(GL_CULL_FACE);

    // --- Calculo del layout de las 3 letras, centradas en la pantalla 
    float cellW = 0.08f;                 // ancho de cada "pixel" de la letra
    float cellH = 0.10f;                 // alto de cada "pixel" de la letra
    float gapLetras = 0.15f;             // espacio horizontal entre letras
    float anchoLetra = COLUMNAS * cellW; // ancho total de una letra
    float altoLetra = FILAS * cellH;     // alto total de una letra

    float startX = -(3 * anchoLetra + 2 * gapLetras) / 2.0f; // borde izquierdo del conjunto AGA
    float baseY = -altoLetra / 2.0f;                          // borde inferior de las letras

    auto vertsA1 = GenerarVerticesLetra(letraA, startX, baseY, cellW, cellH);
    auto vertsG = GenerarVerticesLetra(letraG, startX + (anchoLetra + gapLetras), baseY, cellW, cellH);
    auto vertsA2 = GenerarVerticesLetra(letraA, startX + 2 * (anchoLetra + gapLetras), baseY, cellW, cellH);

    CrearLetraVAO(0, vertsA1);
    CrearLetraVAO(1, vertsG);
    CrearLetraVAO(2, vertsA2);

    CompileShaders();

    double lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(mainWindow))
    {
        double currentTime = glfwGetTime();
        deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        ActualizarColorFondo(deltaTime);
        glfwPollEvents();

        glClearColor(bgR, bgG, bgB, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
        GLint colorLocation = glGetUniformLocation(shader, "uniColor");
        glUniform3f(colorLocation, 1.0f, 0.5f, 0.0f); // Color naranja, mismo para las 3 letras

        for (int i = 0; i < 3; i++)
        {
            glBindVertexArray(VAO[i]);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount[i]);
            glBindVertexArray(0);
        }

        glUseProgram(0);
        glfwSwapBuffers(mainWindow);
    }

    return 0;
}