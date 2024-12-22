#pragma once

#include <vector>
#include <string>
#include <stdexcept>

constexpr int minimalShipLength = 1;
constexpr int maximalShipLength = 4;

using namespace std;

struct Coords {
    int x;
    int y;
};

enum class SegmentStatus { Intact, Damaged, Destroyed };

enum class Orientation { Horizontal, Vertical, Unknown };

class Ship {
public:
    struct ShipSegment {
        Coords coords;
        SegmentStatus status { SegmentStatus::Intact };
        Ship* ship_pointer = nullptr;

        void setStatus(SegmentStatus new_status){
            status = new_status;
        }

        SegmentStatus getStatus() const {
            return status;
        }
    };

    Ship();
    Ship(int size);
    Ship(const Ship& other);
    Ship(Ship&& other) noexcept;
    Ship& operator=(const Ship& other);
    Ship& operator=(Ship&& other) noexcept;

    int getLength() const;
    int getMaxHealth();
    int getCurrentHealth();
    void setLength_and_health(int new_length);
    Orientation getOrientation() const;
    void setOrientation(Orientation new_orientation);
    bool isAlive() const;
    bool isUnharmed();
    bool isDestroyed();
    void setShipCoordinates(Orientation orientation, Coords new_top_left);
    void damageSegment(ShipSegment* segment, int damage = 1);
    const ShipSegment* getSegmentByIndex(int index) const;
    void setSegmentByIndex(int index, const ShipSegment& segment);
    Coords getCoords() const;
    void setCoords(const Coords& new_coords);
    void print_info() const;
    const std::vector<Ship::ShipSegment>& getSegments() const;

private:
    int length;
    Orientation orientation;
    Coords top_left;
    int max_health;
    int health;
    std::vector<ShipSegment> segments;
};