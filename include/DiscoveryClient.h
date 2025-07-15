#ifndef DISCOVERYCLIENT_H
#define DISCOVERYCLIENT_H

#include <Arduino.h>

class Publisher {
public:
    virtual void publish(const String& topic, const String& message, bool retain) = 0;
};

class DiscoveryClient {
public:
    DiscoveryClient(Publisher& publisher, const String& chipid);

    void sendWifitries();
    void sendBattery();
    void sendSignalStrength();
    void sendAbat();
    void sendLocalIp();
    void sendSsid();
    void sendChipid();
    void sendCalfactor();
    void sendRawWeight();

private:
    Publisher& publisher_;
    String chipid_;
};

#endif // DISCOVERYCLIENT_H