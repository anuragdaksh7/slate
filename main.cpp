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

int main(int argc, char* argv[]) {
	if (argc != 2) {
		throw InvalidParameterException();
	}
	if (argv[1] == "") {
		throw InvalidFileNameException();
	}

	validateFile(argv[1]);

	createOrGetFile(argv[1], fileContents);

	std::thread windowUpdateWatcher(screenDimUpdate);
	
	windowUpdateWatcher.join();

	return 0;
}

void render() {
	std::cout<<"rendering...";
	int n = fileContents.size();

	system("cls");
	for (int i = currentLine; i< height -  reservedLines; i++) {
		if (i >= n) {
			std::cout<< "~\n";
		} else {
			std::cout<< fileContents[i] << '\n';
		}
	}
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
