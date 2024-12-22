#pragma once

#include "Ability.h"

class Scanner : public Ability {
public:
    void apply(GameField& field, ShipManager& manager, Coords coords) override;
    std::string getName() const override;
};