#include <iostream>
#include <string>

int main() {
	std::cout << "Welcome to CS 280 (Spring 2026)" << '\n' << "What are your first name, last name, and section number?" << '\n';
	std::string firstname;
	std::string lastname;
	std::string sec_number;

	std::cin >> firstname >> lastname >> sec_number;

	std::cout << "Hello " << firstname << ", Welcome to CS 280 Section " << sec_number << "." << '\n';

	return 0;
}
