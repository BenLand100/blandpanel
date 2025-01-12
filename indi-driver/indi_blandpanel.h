#pragma once

#include "libindi/defaultdevice.h"
#include "libindi/indilightboxinterface.h"

namespace Connection
{
    class Serial;
}

class BLandPanel : public INDI::DefaultDevice, public INDI::LightBoxInterface
{
public:
    const char *LIGHT_BOX_TAB = "Light Box";
    
    BLandPanel();
    virtual ~BLandPanel() = default;

    virtual const char *getDefaultName() override;

    virtual bool initProperties() override;
    virtual bool updateProperties() override;

    virtual void ISGetProperties(const char *dev) override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
    virtual bool ISSnoopDevice(XMLEle *root) override;

protected:
    virtual bool saveConfigItems(FILE *fp) override;

    virtual bool SetLightBoxBrightness(uint16_t value) override;
    virtual bool EnableLightBox(bool enable) override;

private: // serial connection
    bool Handshake();
    std::string sendCommand(const std::string &cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
};
