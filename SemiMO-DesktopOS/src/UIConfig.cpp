#include <GLFW/glfw3.h>

class UIConfig {
public:
    static void initialize() {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        scaleFactor = static_cast<float>(mode->width) / 1920.0f;
        if (scaleFactor < 1.0f) scaleFactor = 1.0f;
        if (scaleFactor > 2.0f) scaleFactor = 2.0f;
    }
    static float getScaleFactor() { return scaleFactor; }
private:
    static float scaleFactor;
};