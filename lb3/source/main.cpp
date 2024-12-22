#include <iostream>

#include <stdexcept>
#include "Game.h"
#include "GameState.h"

using namespace std;

//////////////////////////  g++ -I lb3/include lb3/source/GameField.cpp lb3/source/Ship.cpp lb3/source/ShipManager.cpp lb3/source/main.cpp lb3/source/Bombardment.cpp lb3/source/DoubleDamage.cpp  lb3/source/Scanner.cpp lb3/source/AbilityManager.cpp lb3/source/Game.cpp lb3/source/FileHandler.cpp lb3/source/FileExeption.cpp lb3/source/GameState.cpp -o build_lb/lb3
int main() {
    try {
        Game game;
        cout << "Хотите загрузить сохраненную игру? (y/n): ";
        char answer;
        cin >> answer;
        if (answer == 'y') {
            string file_name;
            cout << "Введите имя файла загрузки: ";
            cin >> file_name;
            GameState gameState(file_name);
            game = gameState.load();
            cout << "Игра успешно загружена!\n";
            game.startGame(1);
        }
        else{
            game.startGame(0);
        }
    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << "\n";
    } catch (...) {
        cerr << "An unknown error occurred.\n";
    }
    return 0;
}
