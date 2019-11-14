#include <inttypes.h>

#include "Ds2413.h"

#define CMD_DS2413_READ 0xF5
#define CMD_DS2413_WRITE 0x5A

Ds2413::Ds2413(OneWireBus* bus, uint64_t id)
    : _bus(bus)
    , _id(id)
{
}

int Ds2413::init()
{
    return 0;
}

int Ds2413::write(bool pioA, bool pioB)
{
    int err = 0;
    _bus->select(true);
    uint8_t cmd[3] = { CMD_DS2413_WRITE };
    uint8_t resp[1];

    cmd[1] = 0xFC | pioA | pioB << 1;
    cmd[2] = ~cmd[1];

    err = _bus->txMatch(_id, cmd, sizeof(cmd), resp, sizeof(resp));

    if (err) {
        goto read_done;
    }

    if (resp[0] != 0xAA) {
        Serial.println("Did not get 0xAA back from DS2413 write command");
        err = OneWireErrorWrite;
    }

    Serial.printf("ds2413: 0x%x\n", resp[0]);

read_done:
    _bus->select(false);
    return err;
}

int Ds2413::read(Sample* sample)
{
    int err = 0;
    _bus->select(true);
    uint8_t cmd[] = { CMD_DS2413_READ };
    uint8_t resp[1];
    err = _bus->txMatch(_id, cmd, sizeof(cmd), resp, sizeof(resp));

    if (err) {
        goto read_done;
    }

    Serial.printf("ds2413: 0x%x\n", resp[0]);

read_done:
    _bus->select(false);
    return err;
}