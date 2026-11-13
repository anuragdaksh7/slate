#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>

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

std::string createOrGetFile(const std::string& filename) {
	std::fstream file(filename, std::ios::in | std::ios::out | std::ios::app);
	
	if (!file.is_open()) {
		throw FileOpeningException();
	}

	std::string line;
	while (std::getline(file, line)) {
		std::cout<< line << '\n';
	}

	file.close();

	return line;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		throw InvalidParameterException();
	}
	if (argv[1] == "") {
		throw InvalidFileNameException();
	}

	validateFile(argv[1]);

	std::string fileContents = createOrGetFile(argv[1]);

	std::cout<<fileContents;	

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	
	/*
	for (;;) {
		system("cls");
		if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
			int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			std::cout<< width << " X " << height << std::endl;
		}
	}
	*/
	return 0;
}
