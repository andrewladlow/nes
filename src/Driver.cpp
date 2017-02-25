#include <iostream>
#include "CPU.h"
#include "Cartridge.h"
#include "Windows.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

using namespace std;

//string ExePath() {
//    char buffer[MAX_PATH];
//    GetModuleFileNameA( NULL, buffer, MAX_PATH );
//    string::size_type pos = string( buffer ).find_last_of( "\\/" );
//    return string( buffer ).substr( 0, pos);
//}


int main(int argc, char * argv[]) {

    //cout << "my directory is " << ExePath() << "\n";

    string filePath = "C:/Users/Orcworm/Documents/GitHub/nes/roms/mario.nes";
    Cartridge cartridge(filePath);

    PPU *ppu = new PPU();
    CPU cpu(ppu, cartridge.getPrgRom());

    cpu.reset();
    //while(true) {
    for (int i = 0; i < 20; i++) {
    	cpu.cycle();
    }



//	sf::RenderWindow window(sf::VideoMode(640,480), "Window");
//
//	while (window.isOpen()) {
//		sf::Event event;
//		while (window.pollEvent(event)) {
//			if (event.type == sf::Event::Closed) {
//				window.close();
//			}
//		}
//
//		window.clear(sf::Color::Black);
//
//		window.display();
//	}
    return 0;
}
