#include "ShipManager.h"

ShipManager::ShipManager(std::vector<int> sizes) {
    for (int size : sizes) {
        free_ships.emplace_back(size);
    }
}

int ShipManager::getAliveShipsNumber() {
    int shipNum = 0;
    for (const auto& ship : active_ships) {
        if (ship.isAlive()) {
            shipNum++;
        }
    }
    return shipNum;
}

int ShipManager::getDestroyedShipsNumber() {
    int shipNum = 0;
    for (const auto& ship : active_ships) {
        if (!(ship.isAlive())) {
            shipNum++;
        }
    }
    return shipNum;
}

int ShipManager::getShipCount() const {
    return active_ships.size();
}

int ShipManager::getFreeShipCount() {
    return free_ships.size();
}

int ShipManager::getActiveShipCount() {
    return active_ships.size();
}

Ship& ShipManager::getFreeShip(int index) {
    if (index >= 0 && index < free_ships.size()) {
        return free_ships[index];
    }
    throw std::out_of_range("Invalid index for free ship");
}

const Ship& ShipManager::getActiveShip(int index) const {
    if (index >= 0 && index < active_ships.size()) {
        return active_ships[index];
    }
    throw std::out_of_range("Invalid index for active ship");
}

std::vector<Ship>& ShipManager::getActiveShips() {
    return active_ships;
}

void ShipManager::moveShipToActive(int index) {
    if (index >= 0 && index < free_ships.size()) {
        active_ships.push_back(std::move(free_ships[index]));
        free_ships.erase(free_ships.begin() + index);
    } else {
        throw std::out_of_range("Invalid index for moving ship");
    }
}

std::vector<int> ShipManager::getShipSizes() {
    std::vector<int> sizes;
    for (const auto& ship : active_ships) {
        sizes.push_back(ship.getLength());
    }
    return sizes;
}
