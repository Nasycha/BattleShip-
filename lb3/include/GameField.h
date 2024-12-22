#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include "Ship.h"

constexpr int minimalFieldSize = 3;
constexpr int maximalFieldSize = 25;

enum class Status { Unknown, Empty, Ship };

struct Cell {
    bool ship_is_here = false;
    bool ship_is_nearly = false;
    Status status_cell { Status::Unknown };
    Ship::ShipSegment* ship_segment_pointer = nullptr;
    bool missed = false; // для промахов чтобы печатать x вместо o
};

class GameField {
private:
    int height;
    int width;
    std::vector<std::vector<Cell>> field; // матрица поля из информации о клетке

public:
    GameField();
    GameField(int new_width, int new_height);
    GameField(const GameField& other);
    GameField(GameField&& other) noexcept;

    GameField& operator=(const GameField& other);
    GameField& operator=(GameField&& other) noexcept;

    Cell& getCellAt(const Coords& coords);
    const Cell& getCellAt(const Coords& coords) const;
    std::vector<std::vector<Cell>>& getField();
    int getWidth() const;
    int getHeight() const;
    bool coordinatsInField(Coords coords_check) const;
    bool shipCoordinatesCorrect(Coords top_left, Orientation orientation, int size);
    void resizeSize(int width, int height);
    void placeShip(Ship& ship, Coords top_left, Orientation orientation);
    void attackCell(Coords attack_coords);
    void print_field() const;
    Status getCellStatus(const Coords& coords) const;
    void setCellStatus(const Coords& coords, Status new_status);
    bool isShipHereDestroyed(const Coords& coords);
    bool isShipHere(const Coords& coords);
};