#ifndef DISCOVERYCLIENT_H
#define DISCOVERYCLIENT_H

#include <Arduino.h>
#include "Publisher.h"

class DiscoveryClient
{
public:
    DiscoveryClient(Publisher &publisher, const String &chipid);

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
    Publisher &publisher_;
    String chipid_;
};

#endif // DISCOVERYCLIENT_H