#ifndef DISPLAYCONTROLLER_H_
#define DISPLAYCONTROLLER_H_

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "PPU.h"
#include "Windows.h"

class DisplayController {
public:
	DisplayController(sf::RenderWindow *window, PPU *ppu);
	~DisplayController();
	void update();
private:
	sf::RenderWindow *window;
	PPU *ppu;
};


#endif
