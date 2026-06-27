this 

g++ -std=c++17 -o test_sleep.exe test_sleep_phenomenon.cpp `
  src/Process.cpp src/Scheduler.cpp src/ConfigParser.cpp `
  src/Console.cpp src/ScreenManager.cpp src/ReportGenerator.cpp `
  src/PrintCommand.cpp src/DeclareCommand.cpp src/AddCommand.cpp `
  src/SubtractCommand.cpp src/SleepCommand.cpp src/ForCommand.cpp `
  -lpthread


then this
.\test_sleep.exe