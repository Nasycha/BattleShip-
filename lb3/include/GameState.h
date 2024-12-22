#pragma once

#include <string>
#include "json.hpp"
#include "Game.h"
#include "FileHandler.h"
#include "Exceptions.h"
#include "json.hpp"
#include <stdexcept>
using namespace std;

using json = nlohmann::json;

class GameState {
private:
    json current_state;

public:
    GameState(const string& file_name);
    GameState(Game& game);
    ~GameState() = default;

    json saveFieldToJson(const GameField& field);
    json saveShipsToJson(const ShipManager& ship_manager);
    json saveAbilityManagerToJson(const AbilityManager& ability_manager);

    bool save(const string& file_name);

    GameField loadFieldFromJson(int width, int height, json& field_data);
    ShipManager loadShipManagerFromJson(json& ship_data, GameField& field);
    AbilityManager loadAbilityManagerFromJson(json& ability_data);

    Game load();

    friend ostream& operator<<(ostream& os, GameState& state);
    friend istream& operator>>(istream& is, GameState& state);

    // Getter methods for JSON data
    const json& getCurrentState() const { return current_state; }
    json& getPlayerFieldData() { return current_state["player_field"]; }
    json& getEnemyFieldData() { return current_state["bot_field"]; }
    json& getPlayerShipData() { return current_state["player_ship_data"]; }
    json& getEnemyShipData() { return current_state["bot_ship_data"]; }
    json& getAbilityManagerData() { return current_state["player_ability_manager"]; }

};