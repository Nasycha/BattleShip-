#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include "GameField.h"
#include "ShipManager.h"

class NoAvailableAbilitiesException : public std::runtime_error {
public:
    NoAvailableAbilitiesException() : std::runtime_error("No available abilities to apply.") {}
};

class Ability {
public:
    virtual void apply(GameField& field, ShipManager& manager, Coords coords) = 0;
    virtual std::string getName() const = 0;
    virtual ~Ability() = default;
};