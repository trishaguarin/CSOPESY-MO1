#include <iostream>
#include <string>

using namespace std;

void openingHeader() {
	cout << "CSOPESY ASCII HERE" << endl;

	cout << "Hello, welcome to CSOPESY command line!" << endl;
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
			cout << command << " command is recognized. Doing something." << endl;
		}

		else if (command == "screen") {
			cout << command << " command is recognized. Doing something." << endl;
		}

		else if (command == "scheduler-start") {
			cout << command << " command is recognized. Doing something." << endl;
		}

		else if (command == "scheduler-stop") {
			cout << command << " command is recognized. Doing something." << endl;
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