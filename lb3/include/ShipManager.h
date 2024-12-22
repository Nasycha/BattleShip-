#pragma once
#include <vector>
#include "Ship.h"
using namespace std;


class ShipManager {
private:
    vector<Ship> free_ships;
    vector<Ship> active_ships;

public:
    ShipManager(vector<int> sizes);
    int getAliveShipsNumber();
    int getDestroyedShipsNumber();
    int getShipCount() const;
    int getFreeShipCount();
    int getActiveShipCount();
    Ship& getFreeShip(int index);
    const Ship& getActiveShip(int index) const;
    vector<Ship>& getActiveShips();
    void moveShipToActive(int index);
    vector<int> getShipSizes();
};
