#ifndef CRC_H
#define CRC_H

#include <QByteArray>

namespace QuCLib {

class Crc
{

public:
    static uint16_t crc16(QByteArray data);
    static uint16_t crc16_addByte(uint16_t CRC_value, uint8_t data);
    static uint32_t crc32(QByteArray data);
private:
};

};
#endif //  CRC_H
