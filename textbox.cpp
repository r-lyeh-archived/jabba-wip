#include "textbox.hpp"
#include <iostream>

/*
class grid {
	unsigned w, h;
	std::map< unsigned, std::string > map;

	std::string &
}; */


int main() {
	window w = { "sample", true, { 10, 10, "hello world\nlorem ipsum, lorem ipsum, lorem ipsum\nbye bye world\n" } };

	std::cout << render(w) << std::endl;

	w.tx.w *= 2;
	std::cout << render(w) << std::endl;

	std::cout << box("hello world") << std::endl;
}
