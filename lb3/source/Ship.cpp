#include "Ship.h"
#include <iostream>
#include <stdexcept>
#include <string>

Ship::Ship() {
    length = 0;
    orientation = Orientation::Unknown;
    top_left = {-1, -1};
    max_health = 0;
    health = 0;
}

Ship::Ship(int new_size) {
    if (new_size < minimalShipLength || new_size > maximalShipLength)
        throw logic_error("Invalid ship size");

    length = new_size;
    orientation = Orientation::Unknown;
    top_left = {-1, -1};
    max_health = new_size * 2;
    health = new_size * 2;
}

Ship::Ship(const Ship& other) { // глубокое копирование
    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = other.segments;
}

Ship::Ship(Ship&& other) noexcept { // Конструктор перемещения
    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = move(other.segments);
}

Ship& Ship::operator=(const Ship& other) { // оператор присваивания
    if (this == &other) {
        return *this;
    }

    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = other.segments; // Копируем сегменты

    return *this;
}

Ship& Ship::operator=(Ship&& other) noexcept { // оператор присваивания
    if (this == &other) {
        return *this;
    }

    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = std::move(other.segments); // Перемещаем сегменты

    return *this;
}

int Ship::getLength() const {
    return length;
}

int Ship::getMaxHealth() {
    return max_health;
}

int Ship::getCurrentHealth() {
    int recalculatedHealth = 0;
    for (const auto& segment : segments) {
        if (segment.status == SegmentStatus::Intact) {
            recalculatedHealth += 2; // Intact сегмент стоит 2 здоровья
        } else if (segment.status == SegmentStatus::Damaged) {
            recalculatedHealth += 1; // Damaged сегмент стоит 1 здоровье
        }
    }
    health = recalculatedHealth; // Обновляем текущее здоровье
    return health;
}


void Ship::setLength_and_health(int new_length) {
    if (new_length < minimalShipLength || new_length > maximalShipLength) {
        throw std::logic_error("Invalid ship length");
    }
    length = new_length;
    max_health = length * 2; // Максимальное здоровье зависит от длины
    health = max_health;     // Устанавливаем текущее здоровье равным максимуму
    // Инициализируем сегменты
    segments.resize(length, { {0, 0}, SegmentStatus::Intact, this });
}


Orientation Ship::getOrientation() const {
    return orientation;
}

void Ship::setOrientation(Orientation new_orientation) {
    orientation = new_orientation;
}


bool Ship::isAlive() const {
    for (auto& el : segments)
        if (el.status != SegmentStatus::Destroyed)
            return true;
    return false;
}

bool Ship::isUnharmed() {
    return health == max_health;
}

bool Ship::isDestroyed() {
    return health == 0;
}

void Ship::setShipCoordinates(Orientation new_orientation, Coords new_top_left) {
    orientation = new_orientation;
    top_left = new_top_left;

    // Убедимся, что сегменты корректно инициализированы
    if (segments.size() != length) {
        segments.resize(length, { {0, 0}, SegmentStatus::Intact, this });
    }

    // Обновляем координаты сегментов
    for (int i = 0; i < length; ++i) {
        if (orientation == Orientation::Horizontal) {
            segments[i].coords = {top_left.x + i, top_left.y};
        } else {
            segments[i].coords = {top_left.x, top_left.y + i};
        }
        segments[i].ship_pointer = this;
    }
}


void Ship::damageSegment(ShipSegment* segment, int damage) {
    if (segment->status == SegmentStatus::Destroyed) {
        return;
    } else if (segment->status == SegmentStatus::Damaged) {
        segment->status = SegmentStatus::Destroyed;
    } else if (segment->status == SegmentStatus::Intact) {
        segment->status = (damage >= 2) ? SegmentStatus::Destroyed : SegmentStatus::Damaged;
    }
    getCurrentHealth(); // Пересчет здоровья после нанесения урона
}


const Ship::ShipSegment* Ship::getSegmentByIndex(int index) const{ // нужен чтобы связать клетку и корабль
    return (index >= 0 && index < length) ? &segments[index] : nullptr;
}

void Ship::setSegmentByIndex(int index, const ShipSegment& segment) {
    if (index >= 0 && index < length) {
        segments[index] = segment;
        segments[index].ship_pointer = this; // Устанавливаем указатель на текущий корабль
    } else {
        throw std::out_of_range("Invalid segment index in setSegmentByIndex");
    }
}

string orientationToString(Orientation orientation) {
    switch (orientation) {
        case Orientation::Horizontal: return "Horizontal";
        case Orientation::Vertical: return "Vertical";
        default: return "Unknown";
    }
}

Coords Ship::getCoords() const {
    return top_left;
}

void Ship::setCoords(const Coords& new_coords) {
    top_left = new_coords;
}

void Ship::print_info()  const{
    cout << "length " << length << "\n";
    cout << "orientation " << orientationToString(orientation) << "\n";
    cout << "top left [x][y] " << top_left.x << " " << top_left.y << "\n";
    cout << "max health " << max_health << "\n";
    cout << "health " << health << "\n";
}

const std::vector<Ship::ShipSegment>& Ship::getSegments() const {
    return segments;
}


string segmentStatusToString(SegmentStatus status) { // Чтобы правильно вывести статус сегмента, конвертируем значение SegmentStatus в строку
    switch (status) {
        case SegmentStatus::Intact: return "Intact";
        case SegmentStatus::Damaged: return "Damaged";
        case SegmentStatus::Destroyed: return "Destroyed";
        default: return "Unknown";
    }
}

