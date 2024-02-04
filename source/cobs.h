#ifndef COBS_H
#define COBS_H

#include <QByteArray>
#include <QByteArrayList>

namespace QuCLib {

class Cobs
{
public:
    static QByteArray encode(QByteArray data);
    static QByteArray decode(QByteArray data);

    QByteArrayList streamDecode(QByteArray data);
    void clear(void);

private:
    QByteArray _buffer;
};

}
#endif //  COBS_H
