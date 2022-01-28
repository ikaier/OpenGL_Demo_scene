#define STB_IMAGE_IMPLEMENTATION
// OpenGLApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<stdio.h>
#include<string.h>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<stdlib.h>

#include<vector>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"Window.h"
#include "Mesh.h"
#include "Shader.h"
#include"Camera.h"
#include"Texture.h"
#include"DirectionalLight.h"
#include "Material.h"
#include"PointLight.h"
#include"CommonValues.h"

#include"Model.h"
#include"Skybox.h"




//Window dimensions
const float toRadians = 3.14159265f / 180.0f;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0, uniformSpecularIntensity = 0, uniformShininess = 0,
uniformDirectionalLightTransform = 0, uniformOmniLightPos = 0, uniformFarPlane = 0;

Window mainWindow;
std::vector<Mesh*>meshList;

std::vector<Shader> shaderList;
Shader directionalShadowShader;
Shader omniShadowShader;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture PlainTexture;

Material shinyMaterial;
Material dullMaterial;

Model oldHouse;
Model seaHawk;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Skybox skybox;

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

GLfloat deltaTime2 = 0.0f;
GLfloat lastTime2 = 0.0f;

GLfloat SeaHawkAngle = 0.0f;
GLfloat SpotAngle = 0.0f;

//Vertex shader
static const char* vShader = "Shaders/shader.vert";

//fragment shader
static const char* fShader = "Shaders/shader.frag";

void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount, unsigned int vLength, unsigned int normalOffset) {
    for (size_t i = 0; i < indiceCount; i += 3) {
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
    for (size_t i = 0; i < verticeCount / vLength; i++) {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        //printf("%f, %f,%f\n", vec.x, vec.y,vec.z);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}


void CreateTriangle()
{
    unsigned int indices[] = {
        0,3,1,
        1,3,2,
        2,3,0,
        0,1,2
    };

    GLfloat vertices[] = {
        //  x     y    z    u   v       nx   ny   nz
        -1.0f,-1.0f,-0.6f,   0.0f,0.0f,  0.0f,0.0f,0.0f,
        0.0f,-1.0f,1.0f,    0.5f,0.0f,  0.0f,0.0f,0.0f,
        1.0f,-1.0f,-0.6f,    1.0f,0.0f,  0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,     0.5f,1.0f,  0.0f,0.0f,0.0f
    };

    calcAverageNormals(indices, 12, vertices, 32, 8, 5);

    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj1);

    //Mesh* obj2 = new Mesh();
    //obj2->CreateMesh(vertices, indices, 32, 12);
    //meshList.push_back(obj2);
}

void CreateFloor()
{
    unsigned int Floorindices[] = {
        0,2,1,
        1,2,3
    };

    GLfloat Floorvertices[] = {
        //  x     y     z       u       v       nx   ny   nz
        -10.0f, 0.0f,   -10.0f, 0.0f,   0.0f,   0.0f,   -1.0f,  0.0f,
        10.0f,  0.0f,    -10.0f, 10.0f,  0.0f,   0.0f,   -1.0f,  0.0f,
        -10.0f, 0.0f,   10.0f,  0.0f,   10.0f,  0.0f,   -1.0f,  0.0f,
        10.0f,  0.0f,   10.0f,  10.0f,  10.0f,  0.0f,   -1.0f,  0.0f,
    };


    Mesh* obj3 = new Mesh();
    obj3->CreateMesh(Floorvertices, Floorindices, 32, 6);
    meshList.push_back(obj3);

    //Mesh* obj2 = new Mesh();
    //obj2->CreateMesh(vertices, indices, 32, 12);
    //meshList.push_back(obj2);
}

void CreateCube()
{
    unsigned int indices[] = {
       //front and back
        0,1,2,
        0,2,3,
       
        6,5,4,
        7,6,4,

        //left and right
        9,13,14,
        14,10,9,

       15,12,8,
        8,11,15,

        //top and buttom
        21,17,16,
        16,20,21,

        19,18,22,
        22,23,19
    };

    GLfloat vertices[] = {
        //  x     y    z    u   v        nx   ny   nz
        -1.0f,-1.0f,0.0f,   0.0f,0.0f,   0.0f,0.0f,1.0f,               //left bottom 0
        1.0f,-1.0f,0.0f,    1.0f,0.0f,   0.0f,0.0f,1.0f,               //right bottom 1
        1.0f,1.0f,0.0f,     1.0f,1.0f,   0.0f,0.0f,1.0f,               //right top 2
        -1.0f,1.0f,0.0f,    0.0f,1.0f,   0.0f,0.0f,1.0f,               //left top 3

        -1.0f,-1.0f,1.0f,   0.0f,0.0f,   0.0f,0.0f,-1.0f,                // 4
        1.0f,-1.0f,1.0f,    1.0f,0.0f,   0.0f,0.0f,-1.0f,                // 5
        1.0f,1.0f,1.0f,     1.0f,1.0f,   0.0f,0.0f,-1.0f,                // 6
        -1.0f,1.0f,1.0f,    0.0f,1.0f,   0.0f,0.0f,-1.0f,                //7

        - 1.0f,-1.0f,0.0f,   1.0f,0.0f,   1.0f,0.0f,0.0f,               //left bottom 8
        1.0f,-1.0f,0.0f,     0.0f,0.0f,   -1.0f,0.0f,0.0f,               //right bottom 9
        1.0f,1.0f,0.0f,      0.0f,1.0f,   -1.0f,0.0f,0.0f,               //right top 10
        -1.0f,1.0f,0.0f,     1.0f,1.0f,   1.0f,0.0f,0.0f,               //left top 11

        -1.0f,-1.0f,1.0f,   0.0f,0.0f,   1.0f,0.0f,0.0f,                // 12
        1.0f,-1.0f,1.0f,    1.0f,0.0f,   -1.0f,0.0f,0.0f,                // 13
        1.0f,1.0f,1.0f,     1.0f,1.0f,   -1.0f,0.0f,0.0f,                // 14
        -1.0f,1.0f,1.0f,    0.0f,1.0f,   1.0f,0.0f,0.0f,                //15

        - 1.0f,-1.0f,0.0f,  0.0f,1.0f,   0.0f,1.0f,0.0f,                //left bottom 16
        1.0f,-1.0f,0.0f,    1.0f,1.0f,   0.0f,1.0f,0.0f,                //right bottom 17
        1.0f,1.0f,0.0f,     1.0f,0.0f,   0.0f,-1.0f,0.0f,                //right top 18
        -1.0f,1.0f,0.0f,    0.0f,0.0f,   0.0f,-1.0f,0.0f,                //left top 19

        -1.0f,-1.0f,1.0f,   0.0f,0.0f,   0.0f,1.0f,0.0f,                // 20
        1.0f,-1.0f,1.0f,    1.0f,0.0f,   0.0f,1.0f,0.0f,                // 21
        1.0f,1.0f,1.0f,     1.0f,1.0f,   0.0f,-1.0f,0.0f,                // 22
        -1.0f,1.0f,1.0f,    0.0f,1.0f,   0.0f,-1.0f,0.0f                 //23
    };
    //Mesh* obj1 = new Mesh();
    //obj1->CreateMesh(vertices, indices, 40, 36);
    //meshList.push_back(obj1);
    //calcAverageNormals(indices, 36, vertices, 24*8, 8, 5);
    
    //for (size_t i = 0; i < 8; i++) {
    //    //printf("%f\n", vertices[i * 8 + 5]);
    //    glm::vec3 v1(vertices[i * 8 + 5] + vertices[i * 8 + 64 + 5] + vertices[i * 8 + 64 * 2 + 5], vertices[i * 8 + 6] + vertices[i * 8 + 64 + 6] + vertices[i * 8+ 64 * 2 + 6], vertices[i * 8 + 7] + vertices[i * 8+ 64 + 7] + vertices[i * 8+ 64 * 2 + 7]);
    //    v1 = glm::normalize(v1);
    //    vertices[i * 8 + 5] = v1.x;
    //    
    //    vertices[i * 8 + 64 + 5] = v1.x;
    //    vertices[i * 8 + 64 * 2 + 5] = v1.x;
    //    vertices[i * 8 + 6] = v1.y;
    //    vertices[i * 8 + 64 + 6] = v1.y;
    //    vertices[i * 8 + 64 * 2 + 6] = v1.y;
    //    vertices[i * 8 + 7] = v1.z;
    //    vertices[i * 8 + 64 + 7] = v1.z;
    //    vertices[i * 8 + 64 * 2 + 7] = v1.z;
    //}
    //printf("%f, %f , %f", vertices[5], vertices[6], vertices[7]);
    Mesh* obj2 = new Mesh();
    obj2->CreateMesh(vertices, indices, 24*8, 36);
    meshList.push_back(obj2);
}





void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);

    directionalShadowShader = Shader();
    directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map.vert", "Shaders/directional_shadow_map.frag");
    omniShadowShader.CreateFromFiles("Shaders/omni_shadow_map.vert", "Shaders/omni_shadow_map.geom", "Shaders/omni_shadow_map.frag");

}

void RenderScene() {
    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
    //model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    
    brickTexture.UseTexture();
    shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    meshList[0]->RenderMesh();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 2.0, -2.5f));
    model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    dirtTexture.UseTexture();
    dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    meshList[1]->RenderMesh();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -2.0, 0.0f));
    //model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    dirtTexture.UseTexture();
    shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    meshList[2]->RenderMesh();


    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, -1.7f, 5.0f));
    model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.005f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    oldHouse.RenderModel();

    SeaHawkAngle += 8.1 * deltaTime;
    if (SeaHawkAngle > 360.0f) {
        SeaHawkAngle = 8.1 * deltaTime;
    }

    model = glm::mat4(1.0f);
    model = glm::rotate(model, -SeaHawkAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(10.0f, 3.0f, 0.0f));
    model = glm::rotate(model, 20.0f*toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
    seaHawk.RenderModel();
}

void DirectionalShadowMapPass(DirectionalLight* light) {
    directionalShadowShader.UseShader();

    glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

    light->GetShadowMap()->Write();
    glClear(GL_DEPTH_BUFFER_BIT);

    uniformModel = directionalShadowShader.GetModelLocation();

    glm::mat4 lightTransform = light->CalculateLightTransform();
    directionalShadowShader.SetDirectionalLightTransform(&lightTransform);

    directionalShadowShader.Validate();

    RenderScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OmniShadowMapPass(PointLight* light) {
    

    glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

    omniShadowShader.UseShader();
    uniformModel = omniShadowShader.GetModelLocation();
    uniformOmniLightPos = omniShadowShader.GetOmniLightPosLocation();
    uniformFarPlane = omniShadowShader.GetFarPlaneLocation();

    light->GetShadowMap()->Write();
    glClear(GL_DEPTH_BUFFER_BIT);



    glUniform3f(uniformOmniLightPos, light->GetPosition().x, light->GetPosition().y, light->GetPosition().z);
    glUniform1f(uniformFarPlane, light->GetFarPlane());
    omniShadowShader.SetLightMatrices(light->CalculateLightTransform());

    omniShadowShader.Validate();
    RenderScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass(glm::mat4 projectionMatrix,glm::mat4 viewMatrix) {
    glViewport(0, 0, 1366, 768);

    //clear window
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    skybox.DrawSkybox(viewMatrix, projectionMatrix);
    shaderList[0].UseShader();

    uniformModel = shaderList[0].GetModelLocation();
    uniformProjection = shaderList[0].GetProjectionLocation();
    uniformView = shaderList[0].GetViewLocation();
    uniformEyePosition = shaderList[0].GetEyePositionLocation();
    uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
    uniformShininess = shaderList[0].GetShininessLocation();



    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

    shaderList[0].SetDirectionalLight(&mainLight);
    shaderList[0].SetPointLights(pointLights, pointLightCount,3,0);
    shaderList[0].SetSpotLights(spotLights, spotLightCount,3+pointLightCount,pointLightCount);

    glm::mat4 mainLightTransform = mainLight.CalculateLightTransform();
    shaderList[0].SetDirectionalLightTransform(&mainLightTransform);
    //mainLight.UseLIght(uniformAmbientIntensity, uniformAmbientColor,uniformDiffuseIntensity,uniformDirection);

    mainLight.GetShadowMap()->Read(GL_TEXTURE2);
    shaderList[0].SetTexture(1);
    shaderList[0].SetDirectionalShadowMap(2);

    
    //GLfloat now2 = glfwGetTime();//SDL_GetPerformaceCounter();
    //deltaTime2 = now2 - lastTime2;//(now-lastTime)*1000/SDL_GetPerformaceFrequency()
    //lastTime2 = now2;

    SpotAngle += 48.5*deltaTime;
    if (SpotAngle > 360.0f) {
        SpotAngle = 48.5*deltaTime;
    }
    glm::mat4 Spotmodel(1.0f);
    Spotmodel = glm::mat4(1.0f);
    Spotmodel = glm::rotate(Spotmodel, -SpotAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    Spotmodel = glm::translate(Spotmodel, glm::vec3(10.0f, 0.0f, 0.0f));
    //Spotmodel = glm::rotate(Spotmodel, 20.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    
    glm::vec3 spotPos(0.0f, 0.0f, 0.0f);
    glm::vec3 spotDir(-1.0f, -1.0f, 0.0f);

    glm::vec4 vPos= (Spotmodel * glm::vec4(spotPos, 1.0f));
    GLfloat vfloat = vPos[3];
    vPos = vPos / vfloat;
    glm::vec3 vPos3=vPos;

    glm::vec4 vDir = (Spotmodel * glm::vec4(spotDir, 1.0f));
    GLfloat vfloatDir = vDir[3];
    vDir = vDir / vfloatDir;
    glm::vec3 vDir3 = vDir;
    

    spotLights[0].SetFlash(vPos3, -vDir3);

    glm::vec3 lowerLight = camera.getCameraPosition();

    lowerLight.y -= 0.3f;
    spotLights[1].SetFlash(lowerLight, camera.getCameraDirection());

    shaderList[0].Validate();
    RenderScene();
}

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateTriangle();
    CreateCube();
    CreateFloor();
    CreateShaders();
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);

    brickTexture = Texture("Textures/brick.jpg");
    brickTexture.LoadTexture();
    dirtTexture = Texture("Textures/dirt.jpg");
    dirtTexture.LoadTexture();
    PlainTexture = Texture("Textures/plain.png");
    PlainTexture.LoadTexture();

    shinyMaterial = Material(1.0f, 32);
    dullMaterial = Material(0.3f, 4);

    oldHouse = Model();
    oldHouse.LoadModel("Models/hatka_local_.obj");
    seaHawk = Model();
    seaHawk.LoadModel("Models/seaHawk.obj");

    mainLight = DirectionalLight(2048,2048,
                                1.0f,0.5f,0.3f,
                                0.1f,1.0f,
                                -10.0f,-12.0f,18.5f);


    pointLights[0] = PointLight(1024,1024,
                                0.01f,100.0f,
                                0.0f, 0.0f, 1.0f,
                                0.0f, 1.0f,
                                1.0f, 2.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);

    pointLightCount++;

    pointLights[1] = PointLight(1024, 1024, 
                                0.01f, 100.0f, 
                                0.0f, 1.0f, 0.0f,
                                 0.0f, 1.0f,
                                -4.0f, 3.0f, 0.0f,
                                0.3f, 0.1f, 0.1f);

    pointLightCount++;

    
    spotLights[0] = SpotLight(1024, 1024, 0.01f, 100.0f, 1.0f, 1.0f, 1.0f,
                                 0.0f, 0.1f,
                                4.5f, 2.0f, 3.8f,
                                -1.0f, -0.6f, -0.3f,
                                0.3f,0.2f,0.1f,
                                40.0f);
    spotLightCount++;

    spotLights[1] = SpotLight(1024, 1024,
        0.1f, 100.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        20.0f);
    spotLightCount++;

    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/right.tga");
    skyboxFaces.push_back("Textures/Skybox/left.tga");
    skyboxFaces.push_back("Textures/Skybox/up.tga");
    skyboxFaces.push_back("Textures/Skybox/bt.tga");
    skyboxFaces.push_back("Textures/Skybox/back.tga");
    skyboxFaces.push_back("Textures/Skybox/front.tga");

    skybox = Skybox(skyboxFaces);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);
    

    //loop until window closed
    while (!mainWindow.getShouldClose()) {

        GLfloat now = glfwGetTime();//SDL_GetPerformaceCounter();
        deltaTime = now - lastTime;//(now-lastTime)*1000/SDL_GetPerformaceFrequency()
        lastTime = now;

        //get + handle user input events
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(),deltaTime);
        camera.mouseControl(mainWindow.getXChange(),mainWindow.getYChange());
        if (mainWindow.getsKeys()[GLFW_KEY_L]) {
            spotLights[1].Toggle();
            mainWindow.getsKeys()[GLFW_KEY_L] = false;
        }

        DirectionalShadowMapPass(&mainLight);
        for (size_t i = 0; i < pointLightCount; i++) {
            OmniShadowMapPass(&pointLights[i]);
        }
        for (size_t i = 0; i < spotLightCount; i++) {
            OmniShadowMapPass(&spotLights[i]);
        }
        RenderPass(projection,camera.calculateViewMatrix());
  
        


        glUseProgram(0);

        mainWindow.swapBuffers();
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
