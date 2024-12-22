#pragma once

#include "Ability.h"
#include "DoubleDamage.h"
#include "Scanner.h"
#include "Bombardment.h"

class AbilityManager {
private:
    std::deque<std::unique_ptr<Ability>> abilityQueue;
    std::vector<std::unique_ptr<Ability>> allAbilities;

public:
    AbilityManager();
    AbilityManager(AbilityManager&& other) noexcept;
    AbilityManager& operator=(AbilityManager&& other) noexcept;
    AbilityManager(std::deque<std::unique_ptr<Ability>>&& initialQueue);

    void grantRandomAbility();
    void applyAbility(GameField& field, ShipManager& manager, Coords coords);
    void addAbility(std::unique_ptr<Ability> ability);
    void printAbilities() const;
    const std::deque<std::unique_ptr<Ability>>& getQueue() const;
};