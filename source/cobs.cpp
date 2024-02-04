#include "cobs.h"
using namespace QuCLib;

QByteArray Cobs::encode(QByteArray data)
{
    QByteArray output;
    uint16_t nullIndex = 0;
    uint16_t i = 0;

    output.append((uint8_t)0); // COBS index byte -> will be overwritten later

    for(i = 0; i<data.size(); i++){
        output.append(data.at(i));
        if(output.at(i+1) == 0x00){
            output[nullIndex] = (i+1-nullIndex);
            nullIndex = i+1;
        }
    }

    output[nullIndex] = (i+1-nullIndex);
    output.append((uint8_t)0); // Ending 0
    return output;
}

QByteArray Cobs::decode(QByteArray data)
{
    if(data.isEmpty()) return QByteArray();

    // Remove all leading 0 bytes
    while(data.at(0) == 0){
        data.remove(0, 1);
        if(data.isEmpty()) return QByteArray();
    }

    if(data.size() < 2) return QByteArray(); // If size too short for valid frame -> empty frame / no data
    if(data.at(data.size()-1)) return QByteArray(); // If last byte is not null -> No valid data

    QByteArray output;
    uint16_t i = 0;
    uint16_t nullIndex = data.at(0);

    for(i=1; i < data.size(); i++){
        output.append(data.at(i));
        if(nullIndex == i){
            if(data.at(i) == 0) break;// End of frame

            nullIndex += output.at(i-1);
            output[i-1] = 0;
        }else if(data.at(i) == 0){ // In case a 0 is in a position where it should not be.
            return QByteArray();
        }
    }

    output = output.remove(output.size()-1, 1); // remove trailing 0
    return output;
}

QByteArrayList Cobs::streamDecode(QByteArray data)
{
    _buffer.push_back(data);

    QByteArrayList output;

    int i= 0;
    while( i < _buffer.length()){
        if(_buffer.at(i) == 0){
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
