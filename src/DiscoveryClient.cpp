#include "DiscoveryClient.h"
#include "DiscoveryMsg.h"

DiscoveryClient::DiscoveryClient(Publisher& publisher, const String& chipid)
    : publisher_(publisher), chipid_(chipid)
{}

void DiscoveryClient::sendWifitries() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "wifitries", "");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "wifitries");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendBattery() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "voltage", "battery", "V");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "battery");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendSignalStrength() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "signal_strength", "rssi", "dB");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "rssi");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendAbat() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "abat", "none");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "abat");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendLocalIp() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "localip", "none");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "localip");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendSsid() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "ssid", "none");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "ssid");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendChipid() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "chipid", "none");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "chipid");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendCalfactor() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "calfactor", "none");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "calfactor");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendRawWeight() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "rweight", "none");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "rweight");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendTemperature() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "temperature", "temperature", "°C");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "temperature");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendHumidity() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "humidity", "humidity", "%");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "humidity");
    publisher_.publish(topic, msg, true);
}

void DiscoveryClient::sendMqttTries() {
    String msg = DiscoveryMsg::createDiscoveryMsg(chipid_, "none", "mqtttries", "none");
    String topic = DiscoveryMsg::createDiscoveryMsgTopic(chipid_, "mqtttries");
    publisher_.publish(topic, msg, true);
}