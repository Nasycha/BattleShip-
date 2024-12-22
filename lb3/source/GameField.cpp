#include "GameField.h"
#include <iostream>
#include <stdexcept>

GameField::GameField() {
    height = 0;
    width = 0;
    field;
}

GameField::GameField(int new_width, int new_height) {
    if (new_width < minimalFieldSize || new_height < minimalFieldSize || new_width > maximalFieldSize || new_height > maximalFieldSize) {
        throw std::logic_error("Invalid field size");
    }
    width = new_width;
    height = new_height;
    field.resize(height, std::vector<Cell>(width));
}

GameField::GameField(const GameField& other) {
    width = other.width;
    height = other.height;
    field = other.field;
}

GameField::GameField(GameField&& other) noexcept {
    width = std::move(other.width);
    height = std::move(other.height);
    field = std::move(other.field);
}

GameField& GameField::operator=(const GameField& other) {
    if (this == &other) {
        return *this;
    }

    width = other.width;
    height = other.height;
    field = other.field;

    return *this;
}

GameField& GameField::operator=(GameField&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    width = std::move(other.width);
    height = std::move(other.height);
    field = std::move(other.field);

    return *this;
}

const Cell& GameField::getCellAt(const Coords& coords) const {
    if (!coordinatsInField(coords)) {
        throw std::out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x];
}

Cell& GameField::getCellAt(const Coords& coords) {
    if (!coordinatsInField(coords)) {
        throw std::out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x];
}

std::vector<std::vector<Cell>>& GameField::getField() {
    return field;
}

int GameField::getWidth() const {
    return width;
}

int GameField::getHeight() const {
    return height;
}

bool GameField::coordinatsInField(Coords coords_check) const {
    if (coords_check.x < 0 || coords_check.x >= width || coords_check.y < 0 || coords_check.y >= height) {
        return false;
    }
    else {
        return true;
    }
}

bool GameField::shipCoordinatesCorrect(Coords top_left, Orientation orientation, int size) {
    Coords coords_cell = top_left;

    for (int i = 0; i < size; ++i) {
        if (orientation == Orientation::Vertical) {
            coords_cell.y = top_left.y + i;
        } else {
            coords_cell.x = top_left.x + i;
        }

        if (!coordinatsInField(coords_cell)) {
            return false;
        }

        if (field[coords_cell.y][coords_cell.x].ship_is_here ||
            field[coords_cell.y][coords_cell.x].ship_is_nearly) {
            return false;
        }
    }
    return true;
}

void GameField::resizeSize(int new_width, int new_height) {
    width = new_width;
    height = new_height;
    field.clear();
    field.resize(height, std::vector<Cell>(width));
}

void GameField::placeShip(Ship& ship, Coords top_left, Orientation orientation) {
    if (!shipCoordinatesCorrect(top_left, orientation, ship.getLength())) {
        throw std::out_of_range("Ship coordinates out of range");
    }

    ship.setShipCoordinates(orientation, top_left);

    Coords ship_cell = top_left;
    for (int i = 0; i < ship.getLength(); i++) {
        if (ship.getOrientation() == Orientation::Vertical) {
            ship_cell.y = top_left.y + i;
        } else {
            ship_cell.x = top_left.x + i;
        }
        field[ship_cell.y][ship_cell.x].ship_is_here = true;
        field[ship_cell.y][ship_cell.x].ship_segment_pointer = const_cast<Ship::ShipSegment*>(ship.getSegmentByIndex(i));
    }

    int offset_x = (orientation == Orientation::Vertical) ? 0 : ship.getLength() - 1;
    int offset_y = (orientation == Orientation::Vertical) ? ship.getLength() - 1 : 0;

    for (int x = top_left.x - 1; x <= top_left.x + offset_x; x++) {
        for (int y = top_left.y - 1; y <= top_left.y + offset_y; y++) {
            if (coordinatsInField({x, y})) {
                if (!field[y][x].ship_is_here) {
                    field[y][x].ship_is_nearly = true;
                }
            }
        }
    }
}

void GameField::attackCell(Coords attack_coords) {
    if (!coordinatsInField(attack_coords)) {
        throw std::out_of_range("Ship coordinates out of range");
    }

    if (field[attack_coords.y][attack_coords.x].ship_is_here) {
        field[attack_coords.y][attack_coords.x].status_cell = Status::Ship;
        field[attack_coords.y][attack_coords.x].ship_segment_pointer->ship_pointer->damageSegment(field[attack_coords.y][attack_coords.x].ship_segment_pointer, 1);
    } else {
        field[attack_coords.y][attack_coords.x].status_cell = Status::Empty;
        field[attack_coords.y][attack_coords.x].missed = true;
    }
}

void GameField::print_field() const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (field[y][x].ship_is_here == true) {
                if (field[y][x].ship_segment_pointer->status == SegmentStatus::Damaged) {
                    std::cout << "1";
                }
                if (field[y][x].ship_segment_pointer->status == SegmentStatus::Destroyed) {
                    std::cout << "0";
                }
                if (field[y][x].ship_segment_pointer->status == SegmentStatus::Intact) {
                    std::cout << "2";
                }
            }
            else {
                if (field[y][x].missed) {
                    std::cout << "x";
                }
                else {
                    std::cout << ".";
                }
            }
        }
        std::cout << "\n";
    }
}

Status GameField::getCellStatus(const Coords& coords) const {
    if (!coordinatsInField(coords)) {
        throw std::out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x].status_cell;
}

void GameField::setCellStatus(const Coords& coords, Status new_status) {
    if (!coordinatsInField(coords)) {
        throw std::out_of_range("Coordinates are out of field bounds");
    }
    field[coords.y][coords.x].status_cell = new_status;
}

bool GameField::isShipHereDestroyed(const Coords& coords) {
    if (!coordinatsInField(coords)) {
        throw std::out_of_range("Coordinates are out of field bounds");
    }
    if (field[coords.y][coords.x].ship_is_here && field[coords.y][coords.x].ship_segment_pointer) {
        return field[coords.y][coords.x].ship_segment_pointer->status == SegmentStatus::Destroyed;
    }
    return false;
}

bool GameField::isShipHere(const Coords& coords) {
    if (!coordinatsInField(coords)) {
        throw std::out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x].ship_is_here;
}
