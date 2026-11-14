#include <iostream>
#include <windows.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

int width = 0, height = 0;
int currentLine = 0;
int reservedLines = 1;
int currentCursorRow = 0;
int currentCursorCol = 0;
std::vector<std::string> fileContents;

class InvalidParameterException : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Invalid Parameters!";
		}
};

class InvalidFileNameException : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Invalid Filename!";
		}
};

class InaccessibleContent : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Inaccessible content!";
		}
};

class FileOpeningException : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Error opening the file!";
		}
};

bool isValidFilename(const std::string& filename);
void validateFile(const std::string& filename);
void createOrGetFile(const std::string& filename, std::vector<std::string>& fileContents);
void screenDimUpdate();
void render();
LRESULT CALLBACK KeyboardListner(int nCode, WPARAM wParam, LPARAM lParam);
void ioUpdate();
void renderCursorUpdate();
void writeAtCoords(int row, int col, const std::string& text);

int main(int argc, char* argv[]) {
	if (argc != 2) {
		throw InvalidParameterException();
	}
	if (argv[1] == "") {
		throw InvalidFileNameException();
	}

	// hide default cursor
	std::cout << "\033[?25l";

	validateFile(argv[1]);

	createOrGetFile(argv[1], fileContents);

	std::thread windowUpdateWatcher(screenDimUpdate);
	std::thread ioUpdateWatcher(ioUpdate);
	
	windowUpdateWatcher.join();
	ioUpdateWatcher.join();
	
	// restore default cursor
	std::cout << "\033[?25h";
	std::cout << "\033[2J\033[H";
	return 0;
}

void render() {
	// std::cout<<"rendering...";
	int n = fileContents.size();

	// system("cls");
	std::cout << "\033[2J";
	std::cout << "\033[H";
	for (int i = currentLine; i< height + currentLine -  reservedLines; i++) {
		if (i >= n) {
			std::cout<< "~\n";
		} else {
			std::cout<< fileContents[i] << '\n';
		}
	}
	
	// std::cout<<currentCursorRow + 1 << "," << currentCursorCol + 1;

	std::cout << "\033[" << currentCursorRow << ";"<< currentCursorCol << "H";

	char _cursor = ' ';

	if  ((currentLine + currentCursorRow) < fileContents.size() && currentCursorCol < fileContents[currentLine].size()) {
		_cursor = fileContents[currentLine + currentCursorRow][currentCursorCol];
	}
	if ((currentLine + currentCursorRow) >= fileContents.size())
		_cursor = '~';

	std::cout << "\033[7m" << _cursor << "\033[0m";

	std::cout.flush();
}

void flushCursor(int cursorRow, int cursorCol) {
	std::cout << "\033[" << cursorRow + 1 << ";" << cursorCol + 1 << "H";

	char _cursor = ' ';

	if ((currentLine + cursorRow) < fileContents.size() && cursorCol < fileContents[currentLine + cursorRow].size()) {
		_cursor = fileContents[currentLine + cursorRow][cursorCol];
	}
	if ((currentLine + cursorRow) >= fileContents.size())
		_cursor = '~';

	std::cout << _cursor;
}

void writeAtCoords(int row, int col, const std::string& text) {
	std::cout << "\033[" << row << ";" << col << "H";
	std::cout << text;
}

void renderCursorUpdate() {
	std::cout << "\033[" << currentCursorRow + 1 << ";1H";
	
	char _cursor = ' ';
	
	if ((currentLine + currentCursorRow) < fileContents.size() && currentCursorCol < fileContents[currentLine].size()) {
		_cursor = fileContents[currentLine + currentCursorRow][currentCursorCol];
	}
	if ((currentLine + currentCursorRow) >= fileContents.size())
		_cursor = '~';

	std::cout << "\033[7m" << _cursor << "\033[0m";

	std::cout.flush();
}

bool isValidFilename(const std::string& filename) {
	if (filename.empty() || filename == "." || filename == "..") {
		return false;
	}

	const std::string invalid = "<>:\"/\\|?*";

	if (filename.find_first_of(invalid) != std::string::npos) 
		return false;

	if (filename.back() == '.' || filename.back() == ' ') 
		return false;
	
	return true;
}

void validateFile(const std::string& filename) {
	if (!isValidFilename(filename)) {
		throw InvalidFileNameException();
	}

	if (std::filesystem::exists(filename) &&
			!std::filesystem::is_regular_file(filename)) {
		throw InaccessibleContent();
	}
}

void createOrGetFile(const std::string& filename, std::vector<std::string>& fileContents) {
	std::fstream file(filename, std::ios::in | std::ios::out | std::ios::app);
	
	if (!file.is_open()) {
		throw FileOpeningException();
	}

	std::string line;
	while (std::getline(file, line)) {
		// std::cout<< line << '\n';
		fileContents.push_back(line);
	}

	file.close();
}

void screenDimUpdate() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	render();
	bool flag = false;
	for (;;) {
		if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
			if (width != (csbi.srWindow.Right - csbi.srWindow.Left + 1) || height != (csbi.srWindow.Bottom - csbi.srWindow.Top + 1)) {
				flag = true;
			}

			width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			
			if (flag)
				render();
			flag = false;
		}
	}
}

void ioUpdate() {
	HHOOK hook = SetWindowsHookEx(
			WH_KEYBOARD_LL,
			KeyboardListner,
			GetModuleHandle(nullptr),
			0
		);

	if (!hook) {
		std::cerr << "Hook installer failed\n";
		return;
	}

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {}

	UnhookWindowsHookEx(hook);
}

LRESULT CALLBACK KeyboardListner(int nCode, WPARAM wParam, LPARAM lParam) {
	int tmpCurr = currentLine;
	int tmpCursorRow = currentCursorRow;
	if (nCode == HC_ACTION) {
		auto* key = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			if (key->vkCode == VK_UP) {
				if (currentCursorRow > 0) {
					currentCursorRow -= 1;
				} else {
					currentLine = currentLine > 0 ? currentLine - 1 : currentLine;
				}
			}
			if (key->vkCode == VK_DOWN) {
				if (currentCursorRow < height - 2) {
					currentCursorRow += 1;
				} else {
					currentLine = currentLine > (fileContents.size() - height/2) ? currentLine : currentLine + 1;
				}
			}
		}
	}

	if (tmpCurr == currentLine || tmpCursorRow != currentCursorRow) {
		flushCursor(tmpCursorRow, 0);
		writeAtCoords(height, 0, std::to_string(currentCursorRow) + "," + std::to_string(currentCursorCol));
		renderCursorUpdate();
	} else if (tmpCurr != currentLine) {
		render();
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

