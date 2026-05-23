#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

void openingHeader() {
	cout << R"(
 
	 _______  _______  _______  _______  _______  _______  __   __ 
	|       ||       ||       ||       ||       ||       ||  | |  |
	|       ||  _____||   _   ||    _  ||    ___||  _____||  |_|  |
	|       || |_____ |  | |  ||   |_| ||   |___ | |_____ |       |
	|      _||_____  ||  |_|  ||    ___||    ___||_____  ||_     _|
	|     |_  _____| ||       ||   |    |   |___  _____| |  |   |  
	|_______||_______||_______||___|    |_______||_______|  |___|  

	)" << endl;

	cout << "Hello, welcome to the CSOPESY command line!" << endl;
	cout << "Type 'exit' to quit, 'clear' to clear the screen" << endl;

	cout << "\n** IMPORTANT: Type 'initialize' to load config and start system ** " << endl;
}

int main() {
	using namespace std;
	string command;

	openingHeader();

	while (true) {
		cout << "\nEnter a command: "; getline(cin, command);

		if (command == "initialize") {
			cout << command << " command recognized. Doing something." << endl;
		}

		else if (command == "screen") {
			cout << command << " command recognized. Doing something." << endl;
		}

		else if (command == "scheduler-start") {
			cout << command << " command recognized. Doing something." << endl;
		}

		else if (command == "scheduler-stop") {
			cout << command << " command recognized. Doing something." << endl;
		}

		else if (command == "report-util") {
            cout << command << " command recognized. Doing something." << endl;
        }

		else if (command == "clear") {
			#ifdef _WIN32 //kung windows
				system("cls");
			#else
				system("clear");
			#endif

			openingHeader();

		}

		else if (command == "exit") {
			return 0;
		}

		else {
			cout << "Unknown command: " << command << endl;
		}

	}
	return 0;
}