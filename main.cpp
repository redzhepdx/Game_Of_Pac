#include <GameWindow.h>
#include <config.h>
#include <memory>

#include "callstack/Callstack.h"
#include "segvcatch.h"

void handle_segv()
{
    throw std::runtime_error("SEGV");
}

void handle_fpe()
{
    throw std::runtime_error("FPE");
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

int main(int argc, const char * argv[])
{
    segvcatch::init_segv(&handle_segv);
    segvcatch::init_fpe(&handle_fpe);

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
    
    try{
        gameWindow->play(window);
    }
    catch(std::exception& e){
        spdlog::error("Exception Catched : {}", e.what());

        NL_RETURN(-1);
    }

    glfwDestroyWindow(window);
    
    NL_RETURN(0);
}
