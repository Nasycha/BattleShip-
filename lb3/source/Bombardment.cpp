#include "Bombardment.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

void Bombardment::apply(GameField& field, ShipManager& manager, Coords coords) {
    if (!manager.getActiveShips().empty()) {
        int shipIndex = rand() % manager.getActiveShips().size();
        Ship& ship = manager.getActiveShips()[shipIndex];
        Ship::ShipSegment* segment = nullptr;
        int maxAttempts = ship.getLength();

        while (maxAttempts-- > 0) {
            int segmentIndex = rand() % ship.getLength();
            segment = const_cast<Ship::ShipSegment*>(ship.getSegmentByIndex(segmentIndex));
            if (segment != nullptr && segment->status != SegmentStatus::Destroyed) {
                segment->ship_pointer->damageSegment(const_cast<Ship::ShipSegment*>(segment), 1);
                break;
            }
        }
    }
}

std::string Bombardment::getName() const {
    return "Bombardment";
}
