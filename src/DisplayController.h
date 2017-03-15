/*
 * DisplayController.h
 *
 *  Created on: 15 Mar 2017
 *      Author: Orcworm
 */

#ifndef DISPLAYCONTROLLER_H_
#define DISPLAYCONTROLLER_H_

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "PPU.h"

class DisplayController {
public:
	DisplayController();
	~DisplayController();
private:
	sf::RenderWindow *window;
	PPU *ppu;
};


#endif /* DISPLAYCONTROLLER_H_ */
