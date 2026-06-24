# CSOPESY Major Output 1 — Process Scheduler & CLI Emulator

**Course:** CSOPESY — Operating Systems  
**Type:** Group Machine Output  
**Branch:** `MO1-Process-Multiplexer`

---

## Overview

A command-line process multiplexer and CPU scheduler emulator written in C++17.  
The system emulates a simplified OS shell with:

- A **CLI console** recognizing OS-level commands
- A **screen multiplexer** (inspired by Linux `screen`)
- A **CPU scheduler** supporting FCFS and Round-Robin
- **Process instructions** (PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR)
- A **config.txt**-driven setup (no recompilation needed to change parameters)

---

## Project Structure

```
CSOPESY-MO1/
├── README.md               # You are here
├── config.txt              # Runtime configuration (read by `initialize`)
├── MO1SPECS.pdf            # Official specifications
│
├── src/
│   ├── main.cpp            # Entry point — boots the CLI
│   ├── Console.h/.cpp      # Main menu console, command dispatcher
│   ├── ConfigParser.h/.cpp # Parses config.txt into runtime settings
│   ├── Process.h/.cpp      # Process representation + instruction execution
│   ├── Instruction.h       # Instruction types enum and struct
│   ├── Scheduler.h/.cpp    # FCFS & RR scheduling with CPU tick model
│   ├── ScreenManager.h/.cpp# Screen multiplexer (screen -s, -r, -ls)
│   └── ReportGenerator.h/.cpp # report-util + csopesy-log.txt
│
└── docs/
    └── architecture.md     # Architectural notes (optional)
```

---

## Build & Run

### Prerequisites
- C++17 compatible compiler (g++ / MSVC / clang++)
- POSIX threads (`-pthread` on Linux/MinGW)

### Compile (g++)
```bash
g++ -std=c++17 src/*.cpp -o csopesy.exe -pthread
```

### Run
```bash
./csopesy.exe
```

### First Command
```
> initialize
```
This reads `config.txt` and boots the scheduler. No other commands work until this is run.

---

## config.txt Format

Space-separated key-value pairs. Example:

```
num-cpu 4
scheduler "fcfs"
quantum-cycles 5
batch-process-freq 1
min-ins 1000
max-ins 2000
delays-per-exec 0
```

| Parameter            | Description                                  | Range       |
|----------------------|----------------------------------------------|-------------|
| `num-cpu`            | Number of CPU cores                          | [1, 128]    |
| `scheduler`          | Scheduling algorithm: `"fcfs"` or `"rr"`     | —           |
| `quantum-cycles`     | Time slice for Round-Robin (ignored by FCFS) | [1, 2³²]   |
| `batch-process-freq` | New process every X CPU cycles               | [1, 2³²]   |
| `min-ins`            | Minimum instructions per process             | [1, 2³²]   |
| `max-ins`            | Maximum instructions per process             | [1, 2³²]   |
| `delays-per-exec`    | Busy-wait delay ticks between instructions   | [0, 2³²]   |

---

## Commands

| Command                     | Context    | Description                                  |
|-----------------------------|-----------|----------------------------------------------|
| `initialize`                | Main menu | Load config.txt, boot scheduler              |
| `screen -s <name>`          | Main menu | Create a new process and enter its screen     |
| `screen -r <name>`          | Main menu | Reattach to an existing process screen        |
| `screen -ls`                | Main menu | List all processes + CPU utilization          |
| `scheduler-start`           | Main menu | Begin batch process generation                |
| `scheduler-stop`            | Main menu | Stop batch process generation                 |
| `report-util`               | Main menu | Generate CPU utilization report → csopesy-log.txt |
| `clear`                     | Main menu | Clear the console                             |
| `exit`                      | Both      | Exit screen → main menu, or quit application  |
| `process-smi`               | Screen    | Show process info and instruction logs        |

---

## Group Members

<!-- TODO: Add your group members here -->
- Member 1
- Member 2
- Member 3
- Member 4

---

## Assessment

Assessed via **black-box quiz** — only `config.txt` is modified between test cases.  
No recompilation allowed during the quiz.