#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <random>
#include <ctime>
#include <memory>
#include <queue>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;
//////// g++ -I lb3_ver2/include lb3_ver2/main7.cpp -o build_lb/main7
/////////////////////////////////////////////////////
//////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////

constexpr int minimalShipLength = 1;
constexpr int maximalShipLength = 4;

using namespace std;

struct Coords {
    int x;
    int y;
};

// Статус сегмента корабля
enum class SegmentStatus { Intact, Damaged, Destroyed };

// Ориентация корабля на поле
enum class Orientation { Horizontal, Vertical, Unknown };

class Ship {
public:
    struct ShipSegment {
        Coords coords;
        SegmentStatus status { SegmentStatus::Intact };
        Ship* ship_pointer = nullptr;

        void setStatus(SegmentStatus new_status){
            status = new_status;
        }

        SegmentStatus getStatus() const {
            return status;
        }
    };

    Ship();
    Ship(int size);
    Ship(const Ship& other);
    Ship(Ship&& other) noexcept;
    Ship& operator=(const Ship& other);
    Ship& operator=(Ship&& other) noexcept;

    int getLength() const;
    int getMaxHealth();
    int getCurrentHealth();
    void setLength_and_health(int new_length);
    Orientation getOrientation() const;
    void setOrientation(Orientation new_orientation);
    bool isAlive() const;
    bool isUnharmed();
    bool isDestroyed();
    void setShipCoordinates(Orientation orientation, Coords new_top_left);
    void damageSegment(ShipSegment* segment, int damage = 1);
    const ShipSegment* getSegmentByIndex(int index) const;
    void setSegmentByIndex(int index, const ShipSegment& segment);
    Coords getCoords() const;
    void setCoords(const Coords& new_coords);
    void print_info() const;
    const std::vector<Ship::ShipSegment>& getSegments() const;

private:
    int length;
    Orientation orientation;
    Coords top_left;
    int max_health;
    int health;
    vector<ShipSegment> segments;
};

constexpr int minimalFieldSize = 3;
constexpr int maximalFieldSize = 25;

enum class Status { Unknown, Empty, Ship };



class ShipManager {
private:
    vector<Ship> free_ships;
    vector<Ship> active_ships;

public:
    ShipManager(vector<int> sizes);
    int getAliveShipsNumber();
    int getDestroyedShipsNumber();
    int getShipCount() const;
    int getFreeShipCount();
    int getActiveShipCount();
    Ship& getFreeShip(int index);
    const Ship& getActiveShip(int index) const;
    vector<Ship>& getActiveShips();
    void moveShipToActive(int index);
    vector<int> getShipSizes();
};

struct Cell {
    bool ship_is_here = false;
    bool ship_is_nearly = false;
    Status status_cell { Status::Unknown };
    Ship::ShipSegment* ship_segment_pointer = nullptr;
    bool missed = false; // для промахов чтобы печатать x вместо o
};

class GameField {
private:
    int height;
    int width;
    vector<vector<Cell>> field; // матрица поля из информации о клетке

public:
    GameField();
    GameField(int new_width, int new_height);
    GameField(const GameField& other);
    GameField(GameField&& other) noexcept;

    GameField& operator=(const GameField& other);
    GameField& operator=(GameField&& other) noexcept;

    Cell& getCellAt(const Coords& coords);
    const Cell& getCellAt(const Coords& coords) const;
    vector<vector<Cell>>& getField();
    int getWidth() const;
    int getHeight() const;
    bool coordinatsInField(Coords coords_check) const;
    bool shipCoordinatesCorrect(Coords top_left, Orientation orientation, int size);
    void resizeSize(int width, int height);
    void placeShip(Ship& ship, Coords top_left, Orientation orientation);
    void attackCell(Coords attack_coords);
    void print_field() const;
    Status getCellStatus(const Coords& coords) const;
    void setCellStatus(const Coords& coords, Status new_status);
    bool isShipHereDestroyed(const Coords& coords);
    bool isShipHere(const Coords& coords);
};

// Исключения
class NoAvailableAbilitiesException : public runtime_error {
public:
    NoAvailableAbilitiesException() : runtime_error("No available abilities to apply.") {}
};

class ShipPlacementException : public runtime_error {
public:
    ShipPlacementException() : runtime_error("Invalid ship placement. Ships are too close or overlapping.") {}
};

class OutOfFieldAttackException : public runtime_error {
public:
    OutOfFieldAttackException() : runtime_error("Attack out of field bounds.") {}
};

class Ability {
public:
    virtual void apply(GameField& field, ShipManager& manager, Coords coords) = 0;
    virtual std::string getName() const = 0;
    virtual ~Ability() = default;
};

// Способность двойного урона
class DoubleDamage : public Ability {
public:
    void apply(GameField& field, ShipManager& manager, Coords coords) override {
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
    std::string getName() const override {
        return "DoubleDamage";
    }
};

// Способность сканирования
class Scanner : public Ability {
public:
    void apply(GameField& field, ShipManager& manager, Coords coords) override {
        for (int y = coords.y; y < coords.y + 2; ++y) {
            for (int x = coords.x; x < coords.x + 2; ++x) {
                if (field.coordinatsInField({x, y}) && field.getField()[y][x].ship_is_here) {
                    cout << "Ship segment found at [" << x << "][" << y << "]\n";
                }
            }
        }
    }
    std::string getName() const override {
        return "Scanner";
    }
};

// Способность случайного обстрела
class Bombardment : public Ability {
public:
    void apply(GameField& field, ShipManager& manager, Coords coords) override {
        if (!manager.getActiveShips().empty()) {
            int shipIndex = rand() % manager.getActiveShips().size();
            Ship& ship = manager.getActiveShips()[shipIndex];
            Ship::ShipSegment* segment = nullptr;
            int maxAttempts = ship.getLength();

            while (maxAttempts-- > 0) {
                int segmentIndex = rand() % ship.getLength();
                segment = const_cast<Ship::ShipSegment*>(ship.getSegmentByIndex(segmentIndex));
                if (segment != nullptr && segment->status != SegmentStatus::Destroyed) {
                    segment->ship_pointer->damageSegment(const_cast<Ship::ShipSegment*>(segment), 1);
                    break;
                }
            }
        }
    }
    std::string getName() const override {
        return "Bombardment";
    }
};

// Класс менеджера способностей
class AbilityManager {
private:
    deque<unique_ptr<Ability>> abilityQueue;
    vector<unique_ptr<Ability>> allAbilities;

public:
    AbilityManager() {
        srand(time(0));

        // Добавляем все возможные способности
        allAbilities.emplace_back(make_unique<DoubleDamage>());
        allAbilities.emplace_back(make_unique<Scanner>());
        allAbilities.emplace_back(make_unique<Bombardment>());

        // Изначально добавляем одну случайную способность
        grantRandomAbility();
    }

    // Конструктор перемещения
    AbilityManager(AbilityManager&& other) noexcept
        : abilityQueue(std::move(other.abilityQueue)),
          allAbilities(std::move(other.allAbilities)) {}

    // Оператор перемещения
    AbilityManager& operator=(AbilityManager&& other) noexcept {
        if (this != &other) {
            abilityQueue = std::move(other.abilityQueue);
            allAbilities = std::move(other.allAbilities);
        }
        return *this;
    }

    // Конструктор для инициализации объектом abilityQueue
    AbilityManager(deque<unique_ptr<Ability>>&& initialQueue) : abilityQueue(std::move(initialQueue)) {
        srand(time(0));

        // Добавляем все возможные способности
        allAbilities.emplace_back(make_unique<DoubleDamage>());
        allAbilities.emplace_back(make_unique<Scanner>());
        allAbilities.emplace_back(make_unique<Bombardment>());
    }

    void grantRandomAbility() {
        if (!allAbilities.empty()) {
            int index = rand() % allAbilities.size();
            // Создаём новую способность на основе выбранной
            if (dynamic_cast<DoubleDamage*>(allAbilities[index].get())) {
                abilityQueue.push_back(make_unique<DoubleDamage>());
            } else if (dynamic_cast<Scanner*>(allAbilities[index].get())) {
                abilityQueue.push_back(make_unique<Scanner>());
            } else if (dynamic_cast<Bombardment*>(allAbilities[index].get())) {
                abilityQueue.push_back(make_unique<Bombardment>());
            }
        }
    }

    void applyAbility(GameField& field, ShipManager& manager, Coords coords) {
        if (abilityQueue.empty()) {
            throw NoAvailableAbilitiesException();
        }

        unique_ptr<Ability> ability = move(abilityQueue.front());
        abilityQueue.pop_front();
        ability->apply(field, manager, coords);
    }

    void addAbility(unique_ptr<Ability> ability) {
        abilityQueue.push_back(move(ability));
    }

    void printAbilities() const {
        if (abilityQueue.empty()) {
            cout << "No abilities available in the queue.\n";
        } else {
            cout << "Abilities in the queue:\n";
            int index = 1;
            for (const auto& ability : abilityQueue) {
                cout << index++ << ". ";
                if (dynamic_cast<DoubleDamage*>(ability.get())) {
                    cout << "Double Damage\n";
                } else if (dynamic_cast<Scanner*>(ability.get())) {
                    cout << "Scanner\n";
                } else if (dynamic_cast<Bombardment*>(ability.get())) {
                    cout << "Bombardment\n";
                }
            }
        }
    }

    const deque<unique_ptr<Ability>>& getQueue() const {
        return abilityQueue;
    }
};

Ship::Ship() {
    length = 0;
    orientation = Orientation::Unknown;
    top_left = {-1, -1};
    max_health = 0;
    health = 0;
}

Ship::Ship(int new_size) {
    if (new_size < minimalShipLength || new_size > maximalShipLength)
        throw logic_error("Invalid ship size");

    length = new_size;
    orientation = Orientation::Unknown;
    top_left = {-1, -1};
    max_health = new_size * 2;
    health = new_size * 2;
}

Ship::Ship(const Ship& other) { // глубокое копирование
    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = other.segments;
}

Ship::Ship(Ship&& other) noexcept { // Конструктор перемещения
    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = move(other.segments);
}

Ship& Ship::operator=(const Ship& other) { // оператор присваивания
    if (this == &other) {
        return *this;
    }

    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = other.segments; // Копируем сегменты

    return *this;
}

Ship& Ship::operator=(Ship&& other) noexcept { // оператор присваивания
    if (this == &other) {
        return *this;
    }

    length = other.length;
    orientation = other.orientation;
    top_left = other.top_left;
    max_health = other.max_health;
    health = other.health;
    segments = std::move(other.segments); // Перемещаем сегменты

    return *this;
}

int Ship::getLength() const {
    return length;
}

int Ship::getMaxHealth() {
    return max_health;
}

int Ship::getCurrentHealth() {
    int recalculatedHealth = 0;
    for (const auto& segment : segments) {
        if (segment.status == SegmentStatus::Intact) {
            recalculatedHealth += 2; // Intact сегмент стоит 2 здоровья
        } else if (segment.status == SegmentStatus::Damaged) {
            recalculatedHealth += 1; // Damaged сегмент стоит 1 здоровье
        }
    }
    health = recalculatedHealth; // Обновляем текущее здоровье
    return health;
}


void Ship::setLength_and_health(int new_length) {
    if (new_length < minimalShipLength || new_length > maximalShipLength) {
        throw std::logic_error("Invalid ship length");
    }
    length = new_length;
    max_health = length * 2; // Максимальное здоровье зависит от длины
    health = max_health;     // Устанавливаем текущее здоровье равным максимуму
    // Инициализируем сегменты
    segments.resize(length, { {0, 0}, SegmentStatus::Intact, this });
}


Orientation Ship::getOrientation() const {
    return orientation;
}

void Ship::setOrientation(Orientation new_orientation) {
    orientation = new_orientation;
}


bool Ship::isAlive() const {
    for (auto& el : segments)
        if (el.status != SegmentStatus::Destroyed)
            return true;
    return false;
}

bool Ship::isUnharmed() {
    return health == max_health;
}

bool Ship::isDestroyed() {
    return health == 0;
}

void Ship::setShipCoordinates(Orientation new_orientation, Coords new_top_left) {
    orientation = new_orientation;
    top_left = new_top_left;

    // Убедимся, что сегменты корректно инициализированы
    if (segments.size() != length) {
        segments.resize(length, { {0, 0}, SegmentStatus::Intact, this });
    }

    // Обновляем координаты сегментов
    for (int i = 0; i < length; ++i) {
        if (orientation == Orientation::Horizontal) {
            segments[i].coords = {top_left.x + i, top_left.y};
        } else {
            segments[i].coords = {top_left.x, top_left.y + i};
        }
        segments[i].ship_pointer = this;
    }
}


void Ship::damageSegment(ShipSegment* segment, int damage) {
    if (segment->status == SegmentStatus::Destroyed) {
        return;
    } else if (segment->status == SegmentStatus::Damaged) {
        segment->status = SegmentStatus::Destroyed;
    } else if (segment->status == SegmentStatus::Intact) {
        segment->status = (damage >= 2) ? SegmentStatus::Destroyed : SegmentStatus::Damaged;
    }
    getCurrentHealth(); // Пересчет здоровья после нанесения урона
}


const Ship::ShipSegment* Ship::getSegmentByIndex(int index) const{ // нужен чтобы связать клетку и корабль
    return (index >= 0 && index < length) ? &segments[index] : nullptr;
}

void Ship::setSegmentByIndex(int index, const ShipSegment& segment) {
    if (index >= 0 && index < length) {
        segments[index] = segment;
        segments[index].ship_pointer = this; // Устанавливаем указатель на текущий корабль
    } else {
        throw std::out_of_range("Invalid segment index in setSegmentByIndex");
    }
}

string orientationToString(Orientation orientation) {
    switch (orientation) {
        case Orientation::Horizontal: return "Horizontal";
        case Orientation::Vertical: return "Vertical";
        default: return "Unknown";
    }
}

Coords Ship::getCoords() const {
    return top_left;
}

void Ship::setCoords(const Coords& new_coords) {
    top_left = new_coords;
}

void Ship::print_info()  const{
    cout << "length " << length << "\n";
    cout << "orientation " << orientationToString(orientation) << "\n";
    cout << "top left [x][y] " << top_left.x << " " << top_left.y << "\n";
    cout << "max health " << max_health << "\n";
    cout << "health " << health << "\n";
}

const std::vector<Ship::ShipSegment>& Ship::getSegments() const {
    return segments;
}


string segmentStatusToString(SegmentStatus status) { // Чтобы правильно вывести статус сегмента, конвертируем значение SegmentStatus в строку
    switch (status) {
        case SegmentStatus::Intact: return "Intact";
        case SegmentStatus::Damaged: return "Damaged";
        case SegmentStatus::Destroyed: return "Destroyed";
        default: return "Unknown";
    }
}

/////////////////////////////


GameField::GameField(){
    height = 0;
    width = 0;
    field;
}

GameField::GameField(int new_width, int new_height){ //horizontal // vertical
    if (new_width < minimalFieldSize || new_height < minimalFieldSize || new_width > maximalFieldSize  || new_height > maximalFieldSize){
        throw logic_error("Invalid field size");
    }
    width = new_width;
    height = new_height;
    field.resize(height, vector<Cell>(width));
    // field.resize(height);
    // for(int y=0;y<height; y++)
    // {
    //     field[y].resize(width);
    //     for(int x=0; x<width; x++)
    //     {
    //         field[y][x] = Cell();
    //     }
    // }
}

GameField::GameField(const GameField& other) {// глубокое копирование
    width = other.width;
    height = other.height;
    field = other.field;

}

GameField::GameField(GameField&& other) noexcept{ // Конструктор перемещения
    width = move(other.width);
    height = move(other.height);
    field = move(other.field);
}

//////////////////////////////////////////////////////////////// NEW /////////////////
GameField& GameField::operator=(const GameField& other) { // оператор присваивания
    if (this == &other) {
        return *this;
    }

    width = other.width;
    height = other.height;
    field = other.field; // Копируем поле
    
    return *this;
}

GameField& GameField::operator=(GameField&& other) noexcept { // оператор присваивания
    if (this == &other) {
        return *this;
    }

    width = std::move(other.width);
    height = std::move(other.height);
    field = std::move(other.field); // Перемещаем поле
    
    return *this;
}

//////////////////////////////////////////////////////////////// NEW /////////////////

// Новый метод для доступа к конкретной ячейке поля
const Cell& GameField::getCellAt(const Coords& coords) const {
    if (!coordinatsInField(coords)) {
        throw out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x];
}

Cell& GameField::getCellAt(const Coords& coords) {
    if (!coordinatsInField(coords)) {
        throw out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x];
}

vector<vector<Cell>>& GameField::getField() {
    return field;
}

int GameField::getWidth() const{
    return width;
}

int GameField::getHeight() const{
    return height;
}

bool GameField::coordinatsInField(Coords coords_check) const{
    if (coords_check.x < 0 || coords_check.x >= width || coords_check.y < 0 || coords_check.y >= height){
        return false;
    }
    else{
        return true;
    }
}

bool GameField::shipCoordinatesCorrect(Coords top_left, Orientation orientation, int size){
    Coords coords_cell = top_left;

    for (int i = 0; i < size; ++i) {
        // Если ориента ция вертикальная, проверяем смещение по y, иначе по x
        if (orientation == Orientation::Vertical) {
            coords_cell.y = top_left.y + i;
        } else {
            coords_cell.x = top_left.x + i;
        }

        // Проверка, попадают ли координаты в границы поля
        if (!coordinatsInField(coords_cell)) {
            return false; // Координаты за пределами поля
        }

        // Проверка на наличие корабля или его соседства в выбранной клетке
        if (field[coords_cell.y][coords_cell.x].ship_is_here ||
            field[coords_cell.y][coords_cell.x].ship_is_nearly) {
            return false; // В клетке уже есть корабль или рядом с ней
        }

    }
    return true; // Если все проверки пройдены, возвращаем true
}


// Установка размеров поля
void GameField::resizeSize(int new_width, int new_height) {
    width = new_width;
    height = new_height;
    field.clear(); // Очистка текущего поля
    field.resize(height, vector<Cell>(width)); // Резервируем новую память
}


void GameField::placeShip(Ship& ship, Coords top_left, Orientation orientation) {
    if (!shipCoordinatesCorrect(top_left, orientation, ship.getLength())) {
        throw out_of_range("Ship coordinates out of range");
    }

    ship.setShipCoordinates(orientation, top_left); // расстановка сегментов корабля из Ship
    
    Coords ship_cell = top_left;
    for (int i = 0; i < ship.getLength(); i++) {
        if (ship.getOrientation() == Orientation::Vertical) {
            ship_cell.y = top_left.y + i;
        } else {
            ship_cell.x = top_left.x + i;
        }
        field[ship_cell.y][ship_cell.x].ship_is_here = true; //помечаем что ячейка содержит корабль
        field[ship_cell.y][ship_cell.x].ship_segment_pointer = const_cast<Ship::ShipSegment*>(ship.getSegmentByIndex(i));  // ссылка на сигмент орабль
    }

    // значение для итерации
    int offset_x = (orientation == Orientation::Vertical) ? 0 : ship.getLength() - 1;
    int offset_y = (orientation == Orientation::Vertical) ? ship.getLength() - 1 : 0;

    for (int x = top_left.x - 1; x <= top_left.x + offset_x; x++) { 
        for (int y = top_left.y - 1; y <= top_left.y + offset_y; y++) {
            if (coordinatsInField({x, y})) { // если координата на поле
                if (!field[y][x].ship_is_here) { // и если это не корабль
                    field[y][x].ship_is_nearly = true; // то помечаем что координата рядом
                }
            }
        }
    }
}


void GameField::attackCell(Coords attack_coords){
    if (!coordinatsInField(attack_coords)) {
        // return; // Если координаты вне поля, просто выходим
        throw out_of_range("Ship coordinates out of range");
    }

    if (field[attack_coords.y][attack_coords.x].ship_is_here) { // если попали в корабль
        field[attack_coords.y][attack_coords.x].status_cell = Status::Ship; // статус клетки меняется на корабль
        field[attack_coords.y][attack_coords.x].ship_segment_pointer->ship_pointer->damageSegment(field[attack_coords.y][attack_coords.x].ship_segment_pointer, 1);
    } else {
        field[attack_coords.y][attack_coords.x].status_cell = Status::Empty;
        field[attack_coords.y][attack_coords.x].missed = true; // для вывода поля
    }

}

/// дебаг
void GameField::print_field() const{

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (field[y][x].ship_is_here == true) {
                if (field[y][x].ship_segment_pointer->status == SegmentStatus::Damaged) {
                    cout << "1";
                }
                if (field[y][x].ship_segment_pointer->status == SegmentStatus::Destroyed) {
                    cout << "0";
                } 
                if (field[y][x].ship_segment_pointer->status == SegmentStatus::Intact) {
                    cout << "2";
                }
            } 
            else {
                if (field[y][x].missed) {
                    cout << "x";
                }
                else {
                    cout << "."; // Unattacked cell
                }
            }
        }
        cout << "\n";
    }
}

Status GameField::getCellStatus(const Coords& coords) const{
    if (!coordinatsInField(coords)) {
        throw out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x].status_cell;
}

void GameField::setCellStatus(const Coords& coords, Status new_status) {
    if (!coordinatsInField(coords)) {
        throw out_of_range("Coordinates are out of field bounds");
    }
    field[coords.y][coords.x].status_cell = new_status;
}


bool GameField::isShipHereDestroyed(const Coords& coords) {
    if (!coordinatsInField(coords)) {
        throw out_of_range("Coordinates are out of field bounds");
    }
    if (field[coords.y][coords.x].ship_is_here && field[coords.y][coords.x].ship_segment_pointer) {
        return field[coords.y][coords.x].ship_segment_pointer->status == SegmentStatus::Destroyed;
    }
    return false;
}

bool GameField::isShipHere(const Coords& coords) {
    if (!coordinatsInField(coords)) {
        throw out_of_range("Coordinates are out of field bounds");
    }
    return field[coords.y][coords.x].ship_is_here;
}

///////////////////////////////////////////////

ShipManager::ShipManager(vector<int> sizes) {
    for (int size : sizes) {
        free_ships.emplace_back(size);
    }
}

int ShipManager::getAliveShipsNumber() {
    int shipNum = 0;
    for (const auto& ship : active_ships) {
        if (ship.isAlive()) {
            shipNum++;
        }
    }
    return shipNum;
}

int ShipManager::getDestroyedShipsNumber() {
    int shipNum = 0;
    for (const auto& ship : active_ships) {
        if (!(ship.isAlive())) {
            shipNum++;
        }
    }
    return shipNum;
}

int ShipManager::getShipCount() const{
    return active_ships.size();
}

int ShipManager::getFreeShipCount() {
    return free_ships.size();
}

int ShipManager::getActiveShipCount() {
    return active_ships.size();
}

Ship& ShipManager::getFreeShip(int index) {
    if (index >= 0 && index < free_ships.size()) {
        return free_ships[index]; // Возвращаем ссылку на объект
    }
    throw out_of_range("Invalid index for free ship");
}

const Ship& ShipManager::getActiveShip(int index) const{
    if (index >= 0 && index < active_ships.size()) {
        return active_ships[index]; // Возвращаем ссылку на объект
    }
    throw out_of_range("Invalid index for active ship");
}

vector<Ship>& ShipManager::getActiveShips() {
    return active_ships;
}

void ShipManager::moveShipToActive(int index) {
    if (index >= 0 && index < free_ships.size()) {
        active_ships.push_back(move(free_ships[index]));
        free_ships.erase(free_ships.begin() + index);
    } else {
        throw out_of_range("Invalid index for moving ship");
    }
}

vector<int> ShipManager::getShipSizes() {
    vector<int> sizes;
    for (const auto& ship : active_ships) {
        sizes.push_back(ship.getLength());
    }
    return sizes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FileExeption {
    std::string message;

public:
    FileExeption(std::string message = "");
    std::string what() const noexcept;
};

////////////////////////////////////

FileExeption::FileExeption(std::string message) : message(message) {}

std::string FileExeption::what() const noexcept {
    std::string output = "Error with file:" + message + "\n";
    return output;
}
/////////////////////////
class FileHandler {
private:
    string file_name;
    ifstream input_file;
    ofstream output_file;

public:
    FileHandler(const string& file_name);

    void openForRead();
    void openForWrite();
    void write(const json& info);
    void read(json& info);
    void closeRead();
    void closeWrite();
    ~FileHandler();
};

FileHandler::FileHandler(const string& file_name) : file_name(file_name) {}

void FileHandler::openForRead() {
    input_file.open(file_name);
    if (!input_file.is_open()) {
        throw FileExeption("Can't open file for reading");
    }
}

void FileHandler::openForWrite() {
    output_file.open(file_name);
    if (!output_file.is_open()) {
        throw FileExeption("Can't open file for writing.");
    }
}

void FileHandler::write(const json& info) {
    if (output_file.is_open()) {
        output_file << info.dump(4);
    } else {
        throw FileExeption("File not open for writing.");
    }
}

void FileHandler::read(json& info) {
    if (input_file.is_open()) {
        input_file >> info;
    } else {
        throw FileExeption("File not open for reading.");
    }
}

void FileHandler::closeRead() {
    if (input_file.is_open()) {
        input_file.close();
    }
}

void FileHandler::closeWrite() {
    if (output_file.is_open()) {
        output_file.close();
    }
}

FileHandler::~FileHandler() {
    closeRead();
    closeWrite();
}

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

int main() {
    try {
        Game game;
        cout << "Хотите загрузить сохраненную игру? (y/n): ";
        char answer;
        cin >> answer;
        if (answer == 'y') {
            string file_name;
            cout << "Введите имя файла загрузки: ";
            cin >> file_name;
            GameState gameState(file_name);
            game = gameState.load();
            cout << "Игра успешно загружена!\n";
            game.startGame(1);
        }
        else{
            game.startGame(0);
        }
    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << "\n";
    } catch (...) {
        cerr << "An unknown error occurred.\n";
    }
    return 0;
}


