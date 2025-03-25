#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <memory>
#include <bitset>
#include <vector>
#include <math.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

#include "shaders.h"
#include "gradientnoise.h"
#include "base.h"

#define DEG2RAD 0.01745329252
#define DEFAULT_W 1600
#define DEFAULT_H 1200
#define XDIM 256
#define YDIM 64
#define ZDIM 256

unsigned int compileShader(const char *vertexShaderSource, const char *fragmentShaderSource);

struct SquareData {
    float pos[3];
    int type;
    int side;
};

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, DEFAULT_W, DEFAULT_H);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);
    switch (action)
    {
    case GLFW_PRESS:
        switch (key)
        {
            case GLFW_KEY_W: engine->setPlayerMoving(PlayerMovement::Forward, true); break;
            case GLFW_KEY_A: engine->setPlayerMoving(PlayerMovement::Left, true); break;
            case GLFW_KEY_S: engine->setPlayerMoving(PlayerMovement::Backwards, true); break;
            case GLFW_KEY_D: engine->setPlayerMoving(PlayerMovement::Right, true); break;
            case GLFW_KEY_SPACE: engine->setPlayerMoving(PlayerMovement::Jump, true); break;
            default: break;
        }
        break;
    case GLFW_RELEASE:
        switch (key)
        {
            case GLFW_KEY_W: engine->setPlayerMoving(PlayerMovement::Forward, false); break;
            case GLFW_KEY_A: engine->setPlayerMoving(PlayerMovement::Left, false); break;
            case GLFW_KEY_S: engine->setPlayerMoving(PlayerMovement::Backwards, false); break;
            case GLFW_KEY_D: engine->setPlayerMoving(PlayerMovement::Right, false); break;
            default: break;
        }
        break;
    default:
        break;
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);
    engine->cursorMoved(xpos, ypos);
}

int main(void) {
    //GLFW & GLAD initialisation
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(DEFAULT_W, DEFAULT_H, "glortVox", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    //Engine initialisation
    EngineInitData initData;
    initData.playerDimensions = {0.5f, 2.0f, 0.5f};
    initData.spawnPoint = {128.0f, 48.0f, 128.0f};
    initData.mapDimensionsXYZ[0] = XDIM;
    initData.mapDimensionsXYZ[1] = YDIM;
    initData.mapDimensionsXYZ[2] = ZDIM;
    Engine engine(initData);

    //Matrices, vectors. TODO: create constexpr rotate function 
    glm::mat4 rotations[6];
    rotations[0] = glm::mat4(1.0f);
    rotations[1] = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rotations[2] = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rotations[3] = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotations[4] = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotations[5] = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //Perspective projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)DEFAULT_W / (float)DEFAULT_H, 0.1f, 200.0f);
    
    //Generate heightmap
    float hMap[256*256];
    fractalNoise(hMap, 32, 8, 5, 1.5, 0.5);

    //Map initialisation
    engine.loadHeightmap(hMap, 48);
    std::vector<SquareData> blockData;

    //Generate visible faces
    for (int x = 0; x < XDIM; x++)
    for (int y = 0; y < YDIM; y++)
    for (int z = 0; z < ZDIM; z++) {
        //Skip if no block at location
        if (!engine.getMap().at(x,y,z)) continue;
        //Get surrounding blocks
        std::bitset<6> s = engine.getMap().surroundingBlocks(x, y, z);
        //Skip if block is entirely surrounded
        if (s.all()) continue;
        //2 corresponds to block above. if there is no block above, make block a grass block.
        //TODO: create enum for surrounding block values, change bitset to typedef'd char.
        int type = s[2] ? 1 : 0;
        //push back square for each visible (i.e. not covered) face.
        for (int j = 0; j < 6; j++)
            if (!s[j]) blockData.push_back({{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)}, type, j});
    }

    std::cout << blockData.size() << " visible faces.\r\n";

    //Window setup
    glfwSetWindowUserPointer(window, &engine);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glViewport(0, 0, DEFAULT_W, DEFAULT_H);

    //Square vertices, left face
    const float square[] = {
        //xyz coords            //texture coords
        -0.5f,  0.5f,   0.5f,   1.0f,   0.0f,
        -0.5f,  0.5f,   -0.5f,  0.0f,   0.0f,
        -0.5f,  -0.5f,  -0.5f,  0.0f,   1.0f,
        -0.5f,  -0.5f,  -0.5f,  0.0f,   1.0f,
        -0.5f,  -0.5f,  0.5f,   1.0f,   1.0f,
        -0.5f,  0.5f,   0.5f,   1.0f,   0.0f
    };

    //Declare Buffers
    unsigned int blockVAO, squareVBO, squareDataIBO;

    //Generate VAO & VBOs
    glGenVertexArrays(1, &blockVAO);
    glGenBuffers(1, &squareVBO);
    glGenBuffers(1, &squareDataIBO);

    //Bind VAO & VBO
    glBindVertexArray(blockVAO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    //Vertex positon
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Bind instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, squareDataIBO);
    glBufferData(GL_ARRAY_BUFFER, blockData.size() * sizeof(SquareData), blockData.data(), GL_STATIC_DRAW);
    //Block Position
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SquareData), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);
    //Block type
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(SquareData), (void*)(sizeof(int)*3));
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);
    //Side
    glVertexAttribIPointer(4, 1, GL_INT, sizeof(SquareData), (void*)(4*sizeof(int)));
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(4);

    //Set up texture buffer
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Load texture map
    int width, height, chans;
    unsigned char *data = stbi_load("textures.png", &width, &height, &chans, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    //Block shader
    unsigned int shaderProgram = compileShader(blockVert, blockFrag);

    //Setting state variables
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //Set background to sky blue :-)
    glClearColor(0.8f, 1.0f, 1.0f, 1.0f);

    //Main loop
    while (!glfwWindowShouldClose(window))
    {
        //Get time from start of frame, swap buffers & clear
        double time = glfwGetTime();
        glfwSwapBuffers(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Get matrix locations
        int modelMat = glGetUniformLocation(shaderProgram, "model");
        int viewMat = glGetUniformLocation(shaderProgram, "view");
        int projMat = glGetUniformLocation(shaderProgram, "projection");
        int rotMatAr = glGetUniformLocation(shaderProgram, "rotations");

        //Bind program, set uniform values & bind cube vertex array
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(engine.getCamera()));
        glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(rotMatAr, 6, GL_FALSE, glm::value_ptr(rotations[0]));
        glBindVertexArray(blockVAO);

        //Draw map
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, blockData.size());

        //Poll events
        glfwPollEvents();

        //Action events
        engine.update();
        
        //Wait for frame
        while (glfwGetTime() < time + 1.0 / MAX_FPS) {}
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

//Shader compiling utility function, adapted from Learn OpenGL by Joey de Vries
unsigned int compileShader(const char *vertexShaderSource, const char *fragmentShaderSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, vertexShader);

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}