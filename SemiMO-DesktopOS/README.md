# Semi-MO: Desktop-Style OS Mockup

CSOPESY | Semi-Machine Output  
Tech Stack: **C++ · Dear ImGui · GLFW · OpenGL**

---

## Project Structure

```
SemiMO-DesktopOS/
├── CMakeLists.txt              # Build configuration
├── README.md
│
├── src/                        # Translation units (.cpp)
│   ├── main.cpp                # Entry point — GLFW window init, ImGui context, render loop
│   ├── Application.cpp         # Top-level compositor: orchestrates all components each frame
│   ├── Desktop.cpp             # Component 1 — wallpaper, clock overlay, PWR button
│   ├── Taskbar.cpp             # Component 2 — fixed panel with icon buttons
│   ├── TaskManager.cpp         # Component 3 — task manager window (process table)
│   ├── AppWindow.cpp           # Generic UI screen opened by taskbar icon buttons
│   └── Clock.cpp               # Real-time clock helper (formats system time each frame)
│
├── include/                    # Header files
│   ├── Application.h
│   ├── Desktop.h
│   ├── Taskbar.h
│   ├── TaskManager.h
│   ├── AppWindow.h
│   ├── Clock.h
│   └── Config.h                # Global constants (window size, app name, color palette, etc.)
│
├── assets/
│   └── wallpapers/             # Wallpaper image files (e.g., .png / .jpg)
│
├── third_party/
│   ├── imgui/                  # Dear ImGui source (add as submodule or copy)
│   └── glfw/                   # GLFW source (add as submodule or copy)
│
└── docs/
    └── architecture.md         # Architectural diagram + component walkthrough
```

---

## Required Components

| Component | Description |
|-----------|-------------|
| **Desktop** | Full-screen background layer. Renders wallpaper, real-time clock, and PWR (shutdown) button. |
| **Taskbar** | Fixed panel (top or bottom). ≥3 clickable icon buttons — at least two open unique UI screens, one opens the Task Manager. |
| **Task Manager** | Windows-style window with a placeholder process table showing CPU & memory usage. |

---

## Build Instructions

> *(To be filled in once third-party libs are added)*

```bash
mkdir build && cd build
cmake ..
cmake --build .
```
