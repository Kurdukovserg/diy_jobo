#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>

class DiscoveryUdp {
public:
  void begin(uint16_t port, const char* deviceName, uint16_t httpPort, uint16_t wsPort, const char* fw);
  void tick();
  void setIntervalMs(uint32_t ms) { _intervalMs = ms; }

private:
  WiFiUDP _udp;
  uint16_t _port = 0;

  const char* _deviceName = "DIY-JOBO";
  const char* _fw = "0.0";
  uint16_t _httpPort = 80;
  uint16_t _wsPort   = 81;

  uint32_t _intervalMs = 1000;
  uint32_t _lastMs = 0;

  void sendAnnounce();
  void handleIncoming();

  bool getActiveIpAndBroadcast(IPAddress& ip, IPAddress& bc);
};
