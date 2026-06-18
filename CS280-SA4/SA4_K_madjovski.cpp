#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <map>
#include <regex>
#include <sstream>
#include <set>



int main(int argc, char* argv[]) {
	int totalLineCount = 0;
	std::string fileName;

	if (argc < 2) {
		std::cerr << "NO SPECIFIED INPUT FILE NAME." << '\n';
		exit(1);
	}
	bool hasAll = false;
	bool hasInt = false;
	bool hasReal = false;

	for (size_t i = 2; i < argc; i++) {
		if (std::string(argv[i]) == "-all")
			hasAll = true;
		else if (std::string(argv[i]) == "-int")
			hasInt = true;
		else if (std::string(argv[i]) == "-real")
			hasReal = true;
	}
	
	fileName = std::string(argv[1]);

	std::ifstream inputFile(fileName);

	if (!inputFile.is_open()) {
		std::cerr << "CANNOT OPEN THE FILE " << fileName << '\n';
		exit(1);
	}
	if (inputFile.peek() == EOF) {
		std::cout << "File is Empty." << '\n';
		exit(1);
	}
	std::map<std::string, int>literalCount;
	std::set<std::string> uniqueInts; 
	std::set<std::string> uniqueFloats;

	std::regex intPattern(R"([0-9]+)");
	std::regex floatPattern(R"([0-9]+\.[0-9]+)");
	std::regex expPattern(R"([0-9]+(\.[0-9]+)?[Ee][+-]?[0-9]+)");
	std::regex badFloatPattern(R"([0-9]+\.[0-9]+\.)");

	std::string line;

	while (std::getline(inputFile, line)) {
		totalLineCount++;
		std::string word;
		std::smatch match;
		std::string bad_token;
		if (std::regex_search(line, match, badFloatPattern) ) {
			bad_token = match.str();
			std::cout << "Line " << totalLineCount << ": Invalid floating-point literal " << '\"' << bad_token << '\"' << '\n';
			line = std::regex_replace(line, badFloatPattern, " ");
		}
		std::sregex_iterator end;
		// ---------- STEP 1: FIND EXPONENTIALS ----------
		 std::sregex_iterator expIt(line.begin(), line.end(), expPattern);

		while (expIt != end) {

			std::string number = expIt->str();
			if (number.find('.') != std::string::npos)
				uniqueFloats.insert(number);
			else
				uniqueInts.insert(number);
			literalCount[number]++;

			++expIt;
		} 
		line = std::regex_replace(line, expPattern, " ");
		// ---------- STEP 2: FIND FLOATS ----------
		std::sregex_iterator floatIt(line.begin(), line.end(), floatPattern);


		while (floatIt != end) {
			
			std::string number = floatIt->str();

			literalCount[number]++;
			uniqueFloats.insert(number);

			++floatIt;
		}

		// remove floats from line so ints inside them aren't detected
		line = std::regex_replace(line, floatPattern, " ");

		// ---------- STEP 3: FIND INTEGERS ----------
		std::sregex_iterator intIt(line.begin(), line.end(), intPattern);

		while (intIt != end) {
			std::string number = intIt->str();

			literalCount[number]++;
			uniqueInts.insert(number);

			++intIt;
		}

	}

	std::cout << "Total Number of Lines: " << totalLineCount << '\n'
		<< "Number of Integer Literals: " << uniqueInts.size() << '\n'
		<< "Number of Floating-Point Literals: " << uniqueFloats.size() << '\n';

	if (hasAll) {
		std::cout << std::endl;
		std::cout << "List of All Numeric Literals and their Number of Occurrences: " << '\n';
		for (const auto& pair : literalCount) {
			std::cout << '\"' << pair.first << '\"' << ':' << ' ' << pair.second << '\n';
		}
	}
	if (hasInt) {
		std::cout << std::endl;
		std::cout << "List of Integer Literals and their Values:" << '\n';
		for (const auto& num : uniqueInts) {
			std::cout << '\"' << num << '\"' << ": " << std::stoi(num) << '\n';
		}
	}
	if (hasReal) {
		std::cout << std::endl;
		std::cout << "List of Floating-Point Literals and their Values:" << '\n';
		for (const auto& num : uniqueFloats) {
			std::cout << '\"' << num << '\"' << ": " << std::stod(num) << '\n';
		}
	}
}