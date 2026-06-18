#include <iostream>
#include <fstream>
#include <string>

int totalLineCount = 0;
int commentedLines = 0;
int validCmdLines = 0;
int invalidCmdLines = 0; 
int dirLines = 0;
int cdLines = 0;
int copyLines = 0;
int delLines = 0;
int main() {
	std::ifstream inputFile;
	std::string fileName;
	

	std::cout << "Enter the name of a file to read from: " << '\n';
	std::cin >> fileName;

	inputFile.open(fileName.c_str());

	 if (!inputFile) {
        std::cout << '\n';
		std::cerr << "File cannot be opened: " << fileName  << '\n';
        return 0; }
     

	if (inputFile.peek() == EOF) {
		std::cerr << "File is empty." << '\n';
        return 0;
    }
    std::cout << '\n';
	std::string line;
	while (std::getline(inputFile, line)) {
		totalLineCount++;
		line.erase(0, line.find_first_not_of(" \t\r")); // Whitespace trim
        line.substr(0, line.find_first_of(" "));
		if (line.empty())
			continue;
        
		if (line.size() >= 2 && line.substr(0, 2) == "cd") {
			validCmdLines++;
			cdLines++;
			continue;
		}
        	if (line.size() >= 6 && line.substr(0, 6) == "delete") {
           invalidCmdLines++;
            std::string falseCommand = line.substr(0, line.find_first_of(" \t\n") );
            std::cout << "Error: Unrecognizable command in line " << totalLineCount << ": " << falseCommand << '\n' << '\n';
            continue;
        }
		if (line.size() >= 3 && line.substr(0, 3) == "del") {
			validCmdLines++;
			delLines++;
			continue;
		}
		if (line.size() >= 3 && (line.substr(0, 3) == "dir" || line.substr(0, 3) == "Dir")) {
			validCmdLines++;
			dirLines++;
			continue;
		}
		if (line.size() >= 4 && line.substr(0, 4) == "copy") {
			validCmdLines++;
			copyLines++;
			continue;
		}
		if ((line.size() >= 3 && (line.substr(0, 3) == "REM" || line.substr(0, 3) == "rem" || line.substr(0, 3) == "Rem")) || 
        (line.size() >= 2 && line.substr(0, 2) == "::")) {
			commentedLines++;
			continue;

		}
        else {
           invalidCmdLines++;
            std::string falseCommand = line.substr(0, line.find_first_of(" \t\n") );
            std::cout << "Error: Unrecognizable command in line " << totalLineCount << ": " << falseCommand << '\n' << '\n';
        }

	}
	std::cout << "Total lines: " << totalLineCount << '\n' << "Commented lines: " << commentedLines << '\n' << "Valid Command lines: " << validCmdLines << '\n' << "Invalid Command lines: " << invalidCmdLines << '\n' << "DIR commands: " << dirLines << '\n' << "CD commands: " << cdLines << '\n' << "COPY commands: " << copyLines << '\n' << "DEL commands: " << delLines << '\n'; 
 return 0;
}