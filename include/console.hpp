#include <iostream>
#include <string>

struct Console {
	static void log(std::string s){
		std::cout << s << std::endl;
	}
	static void warn(std::string s){
		log(s);
	}
};

extern Console console;