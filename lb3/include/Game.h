#pragma once

#include "GameField.h"
#include "ShipManager.h"
#include "AbilityManager.h"

using namespace std;

class Game {
private:
    GameField playerField;
    GameField enemyField;
    ShipManager playerShipManager;
    ShipManager enemyShipManager;
    AbilityManager playerAbilitiesManager;

    int roundCounter = 0;
    bool isPlayerStep = true;
    bool isPlayerUseAbility = false;
    bool isPlayerDoAttack = false;

    void initializeGame();
    void print_fields();
    void resetEnemy();
    void playerTurn(int& enemyShipCount);
    void enemyTurn(int& playerShipCount);
    bool playRound();

public:
    Game();
    // Измененный конструктор для перемещения
    Game(GameField playerField, GameField enemyField,
         ShipManager playerShipManager, ShipManager enemyShipManager,
         AbilityManager playerAbilitiesManager, int roundCounter,
         bool isPlayerStep, bool isPlayerUseAbility, bool isPlayerDoAttack)
        : playerField(std::move(playerField)),
          enemyField(std::move(enemyField)),
          playerShipManager(std::move(playerShipManager)),
          enemyShipManager(std::move(enemyShipManager)),
          playerAbilitiesManager(std::move(playerAbilitiesManager)),
          roundCounter(roundCounter),
          isPlayerStep(isPlayerStep),
          isPlayerUseAbility(isPlayerUseAbility),
          isPlayerDoAttack(isPlayerDoAttack) {}

    void startGame(int loaded);
    void saveGame(const string& file_name);
    bool loadGame(const string& file_name);

    // Методы для получения текущего состояния
    int getRoundCounter() const { return roundCounter; }
    bool getIsPlayerStep() const { return isPlayerStep; }
    bool getIsPlayerUseAbility() const { return isPlayerUseAbility; }
    bool getIsPlayerDoAttack() const { return isPlayerDoAttack; }

    GameField& getPlayerField() { return playerField; }
    GameField& getEnemyField() { return enemyField; }
    ShipManager& getPlayerShipManager() { return playerShipManager; }
    ShipManager& getEnemyShipManager() { return enemyShipManager; }
    AbilityManager& getPlayerAbilitiesManager() { return playerAbilitiesManager; }
};
