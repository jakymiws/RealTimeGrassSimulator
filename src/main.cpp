#include <glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <vector>

#include "LoadShaders.h"
#include "camera.h"

struct IndirectRenderParams
{
    GLuint count;
    GLuint primCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// timing
bool debugMode = false;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastTime = 0.0f;
int numFrames = 0;
 
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

glm::vec3 lightPos(0.0f, 1.0f, 3.0f);

int num_blades = 200000;

struct Blade
{
    glm::vec3 up;
    glm::vec3 dir;
    float height;
    float width;
    float stiffness;
};
float sc = 4.0f;
const float MIN_HEIGHT = 1.3f/sc;
const float MAX_HEIGHT = 2.5f/sc;
const float MIN_WIDTH = 0.1f/sc;
const float MAX_WIDTH = 0.14f/sc;
const float MIN_BEND = 7.0f;
const float MAX_BEND = 13.0f;

const float planeDim = 10.0f;

float zTrans = 0.0f;
float xTrans = 0.0f;
float qScale = 2.0f;

unsigned int grassPosBuffer, grassV1Buffer, grassV2Buffer, grassPropBuffer, grassAgeBuffer;
unsigned int grassVBO_Indirect;

int debugSwitch = 0;

int numSphereColliders = 5;
std::vector<glm::vec4> sphereColliders;

glm::mat4 sphereModel = glm::mat4(1.0f);
glm::mat4 gProj = glm::mat4(1.0f);
glm::mat4 gView = glm::mat4(1.0f);

glm::vec3 sPos = glm::vec3(0.0, 0.1f, 0.0);

float randomFloat()
{
    return (rand() / (float)RAND_MAX);
}

float randomFloatRange(float a, float b)
{
    float f = randomFloat();
    float diff = b-a;
    float r = f*diff;
    return a+r;
}

int main(void)
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Real Time Grass Simulator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
  
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwSwapInterval(1);

    glewInit();

    glEnable(GL_DEPTH_TEST);

    ShaderInfo light_shader_info[] = {
        {GL_VERTEX_SHADER, "../shaders/light.vs"},
        {GL_FRAGMENT_SHADER, "../shaders/light.fs"},
        {GL_NONE, NULL}
    };

    GLuint light_shader_program = LoadShaders(light_shader_info);

    printf("light shader program: %d \n", light_shader_program);

float verts[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

    //light cube
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //collision test object
    ShaderInfo coll_shader_info[] = {
        {GL_VERTEX_SHADER, "../shaders/coll.vs"},
        {GL_FRAGMENT_SHADER, "../shaders/coll.fs"},
        {GL_NONE, NULL}
    };

    GLuint coll_shader_program = LoadShaders(coll_shader_info);

    printf("coll shader program: %d \n", coll_shader_program);

    unsigned int collVAO, collVBO;
    glGenVertexArrays(1, &collVAO);
    glGenBuffers(1, &collVBO);

    glBindVertexArray(collVAO);
    glBindBuffer(GL_ARRAY_BUFFER, collVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    for (int i = 0; i < numSphereColliders; i++)
    {
        //sphereColliders.push_back(glm::vec4(0.0, 0.1, 0.0, 1.0));
        sphereColliders.push_back(glm::vec4(sPos, 1.0f));
    }

    // glUseProgram(coll_shader_program);


    // glm::vec3 spherePos = glm::vec3(sphereColliders[0].x, sphereColliders[0].y, sphereColliders[0].z);
    // glm::mat4 cmodel = glm::mat4(1.0f);
    // cmodel = glm::translate(cmodel, spherePos);
    // cmodel = glm::scale(cmodel, glm::vec3(0.2f));

    // glm::mat4 sphereScale = glm::mat4(1.0f);
    // sphereScale = glm::scale(sphereScale, glm::vec3(0.2f));

    // sphereColliders[0] = sphereScale * sphereColliders[0];

    // glUniformMatrix4fv(glGetUniformLocation(coll_shader_program, "model"), 1, GL_FALSE, &cmodel[0][0]);

    //grass blades 
    ShaderInfo grass_shader_info[] = {
        {GL_VERTEX_SHADER, "../shaders/grass.vs"},
        {GL_FRAGMENT_SHADER, "../shaders/grass.fs"},
        {GL_TESS_CONTROL_SHADER, "../shaders/grass.tc"},
        {GL_TESS_EVALUATION_SHADER, "../shaders/grass.te"},
        {GL_NONE, NULL}
    };

    GLuint grass_shader_program = LoadShaders(grass_shader_info);

    printf("grass shader program: %d \n", grass_shader_program);

    //Generate grass blades
    std::vector<glm::vec4> grass_blade_positions;
    std::vector<glm::vec4> grass_blade_v1s;
    std::vector<glm::vec4> grass_blade_v2s;
    std::vector<glm::vec4> grass_blade_props;//y = height, z = width, w = stiffness
    std::vector<float> grass_ages;
   // std::vector<glm::vec3> grass_blade_directions;
    for (int i = 0; i < num_blades; i++)
    {
        float grass_height = MIN_HEIGHT + (randomFloat() * (MAX_HEIGHT - MIN_HEIGHT));
        float grass_width = MIN_WIDTH + (randomFloat() * (MAX_WIDTH - MIN_WIDTH));

        float blade_stiffness = MIN_BEND + (randomFloat() * (MAX_BEND - MIN_BEND));
            
        float xCoord = (randomFloat() - 0.5f) * planeDim;
        float zCoord = (randomFloat() - 0.5f) * planeDim;

        glm::vec3 grass_up = glm::vec3(0, 1.0f, 0);

        glm::vec3 controlPoint0 = glm::vec3(xCoord, 0.0, zCoord);
        glm::vec3 controlPoint1 = controlPoint0 + grass_height*grass_up;
        glm::vec3 controlPoint2 = controlPoint1;
        
        float blade_dir = randomFloat() * 2.0f * M_PI;
        float grass_age = randomFloatRange(0.1, 0.3);

        grass_blade_positions.push_back(glm::vec4(controlPoint0, blade_dir));
        grass_blade_v1s.push_back(glm::vec4(controlPoint1, grass_height));
        grass_blade_v2s.push_back(glm::vec4(controlPoint2, grass_width));
        grass_blade_props.push_back(glm::vec4(grass_up, blade_stiffness));
        grass_ages.push_back(grass_age);
    }

    unsigned int grassVAO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO_Indirect);
    glGenBuffers(1, &grassPosBuffer);
    glGenBuffers(1, &grassV1Buffer);
    glGenBuffers(1, &grassV2Buffer);
    glGenBuffers(1, &grassPropBuffer);
    glGenBuffers(1, &grassAgeBuffer);

    glBindVertexArray(grassVAO);

    glBindBuffer(GL_ARRAY_BUFFER, grassPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(glm::vec4), grass_blade_positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    glBindBuffer(GL_ARRAY_BUFFER, grassV1Buffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(glm::vec4), grass_blade_v1s.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    glBindBuffer(GL_ARRAY_BUFFER, grassV2Buffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(glm::vec4), grass_blade_v2s.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    glBindBuffer(GL_ARRAY_BUFFER, grassPropBuffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(glm::vec4), grass_blade_props.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    glBindBuffer(GL_ARRAY_BUFFER, grassAgeBuffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(float), grass_ages.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    int grassTexWidth, grassTexHeight, grassTexNumChannels;
    unsigned char *grassTexData = stbi_load("../assets/GrassDiffuse.png", &grassTexWidth, &grassTexHeight, &grassTexNumChannels, 4);
    
    printf("width: %d height %d num channels: %d\n", grassTexWidth, grassTexHeight, grassTexNumChannels);

    unsigned int grassTex;
    glGenTextures(1, &grassTex);
    glBindTexture(GL_TEXTURE_2D, grassTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grassTexWidth, grassTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, grassTexData);
    
    std::cout << "error status:" << glGetError() << std::endl;

    stbi_image_free(grassTexData);

    // //Indirect----
    IndirectRenderParams indirectRenderParams = { (GLuint)num_blades, (GLuint)1, (GLuint)0, (GLuint)0, (GLuint)0 };
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, grassVBO_Indirect);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(IndirectRenderParams), &indirectRenderParams, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);//probably don't need this but to be safe keep it in.
    // //End Indirect----

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    //ground plane
    float quad_vertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f
    };

    ShaderInfo ground_plane_shader_info[] = {
        {GL_VERTEX_SHADER, "../shaders/ground.vs"},
        {GL_FRAGMENT_SHADER, "../shaders/ground.fs"},
        {GL_NONE, NULL}
    };

    GLuint ground_shader_program = LoadShaders(ground_plane_shader_info);

    printf("ground shader program: %d \n", ground_shader_program);

    unsigned int groundVAO, groundVBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Compute 
    ShaderInfo force_compute_shader_info[] = {
        {GL_COMPUTE_SHADER, "../shaders/grass_force.comp"},
        {GL_NONE, NULL}
    };

    GLuint force_compute_program = LoadShaders(force_compute_shader_info);
    printf("force compute shader program: %d \n", force_compute_program);

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        numFrames++;
        if (deltaTime >= 1.0)
        {   
            if (debugMode)
            {
                printf("fps = %f\n", (float)numFrames);
                printf("mpf = %f\n", 1000.0f/(float)numFrames);
            }
            
            numFrames = 0;
            lastTime = currentTime;
        }
        
        processInput(window);

        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        //sphereColliders[0] = sphereModel * glm::vec4(0.0, 0.1, 0.0, 1.0f) + glm::vec4(1.0, 0.0, 1.0, 0.0);
        sphereColliders[0] = glm::vec4(sPos,1.0f);

        //compute forces then draw
        glUseProgram(force_compute_program);

        glUniform1f(glGetUniformLocation(force_compute_program, "frameNum"), (float)currentTime);
        glUniform1i(glGetUniformLocation(force_compute_program, "numSphereColliders"), (int)sphereColliders.size());
        glUniform4fv(glGetUniformLocation(force_compute_program, "sphereColliders[0]"), (int)sphereColliders.size(), (const GLfloat*)sphereColliders.data());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, grassPosBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grassV1Buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, grassV2Buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, grassPropBuffer);

        glDispatchCompute((num_blades/16) + 1, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //Display light cube
        glUseProgram(light_shader_program);

        glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //End Display light cube

        //Display coll object
        glUseProgram(coll_shader_program);

        glUniformMatrix4fv(glGetUniformLocation(coll_shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(coll_shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        
        glm::vec3 spherePos = glm::vec4(xTrans, 0.1, zTrans, 1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, spherePos);
        model = glm::scale(model, glm::vec3(0.2f));
        glm::vec4 tmp = glm::vec4(spherePos,1.0f) * glm::inverse(model);
        sPos = glm::vec3(tmp.x, tmp.y, tmp.z);

        glUniformMatrix4fv(glGetUniformLocation(coll_shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(collVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //End Display coll object

        //Display ground plane
        glUseProgram(ground_shader_program);

        glUniformMatrix4fv(glGetUniformLocation(ground_shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(ground_shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        
        model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(xTrans, 0, zTrans));
        model = glm::scale(model, glm::vec3(qScale));
        model = glm::rotate(model, 1.5708f, glm::vec3(1,0,0)); //1.5708 radians = 90 degrees

        glUniformMatrix4fv(glGetUniformLocation(ground_shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(groundVAO);
        glDrawArrays(GL_QUADS, 0, 4);
        //End Display ground plane

        //Display tessellated quad
        glUseProgram(grass_shader_program);

        glBindTexture(GL_TEXTURE_2D, grassTex);
        glUniform1i(glGetUniformLocation(grass_shader_program, "diffuseTexture"), 0);

        glBindVertexArray(grassVAO);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, grassVBO_Indirect);

        glUniformMatrix4fv(glGetUniformLocation(grass_shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(grass_shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(glGetUniformLocation(grass_shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glUniform3fv(glGetUniformLocation(grass_shader_program, "lightPosition"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(grass_shader_program, "cameraPosition"), 1, &camera.Position[0]);
        glUniform1i(glGetUniformLocation(grass_shader_program, "debugSwitch"), debugSwitch);

        glDrawArraysIndirect(GL_PATCHES, 0);

        //End display tessellated quad

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow *window)
{
    float offset = 0.01f;
    float dt = 0.01f;

     if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        debugSwitch = 1;
        printf("debugSwitch on: %d \n", debugSwitch);
    }
     if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        debugSwitch = 0;
        printf("debugSwitch on: %d \n", debugSwitch);
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, dt);
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        xTrans += offset;
        printf("X: %f \n", xTrans);
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        xTrans -= offset;
        printf("X: %f \n", xTrans);
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        zTrans += offset;
        printf("Z: %f \n", zTrans);
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    {
        zTrans -= offset;
        printf("Z: %f \n", zTrans);
    }
    // if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    // {
    //     qScale += offset;
    //     printf("qscale: %f \n", qScale);
    // }
    // if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    // {
    //     qScale -= offset;
    //     printf("qscale: %f \n", qScale);
    // }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        debugMode = !debugMode;
        printf("-------------------\n");
    }

        
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}