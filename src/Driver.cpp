#include <iostream>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "CPU.h"
#include "Cartridge.h"
#include "Windows.h"
#include "DisplayController.h"

int main(int argc, char * argv[]) {
    std::string filePath = "C:/Users/Orcworm/Documents/GitHub/nes/roms/mario.nes";
    Cartridge cartridge(filePath);

    PPU *ppu = new PPU();
    CPU cpu(ppu, cartridge.getPrgRom());
    cpu.reset();



//    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(256,240), "NES Emulator");
// 	window->setFramerateLimit(60);

 	//DisplayController controller(window);

// 	while (true) {
//		while (window->isOpen()) {
//			sf::Event event;
//			while (window->pollEvent(event)) {
//				if (event.type == sf::Event::Closed) {
//					window->close();
//					return 0;
//				}
//			}
//			for (int i = 0; i <= 261; i++) {
//				int clock = 0;
//				while (clock <= 113) {
//					cpu.cycle();
//					clock +=3;
//				}
//				ppu->renderScanline(i);
//				if (ppu->getvBlank()) {
//					cpu.NMI();
//				}
//				// TODO instead of 261 loop, have ppu run 3x for every 1x cpu?
//			}
//
//			window->clear(sf::Color::Black);
//			window->draw(sf::CircleShape(20.0F, 60));
//			window->display();
//		}
//	}

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
//			array2d pixelBuffer = ppu->getpixelBuffer();
//			for (int i = 0; i < 256; i++) {
//				for (int j = 0; j < 240; j++) {
//
//				}
//			}
//
//			window->display();
//		}
//	}

}

void emulate() {

}

