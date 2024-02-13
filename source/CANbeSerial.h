#ifndef CANBESERIAL_H
#define CANBESERIAL_H

#include <QObject>
#include "cobs.h"

struct CanBusFrame
{
    bool isValide = false;

    uint32_t identifier;
    bool extended;
    bool fd;
    bool rtr;
    QByteArray data;
};


class CANbeSerial : public QObject
{
    Q_OBJECT
public:

    enum Baudrate:uint8_t{
        Baud10k,
        Baud20k,
        Baud50k,
        Baud100k,
        Baud125k,
        Baud250k,
        Baud500k,
        Baud1M,
        Baud2M,
        Baud5M,
        Baud10M
    };


    explicit CANbeSerial(QObject *parent = nullptr);

    void write(CanBusFrame &frame);

    void setEnabled(bool enable);
    void setBaudrate(Baudrate baudrate);
    void setDataBaudrate(Baudrate baudrate);

    void setTxPaddingEnable(bool enabled, char value = 0x00);

    enum PayloadId:uint8_t {
        data = 0x00,
        errorFrame = 0x01,
        transmitAcknowledgment = 0x02,

        protocolVersion = 0x08,
        protocolVersionRequest = 0x88,

        configurationState = 0x09,
        configurationStateRequest = 0x89,
        configurationStateCommand = 0xC9,

        deviceInformation = 0x0A,
        deviceInformationRequest = 0x8A
    };

public slots:
    void receive(QByteArray data);

signals:
    void error();
    void writeReady(QByteArray data);
    void readReady(CanBusFrame frame);

private:
    void _parseMessage(QByteArray data);
    void _buildMessage(PayloadId payloadId, QByteArray data);
    void _sendConfiguration(void);

    void _decodeDataframe(QByteArray data);

    CanBusFrame _decodeFrame(QByteArray data);
    QByteArray _encodeFrame(CanBusFrame &frame);

    int8_t _dlcToLength(uint8_t dlc);
    int8_t _lengthToDlc(uint8_t length);

    bool _enabled = false;
    Baudrate _baudrate = Baudrate::Baud125k;
    Baudrate _fdBaudrate = Baudrate::Baud125k;
    bool _automaticRetransmission = false;
    bool _silentMode = false;

    bool _txPaddingEnabled = false;
    char _txPaddingValue = 0x00;

    int _rxErrorCounter = 0;

    QuCLib::Cobs _cobsDecoder;

    const static inline uint8_t _dlc[16] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

};

#endif // CANBESERIAL_H
