#include "GameState.h"
#include <iostream>
#include "FileExeption.h"

GameState::GameState(Game& game) : current_state() {
    // Основные параметры
    current_state["round_counter"] = game.getRoundCounter();
    current_state["is_player_step"] = game.getIsPlayerStep();
    current_state["is_player_use_ability"] = game.getIsPlayerUseAbility();
    current_state["is_player_do_attack"] = game.getIsPlayerDoAttack();

    // Параметры полей
    const GameField& playerField = game.getPlayerField();
    const GameField& enemyField = game.getEnemyField();

    current_state["player_field_width"] = playerField.getWidth();
    current_state["player_field_height"] = playerField.getHeight();
    current_state["player_field"] = saveFieldToJson(playerField);
    current_state["bot_field_width"] = enemyField.getWidth();
    current_state["bot_field_height"] = enemyField.getHeight();
    current_state["bot_field"] = saveFieldToJson(enemyField);

    // Управление кораблями
    current_state["player_ship_data"] = saveShipsToJson(game.getPlayerShipManager());
    current_state["bot_ship_data"] = saveShipsToJson(game.getEnemyShipManager());

    // Управление способностями
    current_state["player_ability_manager"] = saveAbilityManagerToJson(game.getPlayerAbilitiesManager());
}

GameState::GameState(const string& file_name) {
    try {
        FileHandler file_handler(file_name);
        file_handler.openForRead();
        file_handler.read(current_state);
        file_handler.closeRead();
    } catch (const FileExeption& e) {
        cerr << "Error while loading game state: " << e.what() << endl;
    }
}

json GameState::saveFieldToJson(const GameField& field) {
    json field_json = json::array();
    for (int y = 0; y < field.getHeight(); ++y) {
        json row = json::array();
        for (int x = 0; x < field.getWidth(); ++x) {
            row.push_back(static_cast<int>(field.getCellStatus({x, y})));
        }
        field_json.push_back(row);
    }
    return field_json;
}


json GameState::saveShipsToJson(const ShipManager& ship_manager) {
    json ships_data;
    for (int i = 0; i < ship_manager.getShipCount(); i++) {
        const Ship& ship = ship_manager.getActiveShip(i);
        json ship_data;
        ship_data["coords"] = json::array({ship.getCoords().x, ship.getCoords().y});
        ship_data["orientation"] = static_cast<int>(ship.getOrientation());
        json segments_data;
        for (int j = 0; j < ship.getLength(); j++) {
            const Ship::ShipSegment* segment = ship.getSegmentByIndex(j);
            json segment_data;
            segment_data["status"] = static_cast<int>(segment->getStatus());
            segments_data.push_back(segment_data);
        }
        ship_data["segments"] = segments_data;
        ships_data.push_back(ship_data);
    }
    return ships_data;
}


json GameState::saveAbilityManagerToJson(const AbilityManager& ability_manager) {
    json abilities_json = json::array();
    for (const auto& ability : ability_manager.getQueue()) {
        abilities_json.push_back(ability->getName());
    }
    return abilities_json;
}

bool GameState::save(const string& file_name) {
    try {
        FileHandler file_handler(file_name);
        file_handler.openForWrite();
        file_handler.write(current_state);
        file_handler.closeWrite();
        return true;
    } catch (const FileExeption& e) {
        cerr << "Error while saving game state: " << e.what() << endl;
    }
    return false;
}

GameField GameState::loadFieldFromJson(int width, int height, json& field_data) {
    GameField field(width, height);
    for (int y = 0; y < field_data.size(); ++y) {
        for (int x = 0; x < field_data[y].size(); ++x) {
            Status cellStatus = static_cast<Status>(field_data[y][x].get<int>());
            field.setCellStatus({x, y}, cellStatus);

            // Если ячейка имеет статус 1, помечаем её как промах
            if (cellStatus == Status::Empty) {
                field.getCellAt({x, y}).missed = true;
            }
        }
    }
    return field;
}


ShipManager GameState::loadShipManagerFromJson(json& ships_data, GameField& field) {
    vector<int> ship_sizes;

    // Сначала собираем размеры кораблей
    for (const auto& ship_data : ships_data) {
        if (!ship_data.contains("coords") || !ship_data.contains("orientation") || !ship_data.contains("segments")) {
            throw std::runtime_error("Invalid ship data format");
        }
        ship_sizes.push_back(ship_data["segments"].size());
    }

    ShipManager ship_manager(ship_sizes);

    // Теперь размещаем корабли
    for (int i = 0; i < ships_data.size(); ++i) {
        const auto& ship_data = ships_data[i];
        Ship& ship = ship_manager.getFreeShip(0); // Всегда берём первый свободный корабль

        // Получаем координаты и ориентацию
        Coords coords = {ship_data["coords"][0].get<int>(), ship_data["coords"][1].get<int>()};
        Orientation orientation = static_cast<Orientation>(ship_data["orientation"].get<int>()); 

        // Размещаем корабль на поле
        field.placeShip(ship, coords, orientation);

        // Перемещаем корабль в активные
        ship_manager.moveShipToActive(0);

        // Проходим по сегментам корабля и наносим урон в соответствии со статусом
        const auto& segments_data = ship_data["segments"];
        for (size_t j = 0; j < segments_data.size(); ++j) {
            SegmentStatus segment_status = static_cast<SegmentStatus>(segments_data[j]["status"].get<int>());

            // Вычисляем координаты сегмента на поле
            Coords segment_coords = coords;
            if (orientation == Orientation::Horizontal) {
                segment_coords.x += j;
            } else {
                segment_coords.y += j;
            }

            // Наносим урон в соответствии со статусом сегмента
            if (segment_status == SegmentStatus::Damaged) {
                field.attackCell(segment_coords);
            }
            if (segment_status == SegmentStatus::Destroyed) {
                field.attackCell(segment_coords);
                field.attackCell(segment_coords);
            }
        }
    }

    return ship_manager;
}

AbilityManager GameState::loadAbilityManagerFromJson(json& ability_data) {
    deque<unique_ptr<Ability>> abilities;
    for (const auto& ability_name : ability_data) {
        if (ability_name == "DoubleDamage") {
            abilities.push_back(make_unique<DoubleDamage>());
        } else if (ability_name == "Scanner") {
            abilities.push_back(make_unique<Scanner>());
        } else if (ability_name == "Bombardment") {
            abilities.push_back(make_unique<Bombardment>());
        } else {
            throw runtime_error("Unknown ability name in JSON");
        }
    }
    return AbilityManager(std::move(abilities)); 
}

Game GameState::load() {
    int round = current_state["round_counter"].get<int>();
    bool is_player_step = current_state["is_player_step"].get<bool>();
    bool is_player_use_ability = current_state["is_player_use_ability"].get<bool>();
    bool is_player_do_attack = current_state["is_player_do_attack"].get<bool>();

    int player_field_width = current_state["player_field_width"].get<int>();
    int player_field_height = current_state["player_field_height"].get<int>();
    GameField player_field = loadFieldFromJson(player_field_width, player_field_height, current_state["player_field"]);
    ShipManager player_ship_manager = loadShipManagerFromJson(current_state["player_ship_data"], player_field);
    AbilityManager player_ability_manager = loadAbilityManagerFromJson(current_state["player_ability_manager"]);

    int bot_field_width = current_state["bot_field_width"].get<int>();
    int bot_field_height = current_state["bot_field_height"].get<int>();
    GameField bot_field = loadFieldFromJson(bot_field_width, bot_field_height, current_state["bot_field"]);
    ShipManager bot_ship_manager = loadShipManagerFromJson(current_state["bot_ship_data"], bot_field);

    // Создаём и возвращаем игру напрямую
    return Game(std::move(player_field), std::move(bot_field), std::move(player_ship_manager), std::move(bot_ship_manager),
                std::move(player_ability_manager), round, is_player_step, is_player_use_ability, is_player_do_attack);
}





ostream& operator<<(ostream& os, GameState& state) {
    os << state.current_state.dump(4) << "\n";
    return os;
}

istream& operator>>(istream& is, GameState& state) {
    json input_data;
    is >> input_data;
    state.current_state = input_data;
    return is;
}