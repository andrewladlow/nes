#include <iostream>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "CPU.h"
#include "Cartridge.h"
#include "Windows.h"

int main(int argc, char * argv[]) {
    std::string filePath = "C:/Users/Orcworm/Documents/GitHub/nes/roms/nestest.nes";
    Cartridge cartridge(filePath);
    //cartridge.debug();

    PPU *ppu = new PPU();
    CPU cpu(ppu, cartridge.getprgRom(), cartridge.getprgRomBanks());

 	cpu.setDebug(1);

 	while(true) {
 	//for (int i = 0; i < 3; i++) {
 		cpu.cycle();
 		ppu->cycle();
 	}


 	//DisplayController controller(window);

//    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(256, 240), "NES Emulator");
//    sf::Texture texture;
//    texture.create(256, 240);
//    sf::Sprite sprite(texture);
//
//    std::uint8_t *pixels = new std::uint8_t[256 * 240 * 4];
//
// 	window->setFramerateLimit(60);

// 	while (true) {
//		while (window->isOpen()) {
//			sf::Event event;
//			while (window->pollEvent(event)) {
//				if (event.type == sf::Event::Closed) {
//					window->close();
//					return 0;
//				}
//			}
//			cpu.cycle();
//
//			ppu->cycle();
//			ppu->cycle();
//			ppu->cycle();
//			if (ppu->getvBlank()) {
//				cpu.NMI();
//			}
//
//			window->clear(sf::Color::Black);
//
//			array2d<sf::Color, 256, 240> pixelBuffer = ppu->getpixelBuffer();
//
//			int counter = 0;
//
//		    for (unsigned int y = 0; y < 240; ++y) {
//		      for (unsigned int x = 0; x < 256; ++x) {
//		        sf::Color color = pixelBuffer[x][y];
//		        pixels[counter * 4] = color.r;
//		        pixels[4 * counter + 1] = color.g;
//		        pixels[4 * counter + 2] = color.b;
//		        pixels[4 * counter + 3] = color.a;
//		        counter++;
//		        //cout << +pixels[4*pos] << endl;
//		      }
//		    }
//
//		    texture.update(pixels);
//			window->draw(sprite);
//			window->display();
//		}
//	}

}

