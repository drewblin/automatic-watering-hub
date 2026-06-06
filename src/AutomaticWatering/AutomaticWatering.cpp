#include "AutomaticWatering.hpp"

#include <Arduino.h>

AutomaticWatering::AutomaticWatering(const WaterHub &waterHub)
    : waterHub_(waterHub),
      leafWaterCounterReadingRevisions_(waterHub.getLeafWaterCounters().size()),
      soilSensorReadingRevisions_(waterHub.getSoilSensors().size())
{
}

void AutomaticWatering::loop()
{
    processMagistralWaterCounter();
    processLeafWaterCounters();
    processPressureSensor();
    processSoilSensors();
}

void AutomaticWatering::processMagistralWaterCounter()
{
    const WaterCounter *counter = waterHub_.getMagistralWaterCounter();
    if (counter->getReadingRevision() == magistralWaterCounterReadingRevision_)
    {
        return;
    }

    magistralWaterCounterReadingRevision_ = counter->getReadingRevision();

    Serial.print("Magistral water usage total: ");
    Serial.println(counter->getTotalLiters());
}

void AutomaticWatering::processLeafWaterCounters()
{
    const auto &counters = waterHub_.getLeafWaterCounters();
    for (size_t i = 0; i < counters.size(); ++i)
    {
        if (counters[i]->getReadingRevision() == leafWaterCounterReadingRevisions_[i])
        {
            continue;
        }

        leafWaterCounterReadingRevisions_[i] = counters[i]->getReadingRevision();

        Serial.print("Leaf water counter ");
        Serial.print(i);
        Serial.print(" usage total: ");
        Serial.println(counters[i]->getTotalLiters());
    }
}

void AutomaticWatering::processPressureSensor()
{
    const PressureSensor *sensor = waterHub_.getPressureSensor();
    if (sensor->getReadingRevision() == pressureSensorReadingRevision_)
    {
        return;
    }

    pressureSensorReadingRevision_ = sensor->getReadingRevision();

    Serial.print("Pressure: ");
    Serial.println(sensor->getLastReadPressure());
}

void AutomaticWatering::processSoilSensors()
{
    const auto &sensors = waterHub_.getSoilSensors();
    for (size_t i = 0; i < sensors.size(); ++i)
    {
        if (sensors[i]->getReadingRevision() == soilSensorReadingRevisions_[i])
        {
            continue;
        }

        soilSensorReadingRevisions_[i] = sensors[i]->getReadingRevision();

        Serial.print("Soil sensor ");
        Serial.print(i);
        Serial.print(" - Humidity: ");
        Serial.print(sensors[i]->getLastReadHumidity());
        Serial.print("%, Temperature: ");
        Serial.print(sensors[i]->getLastReadTemperature());
        Serial.println("\xC2\xB0"
                       "C");
    }
}
