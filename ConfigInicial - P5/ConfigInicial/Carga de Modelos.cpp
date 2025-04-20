// Std. Includes
#include <string>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// Properties
const GLint WIDTH = 1200, HEIGHT = 800;
int screenWidth, screenHeight;

// Function prototypes

// AGREGAR 1.0
void Inputs(GLFWwindow* window);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

float movX = 0.0f;
float movY = 0.0f;
float movZ = -5.0f;
float roty = 0.0f;
float rotx = 0.0f;
float rotz = 0.0f;

bool ctrlPressed = false; // Track if Ctrl is pressed
bool rightMousePressed = false; // Track if right mouse button is pressed
bool leftMousePressed = false; // Track if right mouse button is pressed
double lastX = WIDTH / 2.0; // Last X position of the mouse
double lastY = HEIGHT / 2.0; // Last Y position of the mouse
double lastZ = WIDTH / 2.0; // Last Y position of the mouse

//For Keyboard
float	movX_fig = 0.0f,
movY_fig = 0.0f,
movZ_fig = -5.0f;

// Camera

////Como vamos a ir generando los movimientos
//Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
//bool keys[1024];
//GLfloat lastX = 400, lastY = 300;
//bool firstMouse = true;
//
//GLfloat deltaTime = 0.0f;
//GLfloat lastFrame = 0.0f;



int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Carga de modelos y camara sintetica", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);


    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Define the viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight);

    // OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup and compile our shaders
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Load models
    Model dog((char*)"Models/A.obj");
    glm::mat4 projection = glm::mat4(1);

    projection = glm::perspective(glm::radians(45.0f), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.01f, 1000000.0f);//FOV, Radio de aspecto,znear,zfar
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);


    //----------------------------------------------INICIO RETICULA Y EJES.--------------
    // 

    float axisVertices[] = {
        // Ejes coordenados
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Origen a X (rojo)
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Origen a Y (azul)
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Origen a Z (verde)
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    };


    // Crear un VAO y un VBO para los ejes
    GLuint axisVBO, axisVAO;
    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);

    // Vincular el VAO y el VBO para los ejes
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

    // Configurar el atributo de posición para los ejes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Configurar el atributo de color para los ejes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Desvincular el VBO y el VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    //Inicio de la retícula--------------
    // Definir los vértices de la retícula (una cuadrícula en el plano XZ)
    std::vector<float> gridVertices;
    float gridSize = 10.0f;       // Tamaño de la retícula (de -1 a 1 en X y Z)
    float gridStep = 0.9999f;    // Separación entre líneas
    float gridColor = 0.2f;      // Color gris para la retícula

    // Líneas paralelas al eje X
    for (float z = -gridSize; z <= gridSize; z += gridStep) {
        gridVertices.push_back(-gridSize); gridVertices.push_back(0.0f); gridVertices.push_back(z);
        gridVertices.push_back(gridColor); gridVertices.push_back(gridColor); gridVertices.push_back(gridColor);
        gridVertices.push_back(gridSize); gridVertices.push_back(0.0f); gridVertices.push_back(z);
        gridVertices.push_back(gridColor); gridVertices.push_back(gridColor); gridVertices.push_back(gridColor);
    }

    // Líneas paralelas al eje Z
    for (float x = -gridSize; x <= gridSize; x += gridStep) {
        gridVertices.push_back(x); gridVertices.push_back(0.0f); gridVertices.push_back(-gridSize);
        gridVertices.push_back(gridColor); gridVertices.push_back(gridColor); gridVertices.push_back(gridColor);
        gridVertices.push_back(x); gridVertices.push_back(0.0f); gridVertices.push_back(gridSize);
        gridVertices.push_back(gridColor); gridVertices.push_back(gridColor); gridVertices.push_back(gridColor);
    }

    // Crear un VAO y un VBO para la retícula
    GLuint gridVBO, gridVAO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    // Vincular el VAO y el VBO para la retícula
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    // Configurar el atributo de posición para la retícula
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Configurar el atributo de color para la retícula
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Desvincular el VBO y el VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // -------------------------------------------------- Final de la retícula Y EJES--------------




    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);



    // Game loop
    while (!glfwWindowShouldClose(window))
    {

        Inputs(window);




        // Check and call events
        glfwPollEvents();
        //DoMovement();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // blanco

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        shader.Use();


        glm::mat4 view = glm::mat4(1);
        view = glm::translate(view, glm::vec3(movX, movY, movZ - 10.0f));
        view = glm::rotate(view, glm::radians(roty + -40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(-rotx + 30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(rotz + -20.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Draw the loaded model

        glm::mat4 model(1);
        //glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //dog.Draw(shader);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));



        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        dog.Draw(shader);



        //-------------------------DIUBJAR EJES Y RETICULA
        // Dibujar los ejes sin rotación (matriz de modelo identidad)
        glm::mat4 axisModel = glm::mat4(1.0f); // Matriz de modelo sin rotación
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(axisModel));

        //R
            //A
            //V
        // Dibujar los ejes
        glBindVertexArray(axisVAO);
        color = glm::vec3(1.0f, 0.0f, 0.0f); //ROJO
        glUniform3fv(shader.getColorLocation(), 1, glm::value_ptr(color));
        glDrawArrays(GL_LINES, 0, 2); // 6 vértices para los 3 ejes

        color = glm::vec3(0.0f, 0.0f, 1.0f); // BLUE
        glUniform3fv(shader.getColorLocation(), 1, glm::value_ptr(color));
        glDrawArrays(GL_LINES, 2, 2); // 6 vértices para los 3 ejes

        color = glm::vec3(0.0f, 1.0f, 0.0f); // GREEN
        glUniform3fv(shader.getColorLocation(), 1, glm::value_ptr(color));
        glDrawArrays(GL_LINES, 4, 2); // 6 vértices para los 3 ejes

        glBindVertexArray(0);




        // Dibujar LA RETICULA

        glm::mat4 gridModel = glm::mat4(1.0f); // Matriz de modelo para la retícula (sin transformación)
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(gridModel));
        glBindVertexArray(gridVAO);


        //color = glm::vec3(0.75f, 0.75f, 0.75f);
        color = glm::vec3(.1f, .1f, .10f);
        glUniform3fv(shader.getColorLocation(), 1, glm::value_ptr(color));
        glDrawArrays(GL_LINES, 0, gridVertices.size() / 6); // Dibujar líneas
        glBindVertexArray(0);


        //-------------------------------FIN  EJES Y RETICULA


        // Swap the buffers
        glfwSwapBuffers(window);
    }



    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &axisVBO);
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVAO);


    glfwTerminate();
    return 0;
}



void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)  //GLFW_RELEASE
        glfwSetWindowShouldClose(window, true);


    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Track Ctrl key state
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        ctrlPressed = true;
    else
        ctrlPressed = false;
}



void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.5f; // Adjust sensitivity as needed
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    if (ctrlPressed && rightMousePressed) {


        // Rotate around Z axis (Ctrl + Left Click)
        movX += xoffset / 10.0f; // Adjust the factor as needed
        movY += yoffset / 10.0f; // Adjust the factor as needed
        //std::cout << "Ctrl + Left Click: rotz = " << rotz << std::endl;

    }
    if (ctrlPressed && leftMousePressed) {
        // Rotate around Z axis (Ctrl + Left Click)
        rotz += (xoffset + yoffset) * 0.5f; // Adjust the factor as needed
        //std::cout << "Ctrl + Left Click: rotz = " << rotz << std::endl;
    }

    if (leftMousePressed && !ctrlPressed) {
        // Rotate around X and Y axes (Ctrl + Right Click)
        rotx += yoffset; // Rotate around the X-axis (vertical movement)
        roty += xoffset; // Rotate around the Y-axis (horizontal movement)

        // std::cout << "Ctrl + Right Click: rotx = " << rotx << ", roty = " << roty << std::endl;

    }

}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {


            rightMousePressed = true;
        }
        else if (action == GLFW_RELEASE)
            rightMousePressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {


            leftMousePressed = true;
        }
        else if (action == GLFW_RELEASE)
            leftMousePressed = false;
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    movZ += yoffset * 5.0f; // Adjust the zoom speed as needed
}
