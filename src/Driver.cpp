#include <iostream>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "CPU.h"
#include "Cartridge.h"
#include "Windows.h"

int main(int argc, char * argv[]) {
    std::string filePath = "C:/Users/Orcworm/Documents/GitHub/nes/roms/mario.nes";
    Cartridge cartridge(filePath);

    PPU *ppu = new PPU();
    CPU cpu(ppu, cartridge.getPrgRom());
    cpu.reset();



    sf::RenderWindow window(sf::VideoMode(256,240), "NES Emulator");
 	window.setFramerateLimit(60);

 	while (true) {
		while (window.isOpen()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window.close();
					return 0;
				}
			}

			cpu.cycle();
			if (ppu->getvBlank()) {
				cpu.NMI();
			}

			window.clear(sf::Color::Black);
			window.draw(sf::CircleShape(50.0F, 30));
			window.display();
		}
	}

}

