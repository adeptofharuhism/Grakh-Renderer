#include <iostream>

using namespace std;

void Output(char** argv) {
	int index = 0;
	char* output = argv[0];

	while (output[index] != '\0')
	{
		cout << output[index++];
	}

	cout.flush();
}

void Output(const char message[]) {
	cout << message;
	cout.flush();
}