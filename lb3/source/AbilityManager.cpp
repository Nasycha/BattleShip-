#include "AbilityManager.h"
#include "DoubleDamage.h"
#include "Scanner.h"
#include "Bombardment.h"
#include <ctime>
#include <cstdlib>
#include <iostream>

AbilityManager::AbilityManager() {
    srand(time(0));

    allAbilities.emplace_back(std::make_unique<DoubleDamage>());
    allAbilities.emplace_back(std::make_unique<Scanner>());
    allAbilities.emplace_back(std::make_unique<Bombardment>());

    grantRandomAbility();
}

AbilityManager::AbilityManager(AbilityManager&& other) noexcept
    : abilityQueue(std::move(other.abilityQueue)),
      allAbilities(std::move(other.allAbilities)) {}

AbilityManager& AbilityManager::operator=(AbilityManager&& other) noexcept {
    if (this != &other) {
        abilityQueue = std::move(other.abilityQueue);
        allAbilities = std::move(other.allAbilities);
    }
    return *this;
}

AbilityManager::AbilityManager(std::deque<std::unique_ptr<Ability>>&& initialQueue)
    : abilityQueue(std::move(initialQueue)) {
    srand(time(0));

    allAbilities.emplace_back(std::make_unique<DoubleDamage>());
    allAbilities.emplace_back(std::make_unique<Scanner>());
    allAbilities.emplace_back(std::make_unique<Bombardment>());
}

void AbilityManager::grantRandomAbility() {
    if (!allAbilities.empty()) {
        int index = rand() % allAbilities.size();
        if (dynamic_cast<DoubleDamage*>(allAbilities[index].get())) {
            abilityQueue.push_back(std::make_unique<DoubleDamage>());
        } else if (dynamic_cast<Scanner*>(allAbilities[index].get())) {
            abilityQueue.push_back(std::make_unique<Scanner>());
        } else if (dynamic_cast<Bombardment*>(allAbilities[index].get())) {
            abilityQueue.push_back(std::make_unique<Bombardment>());
        }
    }
}

void AbilityManager::applyAbility(GameField& field, ShipManager& manager, Coords coords) {
    if (abilityQueue.empty()) {
        throw NoAvailableAbilitiesException();
    }

    std::unique_ptr<Ability> ability = std::move(abilityQueue.front());
    abilityQueue.pop_front();
    ability->apply(field, manager, coords);
}

void AbilityManager::addAbility(std::unique_ptr<Ability> ability) {
    abilityQueue.push_back(std::move(ability));
}

void AbilityManager::printAbilities() const {
    if (abilityQueue.empty()) {
        std::cout << "No abilities available in the queue.\n";
    } else {
        std::cout << "Abilities in the queue:\n";
        int index = 1;
        for (const auto& ability : abilityQueue) {
            std::cout << index++ << ". ";
            if (dynamic_cast<DoubleDamage*>(ability.get())) {
                std::cout << "Double Damage\n";
            } else if (dynamic_cast<Scanner*>(ability.get())) {
                std::cout << "Scanner\n";
            } else if (dynamic_cast<Bombardment*>(ability.get())) {
                std::cout << "Bombardment\n";
            }
        }
    }
}

const std::deque<std::unique_ptr<Ability>>& AbilityManager::getQueue() const {
    return abilityQueue;
}
