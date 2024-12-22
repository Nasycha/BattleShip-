#pragma once

#include <stdexcept>

class ShipPlacementException : public std::runtime_error {
public:
    ShipPlacementException() : std::runtime_error("Invalid ship placement. Ships are too close or overlapping.") {}
};

class OutOfFieldAttackException : public std::runtime_error {
public:
    OutOfFieldAttackException() : std::runtime_error("Attack out of field bounds.") {}
};