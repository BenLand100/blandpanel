#include <cstring>
#include <termios.h>

#include "libindi/indicom.h"
#include "libindi/connectionplugins/connectionserial.h"

#include "config.h"
#include "indi_blandpanel.h"

// We declare an auto pointer to BLandPanel.
static std::unique_ptr<BLandPanel> snowflake(new BLandPanel());

BLandPanel::BLandPanel() : INDI::LightBoxInterface(this)
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);
}

const char *BLandPanel::getDefaultName()
{
    return "BLand Panel";
}

bool BLandPanel::initProperties()
{

    INDI::DefaultDevice::initProperties();
    
    INDI::LightBoxInterface::initProperties(LIGHT_BOX_TAB, INDI::LightBoxInterface::CAN_DIM);

    // TODO: Add any custom properties you need here.

    addAuxControls();

    setDriverInterface(LIGHTBOX_INTERFACE | AUX_INTERFACE);

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_115200);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);

    return true;
}

void BLandPanel::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);
    INDI::LightBoxInterface::ISGetProperties(dev);
}

bool BLandPanel::updateProperties()
{
    if (!INDI::DefaultDevice::updateProperties())
    {
        return false;
    }

    if (!INDI::LightBoxInterface::updateProperties())
    {
        return false;
    }

    if (isConnected())
    {
        // TODO: Call define* for any custom properties only visible when connected.
    }
    else
    {
        // TODO: Call deleteProperty for any custom properties only visible when connected.
    }

    return true;
}

bool BLandPanel::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Number properties.
    }

    if (INDI::LightBoxInterface::processNumber(dev, name, values, names, n))
    {
        return true;
    }

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool BLandPanel::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Switch properties.
    }

    if (INDI::LightBoxInterface::processSwitch(dev, name, states, names, n))
    {
        return true;
    }

    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool BLandPanel::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    {
        // TODO: Check to see if this is for any of my custom Text properties.
    }

    if (INDI::LightBoxInterface::processText(dev, name, texts, names, n))
    {
        return true;
    }

    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool BLandPanel::ISSnoopDevice(XMLEle *root)
{

    INDI::LightBoxInterface::snoop(root);

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool BLandPanel::saveConfigItems(FILE *fp)
{
    INDI::LightBoxInterface::saveConfigItems(fp);

    // TODO: Call IUSaveConfig* for any custom properties I want to save.

    return INDI::DefaultDevice::saveConfigItems(fp);
}

bool BLandPanel::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        return true;
    }

    PortFD = serialConnection->getPortFD();

    return true;
}

#define MAXCMDBUF 1024

std::string BLandPanel::sendCommand(const std::string &cmd)
{
    int nbytes_read = 0, nbytes_written = 0, tty_rc = 0;
    char res[MAXCMDBUF] = {0};
    LOGF_DEBUG("BLPanel CMD %s", cmd.c_str());

    if (!isSimulation())
    {
        tcflush(PortFD, TCIOFLUSH);
        if ((tty_rc = tty_write_string(PortFD, cmd.c_str(), &nbytes_written)) != TTY_OK)
        {
            tty_error_msg(tty_rc, res, MAXCMDBUF);
            LOGF_ERROR("Serial write error: %s", res);
            return "FAULT";
        }
    }

    if (isSimulation())
    {
        strncpy(res, "OK\n", MAXCMDBUF);
        nbytes_read = 3;
    }
    else
    {
        if ((tty_rc = tty_read_section(PortFD, res, '\n', 5, &nbytes_read)) != TTY_OK)
        {
            tty_error_msg(tty_rc, res, MAXCMDBUF);
            LOGF_ERROR("Serial read error: %s", res);
            return "FAULT";
        }
    }

    res[nbytes_read - 2] = '\0';
    LOGF_DEBUG("BLPanel RSP %s", res);

    return std::string(res);
}


bool BLandPanel::SetLightBoxBrightness(uint16_t value)
{
    std::string res = sendCommand("BRIGHT " + std::to_string(value/255.0) + "\n");

    return res == "OK";
}

bool BLandPanel::EnableLightBox(bool enable)
{
    if (enable)
    {
        std::string res1 = sendCommand("STATE CLOSED\n");
        std::string res2 = sendCommand("ON\n");
        return res1 == "OK" and res2 == "OK";
    }
    else
    {
        std::string res1 = sendCommand("OFF\n");
        std::string res2 = sendCommand("STATE OPENED\n");
        return res1 == "OK" and res2 == "OK";
    }
}
