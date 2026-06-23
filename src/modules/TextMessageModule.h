#pragma once
#include "Observer.h"
#include "SinglePortModule.h"

/**
 * Text message handling for meshtastic - draws on the OLED display the most recent received message
 */
class TextMessageModule : public SinglePortModule, public Observable<const meshtastic_MeshPacket *>
{
  public:
    static const uint8_t DS18B20_MAX_SENSORS = 8;
    static const uint8_t DS18B20_ADDRESS_HEX_LENGTH = 16;

    /** Constructor
     * name is for debugging output
     */
    TextMessageModule() : SinglePortModule("text", meshtastic_PortNum_TEXT_MESSAGE_APP) {}

  protected:
    /** Called to handle a particular incoming message

    @return ProcessMessage::STOP if you've guaranteed you've handled this message and no other handlers should be considered for
    it
    */
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;
    virtual bool wantPacket(const meshtastic_MeshPacket *p) override;

  private:
    bool maybeHandleTemperatureCommand(const meshtastic_MeshPacket &mp);
    char lastDs18b20List[DS18B20_MAX_SENSORS][DS18B20_ADDRESS_HEX_LENGTH + 1] = {};
    uint8_t lastDs18b20ListCount = 0;
    uint32_t lastTemperatureCommandResponse = 0;
};

extern TextMessageModule *textMessageModule;
