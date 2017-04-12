#include <iostream>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "CPU.h"
#include "Cartridge.h"
#include "Windows.h"

void emulate(CPU cpu, PPU *ppu) {
	while (true) {

	    for (int scanline = 0; scanline < 262; scanline++) {
	        int i = 0;
	        for (int i = 0; i < 113; i++) {
	        	cpu.cycle();
	        	i++;
	        }

	        ppu->cycle(scanline);
	        if (ppu->isNMI()) {
	        	cpu.NMI();
	        }

	    }
	}
}

int main(int argc, char * argv[]) {
    std::string filePath = "C:/Users/Orcworm/Documents/GitHub/nes/roms/mario.nes";
    Cartridge cartridge(filePath);
    cartridge.debug();

    PPU *ppu = new PPU(cartridge.getchrRom(), cartridge.getromControlByte1());
    CPU cpu(ppu, cartridge.getprgRom(), cartridge.getprgRomBanks());

 	cpu.setDebug(1);

    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(256, 240), "NES Emulator");
    sf::Texture texture;
    texture.create(256, 240);
    sf::Sprite sprite(texture);

    std::uint8_t *pixels = new std::uint8_t[4 * 256 * 240];

 	std::thread emulateThread(emulate, cpu, ppu);
 	emulateThread.detach();

 	while (true) {
		while (window->isOpen()) {
			sf::Event event;
			while (window->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window->close();
					return 0;
				}
			}

			window->clear(sf::Color::Black);

			array2d<colour, 256, 240> pixelBuffer = ppu->getpixelBuffer();

			int counter = 0;

		    for (unsigned int y = 0; y < 240; y++) {
		      for (unsigned int x = 0; x < 256; x++) {
		        colour colour = pixelBuffer[x][y];
		        //cout << "color.r: " << dec << +color.r << endl;
		        pixels[4 * counter] = colour.r;
		        pixels[4 * counter + 1] = colour.g;
		        pixels[4 * counter + 2] = colour.b;
		        pixels[4 * counter + 3] = 255;
		        counter++;
		        //cout << +pixels[4*pos] << endl;
		      }
		    }

		    texture.update(pixels);
			window->draw(sprite);
			window->display();
		}
	}
}


