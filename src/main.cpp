#include "GameWindow.h"
#include <memory>

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}


int main(int argc, const char * argv[])
{
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "C Pac Man", nullptr, nullptr);
    glfwSetKeyCallback(window, key_callback);
    //glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetWindowTitle(window, "C Pac Man");
    glfwSwapInterval(0);


    glfwMakeContextCurrent(window); //Important to make window writeable

    //Initialize this after window not before
    std::unique_ptr<GameWindow> gameWindow = std::make_unique<GameWindow>(true, (int)WIDTH, (int)HEIGHT);

    double lastTime    = glfwGetTime();
    double deltaTime   = 0.0f;
    double currentTime = 0.0f;

    while(gameWindow->getGameStatus()){

        gameWindow->render(window);

        deltaTime += (glfwGetTime() - lastTime) * Updates_Per_Second;
        lastTime = glfwGetTime();
        while (deltaTime >= 1.0f){
            gameWindow->update(window);
            --deltaTime;
        }
        /*
        currentTime += (glfwGetTime() - currentTime);

        if(currentTime >= 0.01f){
          currentTime = 0.0f;
          gameWindow->update(window);
        }
        */
        gameWindow->updateGameStatus(window);
    }

    gameWindow->close(window);
    exit(EXIT_SUCCESS);
}
