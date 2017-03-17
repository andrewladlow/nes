/*
 * DisplayController.cpp
 *
 *  Created on: 15 Mar 2017
 *      Author: Orcworm
 */

#include "DisplayController.h"

DisplayController::DisplayController(sf::RenderWindow *window, PPU *ppu) {
	this->window = window;
	this->ppu = ppu;
}

DisplayController::~DisplayController() {
	// TODO Auto-generated destructor stub
}

void DisplayController::update() {
	array2d pixelBuffer = ppu->getpixelBuffer();
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 240; j++) {

		}
	}
}

