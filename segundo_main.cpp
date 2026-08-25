/*
   PROGRAMA: Rombo y Trapecio con cambio de color de fondo
   DESCRIPCION: Dibuja un rombo a la izquierda y un trapecio
                isosceles a la derecha. El fondo cambia ciclicamente
                entre rojo, verde y azul cada 2 segundos.
*/

#include <stdio.h>      
#include <string.h>     
#include <glew.h>       
#include <glfw3.h>      

/*
   CONFIGURACION DE LA VENTANA
*/
const int WIDTH = 1024, HEIGHT = 768;

/*
   VARIABLES GLOBALES
*/
GLuint VAO[3], VBO[3], shader;
float colorTimer = 0.0f;
int colorState = 0;

/*
   SHADERS (Programas que corren en la GPU)
*/

/*
   VERTEX SHADER: Procesa cada vertice de la figura
*/
static const char* vShader = "						\n\
#version 330										\n\
layout (location = 0) in vec3 pos;					\n\
void main()											\n\
{													\n\
	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f); \n\
}";

/*
   FRAGMENT SHADER: Calcula el color de cada pixel
*/
static const char* fShader = "						\n\
#version 330										\n\
uniform vec3 uniColor;								\n\
out vec4 color;										\n\
void main()											\n\
{													\n\
	color = vec4(uniColor, 1.0f);	 				\n\
}";

/*
   FUNCION: Crear Rombo
   Dibuja un rombo formado por 2 triangulos
   Posicionado en la parte izquierda de la pantalla
*/
void CrearRombo()
{
    GLfloat vertices[] = {
        /* TRIANGULO 1 (Mitad superior del rombo) */
        -0.35f, 0.0f, 0.0f,
        -0.65f, 0.0f, 0.0f,
        -0.50f, 0.35f, 0.0f,

        /* TRIANGULO 2 (Mitad inferior del rombo) */
        -0.50f, -0.35f, 0.0f,
        -0.65f, 0.0f, 0.0f,
        -0.35f, 0.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/*
   FUNCION: Crear Trapecio Isosceles
   Dibuja un trapecio formado por 2 triangulos
   Posicionado en la parte derecha de la pantalla
*/
void CrearTrapecio()
{
    GLfloat vertices[] = {
        /* TRIANGULO 1 (Mitad izquierda del trapecio) */
        0.35f, -0.35f, 0.0f,
        0.65f, 0.35f, 0.0f,
        0.50f, 0.35f, 0.0f,

        /* TRIANGULO 2 (Mitad derecha del trapecio) */
        0.35f, -0.35f, 0.0f,
        0.80f, -0.35f, 0.0f,
        0.65f, 0.35f, 0.0f
    };

    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/*
   FUNCION: Agregar Shader al Programa
   Compila un shader y lo adjunta al programa
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

    /* Verificacion de errores */
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
   Crea el programa de shaders y enlaza vertex + fragment
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

    /* Verificacion de errores */
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
   FUNCION: Actualizar Color de Fondo
   Cambia el color entre Rojo, Verde y Azul cada 2 segundos
*/
void ActualizarColorFondo(float deltaTime)
{
    colorTimer += deltaTime;

    if (colorTimer >= 2.0f)
    {
        colorTimer = 0.0f;
        colorState = (colorState + 1) % 3;
    }
}

/*
   FUNCION: Obtener Color de Fondo Actual
   Devuelve el color correspondiente al estado actual
*/
void ObtenerColorFondo(float& r, float& g, float& b)
{
    switch (colorState)
    {
    case 0: /* ROJO */
        r = 1.0f; g = 0.0f; b = 0.0f;
        break;
    case 1: /* VERDE */
        r = 0.0f; g = 1.0f; b = 0.0f;
        break;
    case 2: /* AZUL */
        r = 0.0f; g = 0.0f; b = 1.0f;
        break;
    }
}

/*
   FUNCION PRINCIPAL: main()
   Inicializa GLFW, GLEW, crea ventana y loop principal
*/
int main()
{
    /*
       INICIALIZAR GLFW
    */
    if (!glfwInit())
    {
        printf("Fallo inicializar GLFW");
        glfwTerminate();
        return 1;
    }

    /* Configurar la version de OpenGL a usar (4.3 Core Profile) */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /*
       CREAR VENTANA
    */
    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT,
        "Rombo y Trapecio",
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

    /*
       INICIALIZAR GLEW
    */
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Fallo inicializacion de GLEW");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    /* Definir el area de dibujo en la ventana */
    glViewport(0, 0, BufferWidth, BufferHeight);

    /* Desactivar el face culling para ver ambos lados de los triangulos */
    glDisable(GL_CULL_FACE);

    /*
       CREAR FIGURAS Y COMPILAR SHADERS
    */
    CrearRombo();
    CrearTrapecio();
    CompileShaders();

    /*
       VARIABLES PARA CONTROL DEL TIEMPO
    */
    double lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    /*
       LOOP PRINCIPAL
    */
    while (!glfwWindowShouldClose(mainWindow))
    {
        /* Calcular el tiempo transcurrido desde el ultimo frame */
        double currentTime = glfwGetTime();
        deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        /* Actualizar el color de fondo */
        ActualizarColorFondo(deltaTime);

        /* Obtener el color de fondo actual */
        float bgR, bgG, bgB;
        ObtenerColorFondo(bgR, bgG, bgB);

        /* Procesar eventos del usuario */
        glfwPollEvents();

        /* Limpiar la pantalla con el color de fondo */
        glClearColor(bgR, bgG, bgB, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Usar el programa de shaders */
        glUseProgram(shader);

        /* Obtener la ubicacion del uniform de color en el shader */
        GLint colorLocation = glGetUniformLocation(shader, "uniColor");

        /* DIBUJAR ROMBO (color blanco) */
        glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        /* DIBUJAR TRAPECIO (color negro) */
        glUniform3f(colorLocation, 0.0f, 0.0f, 0.0f);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        /* Desactivar el programa de shaders */
        glUseProgram(0);

        /* Intercambiar buffers para mostrar lo dibujado */
        glfwSwapBuffers(mainWindow);
    }

    return 0;
}