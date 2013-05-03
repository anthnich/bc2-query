#include "query.hpp"

int main() {

	Main_Window *mainwin = new Main_Window(800, 640, "BF:BC2 Server Query");

	while (Fl::wait() != 0) {

		mainwin->Think();

		sf::sleep(sf::seconds(1/30.f));

	}

}