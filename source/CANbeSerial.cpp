#include "CANbeSerial.h"
#include "cobs.h"
#include "crc.h"

CANbeSerial::CANbeSerial(QObject *parent)
    : QObject{parent}
{

}

void CANbeSerial::write(CanBusFrame &frame)
{
    _buildMessage(PayloadId::data, _encodeFrame(frame));
}

void CANbeSerial::setEnabled(bool enable)
{
    _enabled = enable;
    _sendConfiguration();
}

void CANbeSerial::setBaudrate(Baudrate baudrate)
{
    _baudrate = baudrate;
}

void CANbeSerial::setDataBaudrate(Baudrate baudrate)
{
    _fdBaudrate = baudrate;
}

void CANbeSerial::setTxPaddingEnable(bool enabled, char value)
{
    _txPaddingEnabled = enabled;
    _txPaddingValue = value;
}

void CANbeSerial::receive(QByteArray data)
{
    QByteArrayList frames = _cobsDecoder.streamDecode(data);

    QByteArrayList::iterator i;
    for (i = frames.begin(); i != frames.end(); ++i)
    {
        _parseMessage(*i);
    }
}

void CANbeSerial::_parseMessage(QByteArray data)
{
    if(QuCLib::Crc::crc16(data) != 0)
    {
        _rxErrorCounter++;
        emit error();
        return;
    }

    PayloadId payloadId = (PayloadId)data.at(0);
    switch(payloadId){
        case  PayloadId::data:
            emit readReady(_decodeFrame(data.mid(1,data.size()-3)));
            break;

        case  PayloadId::errorFrame: break;
        case  PayloadId::transmitAcknowledgment: break;

        case  PayloadId::protocolVersion: break;
        case  PayloadId::protocolVersionRequest: break;

        case  PayloadId::configurationState: break;
        case  PayloadId::configurationStateRequest: break;
        case  PayloadId::configurationStateCommand: break;

        case  PayloadId::deviceInformation: break;
        case  PayloadId::deviceInformationRequest: break;
    }

}

void CANbeSerial::_buildMessage(PayloadId payloadId, QByteArray data)
{
    data.prepend(payloadId);
    uint16_t crc = QuCLib::Crc::crc16(data);
    data.append((uint8_t)(crc>>8));
    data.append((uint8_t)crc&0xFF);

    emit writeReady(QuCLib::Cobs::encode(data).prepend((char)0));
}

void CANbeSerial::_sendConfiguration()
{
    uint8_t bus = 0;
    if(_enabled) bus |= 0x01;
    if(_automaticRetransmission) bus |= 0x02;
    if(_silentMode) bus |= 0x04;

    QByteArray data;
    data.append(_baudrate);
    data.append(_fdBaudrate);
    data.append(bus);

    _buildMessage(PayloadId::configurationStateCommand, data);
}

CanBusFrame CANbeSerial::_decodeFrame(QByteArray data)
{
    if(data.size() < 5) return CanBusFrame();

    CanBusFrame frame;
    uint32_t identifier = 0;
    identifier |= static_cast<uint32_t>((uint8_t)data.at(0)<<24) & 0xFF000000;
    identifier |= static_cast<uint32_t>((uint8_t)data.at(1)<<16) & 0x00FF0000;
    identifier |= static_cast<uint32_t>((uint8_t)data.at(2)<<8) & 0x0000FF00;
    identifier |= static_cast<uint32_t>((uint8_t)data.at(3)) & 0x000000FF;

    uint8_t flags = data.at(4);

    frame.identifier = identifier;
    frame.extended = (flags&0x01);
    frame.fd = (flags&0x02);
    frame.rtr = (flags&0x04);
    int8_t lenght = _dlcToLength((flags>>4)&0x0F);
    if(lenght<0) return CanBusFrame();
    if(data.size() < lenght+5) return CanBusFrame();
    frame.data = data.mid(5);
    frame.isValide = true;

    return frame;
}

QByteArray CANbeSerial::_encodeFrame(CanBusFrame &frame)
{
    int8_t dlc = _lengthToDlc(frame.data.size());
    if(dlc < 0) return QByteArray(); // TODO: flag error

    uint8_t flags = 0;
    if(frame.extended) flags |= 0x01;
    if(frame.fd) flags |= 0x02;
    if(frame.rtr) flags |= 0x04;
    flags |= (dlc<<4)&0xF0;

    uint8_t paddingLength = 0;
    if(_dlcToLength(dlc) != frame.data.size()){ // in case the datasize not matching dlc
        if(_txPaddingEnabled) paddingLength = _dlcToLength(dlc) - frame.data.size();
        else  return QByteArray(); // TODO: flag error
    }

    QByteArray data;
    data.append(static_cast<char>(frame.identifier>>24 & 0xFF));
    data.append(static_cast<char>(frame.identifier>>16 & 0xFF));
    data.append(static_cast<char>(frame.identifier>>8 & 0xFF));
    data.append(static_cast<char>(frame.identifier & 0xFF));
    data.append(flags);
    data.append(frame.data);

    while(paddingLength){
        data.append(_txPaddingValue);
        paddingLength--;
    }

    return data;
}

int8_t CANbeSerial::_dlcToLength(uint8_t dlc)
{
    if(dlc >= sizeof(_dlc)) return -1;
    else return _dlc[dlc];
}

int8_t CANbeSerial::_lengthToDlc(uint8_t length)
{
    if(length > 64) return -1;
    if(length <= 8) return length;

    for(uint8_t i = 8; i < sizeof(_dlc); i++)
    {
        if(_dlc[i] >= length) return i;
    }

    return -1;
}

