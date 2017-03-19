#ifndef DISPLAYCONTROLLER_H_
#define DISPLAYCONTROLLER_H_

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "PPU.h"
#include "Windows.h"

template <typename T, size_t M, size_t N> using array2d = std::array<std::array<T, N>, M>;

class DisplayController {
public:
	DisplayController(sf::RenderWindow *window);
	~DisplayController();
	void update();
private:
    array2d<uint8_t, 256, 240> pixelBuffer;
	sf::RenderWindow *window;
};


#endif
