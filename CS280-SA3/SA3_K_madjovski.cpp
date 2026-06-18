#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>

int type1count = 0, type2count = 0, type3count = 0, wordCount = 0, type1Unique = 0, type2Unique = 0, type3Unique = 0;
std::vector<std::string> type1_words, type2_words, type3_words;

bool isUnique(const std::string& word, std::vector<std::string>& uniqueList) {
	for (int i = 0; i < uniqueList.size(); i++) {
		if (uniqueList.at(i) == word)
			return true;
	}
	return false;
}

int whichSpecial(const std::string& word) {
	char first = word[0];
	if (first == '_')
		return 1;
	if (first == '@')
		return 2;
	if (first == '#')
		return 3;
	else
		return 0;
}

bool isSpecialWord(const std::string& word) {
	if (word.size() < 2) {
		return false;
	}

	char first = word[0];

	if (first != '_' && first != '@' && first != '#') {
		return false;
	}

	for (size_t i = 1; i < word.size(); i++) {
		char b = word[i];

		if (!(std::isalnum(b) || b == '_')) {
			return false;
		}
	}

	return true;

}


int main(int argc, char* argv[]) {
	std::ifstream inputFile;
	std::string fileName;


	if (argc < 2) {
		std::cerr << "NO SPECIFIED INPUT FILE NAME." << '\n';
		exit(1);
	}

	fileName = std::string(argv[1]);

	std::ifstream infile(fileName);

	if (!infile.is_open()) {
		std::cerr << "CANNOT OPEN THE FILE " << fileName << '\n';
		exit(1);
	}

	if (infile.peek() == EOF) {
		std::cerr << "The file is empty." << '\n';
		exit(1);
	}

	std::string line;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);
		std::string word;

		while (iss >> word) {
			wordCount++;
			if (isSpecialWord(word)) {
				if (whichSpecial(word) == 1) {
					type1count++;
					if (!isUnique(word, type1_words)) {
						type1_words.push_back(word);
						type1Unique++;
					}
				}
				else if (whichSpecial(word) == 2) {
					type2count++;
					if (!isUnique(word, type2_words)) {
						type2_words.push_back(word);
						type2Unique++;
					}
				}
				else if (whichSpecial(word) == 3) {
					type3count++;
					if (!isUnique(word, type3_words)) {
						type3_words.push_back(word);
						type3Unique++;
					}
				}
				else
					continue;
			}

		}

	}


	bool has_t1 = false, has_t2 = false, has_t3 = false, has_all = false;
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-t1") has_t1 = true;
		else if (arg == "-t2") has_t2 = true;
		else if (arg == "-t3") has_t3 = true;
		else if (arg == "-all") has_all = true;

	}

	if (!(!has_all && (has_t1 || has_t2 || has_t3)))
		std::cout << "Total number of words: " << wordCount << '\n';
	if (has_all) {
		std::cout << "Occurrences of Type1 Names (Starting with '_' character): " << type1count << '\n';
		std::cout << "Occurrences of Type2 Names (Starting with '@' character): " << type2count << '\n';
		std::cout << "Occurrences of Type3 Names (Starting with '#' character): " << type3count << '\n';
	}
	if (has_t1) {
		std::cout << "Count of Type1 Unique Names: " << type1Unique << '\n';
	}
	if (has_t2) {
		std::cout << "Count of Type2 Unique Names: " << type2Unique << '\n';
	}
	if (has_t3) {
		std::cout << "Count of Type3 Unique Names: " << type3Unique << '\n';
	}

	return 0;
}