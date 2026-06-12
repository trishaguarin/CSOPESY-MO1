#include "Desktop.h"
#include "imgui.h"
#include <chrono>
#include <ctime>
#include <cstdio>
#if defined(__has_include)
#  if __has_include("stb_image.h")
#    define STB_IMAGE_IMPLEMENTATION
#    include "stb_image.h"
#    define HAVE_STB_IMAGE 1
#  else
#    define HAVE_STB_IMAGE 0
#  endif
#else
#  define HAVE_STB_IMAGE 0
#endif
#include <iostream>
#include <cstdio>

Desktop::Desktop()
    : AppWindow("Desktop")
{
    show(); // desktop is always visible
}

void Desktop::setAppRunning(bool* running)
{
    appRunning = running;
}

void Desktop::draw()
{
    if (!isShown())
        return;

    renderWallpaper();
    drawClock();
    powerButton();
}

// self-explanatory
void Desktop::renderWallpaper()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();

    draw->AddRectFilledMultiColor(
        ImVec2(0, 0),
        ImVec2(screenSize.x, screenSize.y),
        IM_COL32(10, 15, 40, 255),
        IM_COL32(10, 15, 40, 255),
        IM_COL32(10, 50, 70, 255),
        IM_COL32(10, 50, 70, 255)
    );

    ImU32 gridColor = IM_COL32(255, 255, 255, 12);
    for (float x = 0; x < screenSize.x; x += 60)
        draw->AddLine(ImVec2(x, 0), ImVec2(x, screenSize.y), gridColor);
    for (float y = 0; y < screenSize.y; y += 60)
        draw->AddLine(ImVec2(0, y), ImVec2(screenSize.x, y), gridColor);

    // draw wallpaper if loaded
    if (wallpaperTex) {
        // ImTextureID is GLuint cast to void*
        draw->AddImage((ImTextureID)(intptr_t)wallpaperTex, ImVec2(0, 0), ImVec2(screenSize.x, screenSize.y));
        return; // wallpaper drawn, skip procedural background
    }
}
void Desktop::loadWallpaper(const char* path)
{
    if (!path) return;

#if HAVE_STB_IMAGE
    int w, h, comp;
    unsigned char* data = stbi_load(path, &w, &h, &comp, 4);
    std::string usedPath;
    if (!data) {
        std::string fallback = std::string("../") + path;
        data = stbi_load(fallback.c_str(), &w, &h, &comp, 4);
        if (data) {
            std::cerr << "Loaded wallpaper from fallback: " << fallback << std::endl;
            usedPath = fallback;
        } else {
            std::cerr << "Failed to load wallpaper: " << path << std::endl;
            return;
        }
    } else {
        usedPath = path;
    }

    if (wallpaperTex) {
        glDeleteTextures(1, &wallpaperTex);
        wallpaperTex = 0;
    }

    glGenTextures(1, &wallpaperTex);
    glBindTexture(GL_TEXTURE_2D, wallpaperTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    wallpaperPath = usedPath;
#else
    std::cerr << "stb_image.h not available; wallpaper not loaded: " << path << std::endl;
#endif
}

// self-explanatory
void Desktop::drawClock()
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);

    char timeBuf[16];
    char dateBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", localTime);
    std::strftime(dateBuf, sizeof(dateBuf), "%A, %B %d %Y", localTime);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    float padding = 16.0f;
    draw->AddText(ImGui::GetFont(), 26.0f,
        ImVec2(screenSize.x - 160, padding),
        IM_COL32(255, 255, 255, 220), timeBuf);
    draw->AddText(ImGui::GetFont(), 14.0f,
        ImVec2(screenSize.x - 210, padding + 28),
        IM_COL32(255, 255, 255, 120), dateBuf);
}

// self-explanatory
void Desktop::powerButton()
{
    if (!appRunning)
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    float btnW = 50.0f;
    float btnH = 50.0f;
    float padding = 16.0f;
    float taskbarHeight = 48.0f;

    ImGui::SetNextWindowPos(
        ImVec2(screenSize.x - btnW - padding,
               screenSize.y - btnH - padding - taskbarHeight));

    ImGui::SetNextWindowSize(
        ImVec2(btnW + padding, btnH + padding));

    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("##pwr", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    ImGui::PushStyleColor(ImGuiCol_Button,
        IM_COL32(180, 30, 30, 200));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        IM_COL32(220, 50, 50, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        IM_COL32(255, 80, 80, 255));

    if (ImGui::Button("PWR", ImVec2(btnW, btnH)))
        *appRunning = false;

    ImGui::PopStyleColor(3);
    ImGui::End();
}