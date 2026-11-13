#include <iostream>
#include <windows.h>

int main() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	
	for (;;) {
		if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
			int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			std::cout<< width << " X " << height << std::endl;
		}
	}
	return 0;
}
