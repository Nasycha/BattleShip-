#include "Game.h"
#include "GameState.h"
#include "Exceptions.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <random>
#include <ctime>
#include <memory>


Game::Game()
    : playerField(10, 10), enemyField(10, 10),
    playerShipManager({2, 1}), enemyShipManager({2, 1}),
    playerAbilitiesManager() {}

void Game::initializeGame() {
    roundCounter = 0; // Обнуляем счетчик раундов
    vector<int> ship_sizes = {2, 1};  // Пример размеров кораблей

    playerField = GameField(10, 10);
    enemyField = GameField(10, 10);

    playerShipManager = ShipManager(ship_sizes);
    enemyShipManager = ShipManager(ship_sizes);

    // Инициализируем поля с кораблями

    // Размещение кораблей игрока на поле
    try {
        Ship& player_ship1 = playerShipManager.getFreeShip(0);
        playerField.placeShip(player_ship1, {2, 1}, Orientation::Horizontal);
        playerShipManager.moveShipToActive(0);

        Ship& player_ship3 = playerShipManager.getFreeShip(0);
        playerField.placeShip(player_ship3, {3, 4}, Orientation::Horizontal);
        playerShipManager.moveShipToActive(0);
    } catch (const exception& e) {
        cerr << "Ошибка при размещении кораблей игрока: " << e.what() << "\n";
    }

    // Размещение кораблей врага на поле
    try {
        Ship& enemy_ship1 = enemyShipManager.getFreeShip(0);
        enemyField.placeShip(enemy_ship1, {6, 3}, Orientation::Horizontal);
        enemyShipManager.moveShipToActive(0);

        Ship& enemy_ship3 = enemyShipManager.getFreeShip(0);
        enemyField.placeShip(enemy_ship3, {7, 7}, Orientation::Vertical);
        enemyShipManager.moveShipToActive(0);
    } catch (const exception& e) {
        cerr << "Ошибка при размещении кораблей врага: " << e.what() << "\n";
    }

    // Вывод начального состояния поля
    cout << "Поле игрока:\n";
    playerField.print_field();
    cout << "Поле врага:\n";
    enemyField.print_field();
}

void Game::print_fields(){
    // Вывод загруженного состояния поля
    cout << "Поле игрока:\n";
    playerField.print_field();
    cout << "Поле врага:\n";
    enemyField.print_field();
}

void Game::resetEnemy() {
    vector<int> shipSizes = {2, 3, 1};
    enemyField = GameField(10, 10);
    enemyShipManager = ShipManager(shipSizes);

    // Размещение кораблей врага на поле
    Ship& enemy_ship1 = enemyShipManager.getFreeShip(0);
    enemyField.placeShip(enemy_ship1, {6, 0}, Orientation::Horizontal);
    enemyShipManager.moveShipToActive(0);

    Ship& enemy_ship2 = enemyShipManager.getFreeShip(0);
    enemyField.placeShip(enemy_ship2, {2, 4}, Orientation::Horizontal);
    enemyShipManager.moveShipToActive(0);

    Ship& enemy_ship3 = enemyShipManager.getFreeShip(0);
    enemyField.placeShip(enemy_ship3, {7, 9}, Orientation::Vertical);
    enemyShipManager.moveShipToActive(0);
}

void Game::saveGame(const string& file_name) {
    GameState state(*this);
    if (state.save(file_name)) {
        cout << "Game saved successfully to " << file_name << "\n";
    } else {
        cerr << "Failed to save the game.\n";
    }
}

// Новый метод для загрузки игры
bool Game::loadGame(const string& file_name) {
    GameState state(file_name);
    try {
        *this = state.load();
        cout << "Game loaded successfully from " << file_name << "\n";
        return true;
    } catch (const exception& e) {
        cerr << "Failed to load the game: " << e.what() << "\n";
    } catch (...) {
        cerr << "An unknown error occurred during game loading.\n";
    }
    return false;
}

void Game::startGame(int loaded=0) {
    while (true) {
        if (loaded == 0){
            initializeGame();
        }
        else{
            print_fields();
        }
        while (playRound()) {
            cout << "Игрок выиграл раунд!\nПерезапускаем врага...\n";
            resetEnemy();
            cout << "Поле игрока:\n";
            playerField.print_field();
            cout << "Поле врага:\n";
            enemyField.print_field();
        }

        cout << "Game over!\nСыграем еще раз? (y/n): ";
        char answer;
        cin >> answer;
        if (answer != 'y') break;
    }
}


void Game::playerTurn(int& enemyShipCount) {
    ++roundCounter; // Увеличиваем счетчик раундов
    isPlayerUseAbility = false;
    isPlayerDoAttack = false;

    // Предложить игроку загрузить сохранение перед ходом
    cout << "Хотите загрузить игру? (y/n): ";
    char answer;
    cin >> answer;
    if (answer == 'y') {
        string file_name;
        cout << "Введите имя файла: ";
        cin >> file_name;
        if (loadGame(file_name)) {
            cout << "Сохранение загружено. Начинаем ход заново с текущего состояния игры.\n";
            return; // Выход из метода, чтобы игрок начал новый ход с загруженного состояния
        }
    }

    // Предложить игроку сохранить игру перед ходом
    cout << "Хотите сохранить игру? (y/n): ";
    cin >> answer;
    if (answer == 'y') {
        string file_name;
        cout << "Введите имя файла: ";
        cin >> file_name;
        saveGame(file_name);
    }

    isPlayerStep = true; // Устанавливаем флаг, что сейчас ход игрока

    // Вывод доступных способностей
    cout << "\nДоступные способности:\n";
    playerAbilitiesManager.printAbilities();
    cout << "Вы хотите использовать способность? y/n\n";
    cin >> answer;

    if (answer == 'y') {
        try {
            int x, y;
            cout << "Введите координаты для применения способности (x y): ";
            cin >> x >> y;

            Coords target1 = {x, y};
            playerAbilitiesManager.applyAbility(enemyField, enemyShipManager, target1);
            isPlayerUseAbility = true;
        } catch (const NoAvailableAbilitiesException& e) {
            cerr << "Ошибка применения способности " << e.what() << "\n";
        } catch (const logic_error& e) {
            cerr << e.what() << endl;
        }
    }

    try {
        int x, y;
        cout << "Введите координаты атаки (x y): ";
        cin >> x >> y;

        enemyField.attackCell({x, y});
        isPlayerDoAttack = true;

    } catch (const OutOfFieldAttackException& e) {
        cerr << e.what() << " Check attack coordinates." << endl;
    } catch (const exception& e) {
        cerr << "Invalid attack: " << e.what() << "\n";
    }

    if (enemyShipCount > enemyShipManager.getAliveShipsNumber()) {
        int destroyedShips = enemyShipCount - enemyShipManager.getAliveShipsNumber();
        for (int i = 0; i < destroyedShips; ++i) {
            isPlayerUseAbility = false;
            playerAbilitiesManager.grantRandomAbility();
        }
        enemyShipCount = enemyShipManager.getAliveShipsNumber();
    }
    // Ход передается боту
    isPlayerStep = false;
}


void Game::enemyTurn(int& playerShipCount) {
    isPlayerStep = false; // Устанавливаем флаг, что сейчас ход игрока

    while (true) {
        try {
            int x = rand() % playerField.getWidth();
            int y = rand() % playerField.getHeight();

            cout << "Атака врага (" << x << ", " << y << ")\n";
            playerField.attackCell({x, y});
            break;
        } catch (const OutOfFieldAttackException& e) {
            cerr << e.what() << " Check attack coordinates." << endl;
        } catch (const exception& e) {
            cerr << "Enemy attack failed: " << e.what() << "\n";
        }
    }

    if (playerShipCount > playerShipManager.getAliveShipsNumber()) {
        playerShipCount = playerShipManager.getAliveShipsNumber();
    }
    // Передача хода обратно игроку
    isPlayerStep = true;
    isPlayerDoAttack = false;
}

bool Game::playRound() {
    int playerShipCount = playerShipManager.getAliveShipsNumber();
    int enemyShipCount = enemyShipManager.getAliveShipsNumber();

    while (true) {
        playerTurn(enemyShipCount);
        if (enemyShipCount == 0) {
            return true;
        }

        enemyTurn(playerShipCount);
        if (playerShipCount == 0) {
            return false;
        }

        cout << "Поле игрока:\n";
        playerField.print_field();
        cout << "Поле врага:\n";
        enemyField.print_field();
    }
}
