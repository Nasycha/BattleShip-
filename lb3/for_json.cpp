#include "json.hpp"
#include <iostream>
using json = nlohmann::json;
/// g++ -I lb3_ver2/include lb3_ver2/for_json.cpp -o for_json
int main() {
    // Создание JSON-объекта
    json shipInfo = {
        {"length", 4},
        {"orientation", "Horizontal"},
        {"status", "Intact"}
    };

    // Вывод JSON-объекта
    std::cout << shipInfo.dump(4) << std::endl;

    // Парсинг из строки
    std::string jsonString = R"({"x": 10, "y": 15})";
    json coords = json::parse(jsonString);

    std::cout << "x: " << coords["x"] << ", y: " << coords["y"] << std::endl;

    return 0;
}
