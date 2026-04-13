#include "WaterHub.hpp"

void WaterHub::setMagistralWaterCounter(std::unique_ptr<WaterCounter> counter)
{
    magistralWaterCounter_ = std::move(counter);
}

void WaterHub::setPressureSensor(std::unique_ptr<PressureSensor> sensor)
{
    pressureSensor_ = std::move(sensor);
}

void WaterHub::addLeafWaterCounter(std::unique_ptr<WaterCounter> counter)
{
    leafWaterCounters_.push_back(std::move(counter));
}

void WaterHub::addSoilSensor(std::unique_ptr<SoilSensor> sensor)
{
    soilSensors_.push_back(std::move(sensor));
}

void WaterHub::addValve(std::unique_ptr<Valve> valve, SoilSensor *sensor)
{
    valves_.push_back(std::move(valve));
    valveToSoilSensorMap_[valves_.back().get()] = sensor;
}

void WaterHub::loop()
{
    Serial.print("Presure: ");
    Serial.println(pressureSensor_.get()->readPressure());

    Serial.print("Magistral water usage: ");
    Serial.println(magistralWaterCounter_.get()->getLittersFromLastCall());

    for (size_t i = 0; i < soilSensors_.size(); ++i)
    {
        float humidity, temperature;
        soilSensors_[i]->readData(humidity, temperature);
        Serial.print("Soil sensor ");
        Serial.print(i);
        Serial.print(" - Humidity: ");
        Serial.print(humidity);
        Serial.print("%, Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
    }

    for (size_t i = 0; i < leafWaterCounters_.size(); ++i)
    {
        Serial.print("Leaf water counter ");
        Serial.print(i);
        Serial.print(" usage: ");
        Serial.println(leafWaterCounters_[i]->getLittersFromLastCall());
    }
}