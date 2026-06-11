cmake_minimum_required(VERSION 3.16)
project(SemiMO-DesktopOS)

set(CMAKE_CXX_STANDARD 17)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)

set(SOURCES
    src/main.cpp
    src/GUIApplication.cpp
    src/Desktop.cpp
    src/Taskbar.cpp
    src/TaskManager.cpp
    src/UniqueApp1.cpp
    src/UniqueApp2.cpp
    src/UIManager.cpp
    src/UIConfig.cpp
    src/AppWindow.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(SemiMO ${SOURCES})

target_include_directories(SemiMO PRIVATE
    include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/third_party/glfw/include
)

target_link_directories(SemiMO PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/glfw/lib-mingw-w64
)

target_link_libraries(SemiMO PRIVATE
    glfw3 opengl32 gdi32 user32 kernel32
)