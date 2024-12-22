#include "DoubleDamage.h"
#include "Exceptions.h"

void DoubleDamage::apply(GameField& field, ShipManager& manager, Coords coords) {
    if (!field.coordinatsInField(coords)) throw OutOfFieldAttackException();
    Cell& cell = field.getCellAt(coords);
    if (cell.ship_is_here) {
        cell.ship_segment_pointer->ship_pointer->damageSegment(cell.ship_segment_pointer, 2);
        cell.status_cell = Status::Ship; // Изменяем статус клетки на корабль
    }
    else {
        cell.status_cell = Status::Empty; // Изменяем статус клетки на корабль
    }
}

std::string DoubleDamage::getName() const {
    return "DoubleDamage";
}
