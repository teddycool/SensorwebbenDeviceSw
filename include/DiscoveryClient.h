#ifndef DISCOVERYCLIENT_H
#define DISCOVERYCLIENT_H

#include <Arduino.h>
#include "MqttPublisher.h"

class DiscoveryClient
{
public:
    DiscoveryClient(MqttPublisher &mqttPblisher, const String &chipid);

    void sendWifitries();
    void sendBattery();
    void sendSignalStrength();
    void sendAbat();
    void sendLocalIp();
    void sendSsid();
    void sendChipid();
    void sendCalfactor();
    void sendRawWeight();
    void sendTemperature();
    void sendHumidity();
    void sendMqttTries();

private:
    MqttPublisher &mqttPublisher_;
    String chipid_;
};

#endif // DISCOVERYCLIENT_H