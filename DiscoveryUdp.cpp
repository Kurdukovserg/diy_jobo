#include "DiscoveryUdp.h"
#if defined(ESP32)
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <string.h>

void DiscoveryUdp::begin(uint16_t port, const char* deviceName, uint16_t httpPort, uint16_t wsPort, const char* fw) {
  _port = port;
  _deviceName = deviceName;
  _httpPort = httpPort;
  _wsPort = wsPort;
  _fw = fw;

  _udp.begin(_port);
  _lastMs = 0;
}

bool DiscoveryUdp::getActiveIpAndBroadcast(IPAddress& ip, IPAddress& bc) {
  // Prefer STA if connected; otherwise use AP.
  if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
    IPAddress mask = WiFi.subnetMask();
    bc = IPAddress(
      ip[0] | (~mask[0] & 0xFF),
      ip[1] | (~mask[1] & 0xFF),
      ip[2] | (~mask[2] & 0xFF),
      ip[3] | (~mask[3] & 0xFF)
    );
    return true;
  }

  ip = WiFi.softAPIP();
  if (ip[0] == 0) return false;
  bc = IPAddress(ip[0], ip[1], ip[2], 255);
  return true;
}

void DiscoveryUdp::tick() {
  handleIncoming();

  uint32_t now = millis();
  if (now - _lastMs >= _intervalMs) {
    _lastMs = now;
    sendAnnounce();
  }
}

void DiscoveryUdp::handleIncoming() {
  int psize = _udp.parsePacket();
  if (psize <= 0) return;

  char buf[64];
  int len = _udp.read(buf, sizeof(buf)-1);
  if (len <= 0) return;
  buf[len] = 0;

  if (strcmp(buf, "DISCOVER_DIYJOBO") != 0) return;

  IPAddress ip, bc;
  if (!getActiveIpAndBroadcast(ip, bc)) return;

  char reply[200];
  snprintf(
    reply, sizeof(reply),
    "DIYJOBO;name=%s;ip=%u.%u.%u.%u;http=%u;ws=%u;fw=%s",
    _deviceName,
    ip[0], ip[1], ip[2], ip[3],
    _httpPort, _wsPort,
    _fw
  );

  _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
  _udp.write((const uint8_t*)reply, strlen(reply));
  _udp.endPacket();
}

void DiscoveryUdp::sendAnnounce() {
  IPAddress ip, bc;
  if (!getActiveIpAndBroadcast(ip, bc)) return;

  char msg[200];
  snprintf(
    msg, sizeof(msg),
    "DIYJOBO;name=%s;ip=%u.%u.%u.%u;http=%u;ws=%u;fw=%s",
    _deviceName,
    ip[0], ip[1], ip[2], ip[3],
    _httpPort, _wsPort,
    _fw
  );

  _udp.beginPacket(bc, _port);
  _udp.write((const uint8_t*)msg, strlen(msg));
  _udp.endPacket();
}
