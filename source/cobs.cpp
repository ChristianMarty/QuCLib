#include "cobs.h"
using namespace QuCLib;

Cobs::Cobs(uint8_t delimiter)
    :_delimiter{delimiter}
{

}

QByteArray Cobs::encode(QByteArray data)
{
    QByteArray output;
    uint16_t delimiterIndex = 0;
    uint16_t i = 0;

    output.append((uint8_t)0); // COBS index byte -> will be overwritten later

    for(i = 0; i<data.size(); i++){
        output.append(data.at(i));
        if(output.at(i+1) == _delimiter){
            output[delimiterIndex] = (i+1-delimiterIndex);
            delimiterIndex = i+1;
        }
    }

    output[delimiterIndex] = (i+1-delimiterIndex);
    output.append(_delimiter); // Ending with delimiter
    return output;
}

QByteArray Cobs::decode(QByteArray data)
{
    if(data.isEmpty()) return QByteArray();

    // Remove all leading delimiter bytes
    while(data.at(0) == _delimiter){
        data.remove(0, 1);
        if(data.isEmpty()) return QByteArray();
    }

    if(data.size() < 2) return QByteArray(); // If size too short for valid frame -> empty frame / no data
    if(data.at(data.size()-1) != _delimiter) return QByteArray(); // If last byte is not delimiter -> No valid data

    QByteArray output;
    uint16_t i = 0;
    uint16_t delimiterIndex = data.at(0);

    for(i=1; i < data.size(); i++){
        output.append(data.at(i));
        if(delimiterIndex == i){
            if(data.at(i) == _delimiter) break;// End of frame

            delimiterIndex += output.at(i-1);
            output[i-1] = _delimiter;
        }else if(data.at(i) == _delimiter){ // In case a delimiter is in a position where it should not be.
            return QByteArray();
        }
    }

    output = output.remove(output.size()-1, 1); // remove trailing delimiter
    return output;
}

QByteArrayList Cobs::streamDecode(QByteArray data)
{
    _buffer.push_back(data);

    QByteArrayList output;

    int i= 0;
    while( i < _buffer.length()){
        if(_buffer.at(i) == _delimiter){
            QByteArray temp = decode(_buffer.mid(0,i+1));
            if(temp.length()) output.append(temp);
            _buffer.remove(0,i+1);
            i = 0;
        }else{
            i++;
        }
    }

    return output;
}

void Cobs::clear()
{
    _buffer.clear();
}

uint8_t Cobs::delimiter() const
{
    return _delimiter;
}
