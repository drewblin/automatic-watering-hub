#include "Settings.hpp"

Settings::Settings()
{
    String payload = R"json(
{
  "valveSettings": [
    {
      "pin": 12,
      "name": "Front lawn",
      "soilSensorSlaveAddress": 2
    },
    {
      "pin": 13,
      "name": "Backyard",
      "soilSensorSlaveAddress": 3
    }
  ],
  "pressureSensor": {
    "slaveAddress": 1
  },
  "magistralWaterCounterSetting": {
    "pin": 27,
    "name": "Main counter",
    "litersPerTick": 0.5
  },
  "leafWaterCounterSettings": [
    {
      "pin": 14,
      "name": "Zone 1 counter",
      "litersPerTick": 0.3
    },
    {
      "pin": 15,
      "name": "Zone 2 counter",
      "litersPerTick": 0.25
    }
  ],
  "soilSensorSettings": [
    {
      "slaveAddress": 2,
      "name": "Sensor 1"
    },
    {
      "slaveAddress": 3,
      "name": "Sensor 2"
    }
  ]
}
)json";

    DeserializationError err = deserializeJson(doc_, payload);
    if (err)
    {
        ESP_LOGE("Settings", "Failed to parse JSON: %s", err.c_str());
        return;
    }
}

std::vector<ValveSetting> Settings::getValveSetting()
{
    JsonArray jsonValveSettings = doc_["valveSettings"].as<JsonArray>();

    std::vector<ValveSetting> vector;
    vector.reserve(jsonValveSettings.size());

    for (JsonVariant valveSetting : jsonValveSettings)
    {
        ValveSetting setting(
            (uint8_t)valveSetting["pin"],
            std::string(valveSetting["name"]),
            (uint8_t)valveSetting["soilSensorSlaveAddress"]);

        vector.push_back(setting);
    }

    return vector;
}

PressureSensorSetting Settings::getPressureSensorSetting()
{
    return PressureSensorSetting(
        (uint8_t)doc_["pressureSensor"]["slaveAddress"]);
}

WaterCounterSetting Settings::getMagistralWaterCounterSetting()
{
    JsonVariant jsonSettings = doc_["magistralWaterCounterSetting"];

    return WaterCounterSetting(
        (uint8_t)jsonSettings["pin"],
        std::string(jsonSettings["name"]),
        (float)jsonSettings["litersPerTick"]);
}

std::vector<WaterCounterSetting> Settings::getLeafWaterCounterSetting()
{
    JsonArray jsonSettings = doc_["leafWaterCounterSettings"].as<JsonArray>();

    std::vector<WaterCounterSetting> vector;
    vector.reserve(jsonSettings.size());

    for (JsonVariant counterSetting : jsonSettings)
    {
        WaterCounterSetting setting(
            (uint8_t)counterSetting["pin"],
            std::string(counterSetting["name"]),
            (float)counterSetting["litersPerTick"]);

        vector.push_back(setting);
    }

    return vector;
}

std::vector<SoilSensorSetting> Settings::getSoilSensorSetting()
{
    JsonArray jsonSettings = doc_["soilSensorSettings"].as<JsonArray>();

    std::vector<SoilSensorSetting> vector;
    vector.reserve(jsonSettings.size());

    for (JsonVariant soilSensorSetting : jsonSettings)
    {
        SoilSensorSetting setting(
            (uint8_t)soilSensorSetting["slaveAddress"],
            std::string(soilSensorSetting["name"]));

        vector.push_back(setting);
    }

    return vector;
}
