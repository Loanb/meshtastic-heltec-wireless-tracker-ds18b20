#include "TextMessageModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "buzz.h"
#include "configuration.h"
#include "graphics/Screen.h"

#if defined(HELTEC_TRACKER_V1_1) && __has_include(<DallasTemperature.h>) && __has_include(<OneWire.h>)
#include <DallasTemperature.h>
#include <OneWire.h>
#define HAS_DS18B20_TEXT_COMMAND
#endif

TextMessageModule *textMessageModule;

#ifdef HAS_DS18B20_TEXT_COMMAND
static const uint8_t DS18B20_TEXT_COMMAND_PIN = 5;
static const uint32_t DS18B20_TEXT_COMMAND_THROTTLE_MS = 10 * 1000;
static const char DS18B20_TEXT_COMMAND[] = "/temp";

static bool payloadContainsCommand(const meshtastic_Data &data, const char *command)
{
    const size_t commandLength = strlen(command);
    if (data.payload.size < commandLength) {
        return false;
    }

    for (size_t i = 0; i <= data.payload.size - commandLength; i++) {
        if (memcmp(data.payload.bytes + i, command, commandLength) == 0) {
            return true;
        }
    }
    return false;
}
#endif

ProcessMessage TextMessageModule::handleReceived(const meshtastic_MeshPacket &mp)
{
#if defined(DEBUG_PORT) && !defined(DEBUG_MUTE)
    auto &p = mp.decoded;
    LOG_INFO("Received text msg from=0x%0x, id=0x%x, msg=%.*s", mp.from, mp.id, p.payload.size, p.payload.bytes);
#endif

    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    devicestate.rx_text_message = mp;
    devicestate.has_rx_text_message = true;

    // Only trigger screen wake if configuration allows it
    if (shouldWakeOnReceivedMessage()) {
        powerFSM.trigger(EVENT_RECEIVED_MSG);
    }
    notifyObservers(&mp);
    maybeHandleTemperatureCommand(mp);

    return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

bool TextMessageModule::wantPacket(const meshtastic_MeshPacket *p)
{
    return MeshService::isTextPayload(p);
}

bool TextMessageModule::maybeHandleTemperatureCommand(const meshtastic_MeshPacket &mp)
{
#ifdef HAS_DS18B20_TEXT_COMMAND
    if (mp.decoded.portnum != meshtastic_PortNum_TEXT_MESSAGE_APP) {
        return false;
    }

    if (!payloadContainsCommand(mp.decoded, DS18B20_TEXT_COMMAND)) {
        return false;
    }

    const uint32_t now = millis();
    if (lastTemperatureCommandResponse != 0 &&
        now - lastTemperatureCommandResponse < DS18B20_TEXT_COMMAND_THROTTLE_MS) {
        LOG_WARN("Ignore DS18B20 temperature command due to throttling");
        return true;
    }

    OneWire oneWire(DS18B20_TEXT_COMMAND_PIN);
    DallasTemperature ds18b20(&oneWire);
    ds18b20.begin();
    ds18b20.setWaitForConversion(true);

    char response[meshtastic_Constants_DATA_PAYLOAD_LEN] = {};
    const uint8_t sensorCount = ds18b20.getDeviceCount();
    if (sensorCount == 0) {
        snprintf(response, sizeof(response), "DS18B20 not detected");
    } else {
        ds18b20.requestTemperatures();
        size_t used = 0;
        for (uint8_t i = 0; i < sensorCount && used < sizeof(response); i++) {
            const float tempC = ds18b20.getTempCByIndex(i);
            int written;
            if (tempC == DEVICE_DISCONNECTED_C) {
                if (sensorCount == 1) {
                    written = snprintf(response + used, sizeof(response) - used, "Temperature: invalid");
                } else {
                    written = snprintf(response + used, sizeof(response) - used, "%sTemperature %u: invalid",
                                       used ? "\n" : "", i + 1);
                }
            } else if (sensorCount == 1) {
                written = snprintf(response + used, sizeof(response) - used, "Temperature: %.1f °C", (double)tempC);
            } else {
                written = snprintf(response + used, sizeof(response) - used, "%sTemperature %u: %.1f °C", used ? "\n" : "",
                                   i + 1, (double)tempC);
            }
            if (written < 0 || (size_t)written >= sizeof(response) - used) {
                break;
            }
            used += written;
        }
    }

    meshtastic_MeshPacket *p = allocDataPacket();
    p->to = isBroadcast(mp.to) ? NODENUM_BROADCAST : getFrom(&mp);
    p->channel = mp.channel;
    p->priority = meshtastic_MeshPacket_Priority_BACKGROUND;
    p->want_ack = false;
    p->decoded.want_response = false;
    p->decoded.payload.size = strlen(response);
    memcpy(p->decoded.payload.bytes, response, p->decoded.payload.size);

    LOG_INFO("Send DS18B20 temperature command response: %s", response);
    service->sendToMesh(p, RX_SRC_LOCAL, false, false);
    lastTemperatureCommandResponse = now;
    return true;
#else
    return false;
#endif
}
