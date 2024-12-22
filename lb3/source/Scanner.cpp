#include "Scanner.h"
#include <iostream>

void Scanner::apply(GameField& field, ShipManager& manager, Coords coords) {
    for (int y = coords.y; y < coords.y + 2; ++y) {
        for (int x = coords.x; x < coords.x + 2; ++x) {
            if (field.coordinatsInField({x, y}) && field.getField()[y][x].ship_is_here) {
                std::cout << "Ship segment found at [" << x << "][" << y << "]\n";
            }
        }
    }
}

std::string Scanner::getName() const {
    return "Scanner";
}
