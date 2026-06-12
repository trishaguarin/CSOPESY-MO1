#include "GUIApplication.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdio>

bool GUIApplication::initialize() {

    // Phase 1: Bootstrap 
    if (!glfwInit()) {
        fprintf(stderr, "GLFW init failed.\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Window creation failed.\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowCloseCallback(window, [](GLFWwindow* window){
        GUIApplication* app = static_cast<GUIApplication*>(glfwGetWindowUserPointer(window));
        if (app && app->appRunning) {
            glfwSetWindowShouldClose(window, GLFW_FALSE);
        }
    });

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "GLAD init failed.\n");
        return false;
    }

    // Phase 2: Kernel Init (ImGui setup)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // no imgui.ini writes

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding  = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding  = 4.0f;
    style.Colors[ImGuiCol_WindowBg]     = ImVec4(0.10f, 0.10f, 0.13f, 0.96f);
    style.Colors[ImGuiCol_TitleBg]      = ImVec4(0.08f, 0.08f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.65f, 1.0f);
    style.Colors[ImGuiCol_Button]       = ImVec4(0.20f, 0.35f, 0.65f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered]= ImVec4(0.30f, 0.50f, 0.85f, 1.0f);
    style.Colors[ImGuiCol_Header]       = ImVec4(0.20f, 0.35f, 0.65f, 0.6f);
    style.Colors[ImGuiCol_HeaderHovered]= ImVec4(0.30f, 0.50f, 0.85f, 0.8f);
    style.Colors[ImGuiCol_Tab]          = ImVec4(0.15f, 0.25f, 0.50f, 1.0f);
    style.Colors[ImGuiCol_TabActive]    = ImVec4(0.20f, 0.40f, 0.75f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Phase 3: Start System Services 
    desktop.setAppRunning(&appRunning);
    taskbar.setWindows(&taskManager, &app1, &app2);

    // load wallpaper (place your image at assets/wallpapers/default.jpg)
    desktop.loadWallpaper("assets/wallpapers/default.jpg");

    return true;
}

//  Phase 4: Main Loop 
void GUIApplication::run() {
    while (!glfwWindowShouldClose(window) && appRunning) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render all components every frame
        desktop.draw();
        taskbar.draw();
        if (taskManager.isShown()) taskManager.draw();
        if (app1.isShown())        app1.draw();
        if (app2.isShown())        app2.draw();
        // uiManager can also render all registered windows if needed

        ImGui::Render();

        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

// Phase 5: Shutdown
void GUIApplication::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
