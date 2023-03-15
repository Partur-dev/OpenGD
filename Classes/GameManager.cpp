#include "GameManager.h"

#include "GameToolbox.h"

#include <axmol.h>

GameManager* GameManager::getInstance() {
    static GameManager* g_pGM = new(std::nothrow) GameManager();
    return g_pGM;
}

bool GameManager::hasVariable(std::string name) {
    return this->m_values.contains(name);
}

void GameManager::setDefaults() {
    json defaults = {
        {"player-cube", 1},
        {"player-main-color", 0},
        {"player-secondary-color", 3},
        {"player-username", "Player"},
        {"completed-achievements", {}}
    };

    for (auto& [key, value] : defaults.items()) {
        if (!this->hasVariable(key))
            this->setVariable(key, value);
    }
    this->save();
}

void GameManager::loadFromSave() {
    auto path = ax::FileUtils::getInstance()->getWritablePath() + "/GameManager.json";

    std::ifstream file(path);
    if (file.good())
        this->m_values = json::parse(file);
    file.close();

    this->setDefaults();
}

void GameManager::save() {
    auto path = ax::FileUtils::getInstance()->getWritablePath() + "/GameManager.json";

    std::ofstream file(path);
    if (file.good())
        file << this->m_values.dump(4);
    file.close();
}