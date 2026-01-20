#pragma once
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

class WsServer {
public:
  WsServer(WebSocketsServer& ws): _ws(ws) {}
  void begin(void (*handler)(uint8_t, WStype_t, uint8_t*, size_t)) {
    _ws.begin();
    _ws.onEvent(handler);
  }
  void loop() { _ws.loop(); }

  void broadcastDoc(ArduinoJson::JsonDocument& doc) {
    String msg;
    serializeJson(doc, msg);
    _ws.broadcastTXT(msg);
  }

private:
  WebSocketsServer& _ws;
};
