#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct GameObject final
{
    GLFWwindow* window;
    float deltaTime;
    int frameCount;
    int frameRate;
    glm::vec3 mouseCoOrds;
};

struct RenderObject final
{
    glm::mat4 view;
    glm::mat4 proj;
    RenderObject(const glm::mat4 view, const glm::mat4 proj) : view(view), proj(proj){}
};

class Renderable
{
public:
    virtual ~Renderable() = default;
    virtual void draw(const GameObject& game, const RenderObject& render) {};
};

class Preupdate
{
public:
    virtual ~Preupdate() = default;
    virtual void preupdate(GameObject game);
};

class Update
{
public:
    virtual ~Update() = default;
    virtual void update(GameObject game);
};