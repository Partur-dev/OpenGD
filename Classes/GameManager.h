#pragma once

#include <axmol.h>
#include <fstream>
#include <string>

#include "GameToolbox.h"

#include <nlohmann/json.hpp>
using namespace nlohmann;

class GameManager : public ax::Node {
private:
    json m_values;
public:
    static GameManager* getInstance();

    void setDefaults();

    /// @brief Set a game variable which can be saved. Usage: GameManager::getInstance()->getVariable<var type>(name)
    /// @tparam T Type of the variable
    /// @param name The name of the variable
    /// @return The variable from the json dict
    template <typename T>
    T getVariable(std::string name) {
        return this->m_values.contains(name) ? static_cast<T>(this->m_values[name]) : static_cast<T>(NULL);
    }

    /// @brief Get a game variable. Usage: GameManager::getInstance()->setVariable(name, value)
    /// @param name The name of the variable
    /// @param value The value to assign
    template <typename T>
    void setVariable(std::string name, T value) {
        this->m_values[name] = value;
    }

    void save();
    void loadFromSave();
    bool hasVariable(std::string name);
};