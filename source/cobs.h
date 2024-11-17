#ifndef COBS_H
#define COBS_H

#include <QByteArray>
#include <QByteArrayList>

namespace QuCLib {


class Cobs
{
public:
    explicit Cobs(uint8_t delimiter = 0);

    QByteArray encode(QByteArray data);
    QByteArray decode(QByteArray data);

    QByteArrayList streamDecode(QByteArray data);
    void clear(void);

    uint8_t delimiter() const;

private:
    QByteArray _buffer;
    uint8_t _delimiter;
};

}
#endif //  COBS_H
