#include <GameWindow.h>
#include <config.h>
#include <memory>

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
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

    gameWindow->play(window);

    exit(EXIT_SUCCESS);
}
