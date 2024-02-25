#include "hexFileParser.h"
#include <QTextStream>
using namespace QuCLib;

HexFileParser::HexFileParser(void)
{
    clear();
}

void HexFileParser::clear()
{
    _minAddress = 0;
    _maxAddress = 0xFFFFFFFF;
    _addressGapSize = 16;
    _addressAlignment = 1;
    _fillValue = 0xFF;
    _binary.clear();
}

void HexFileParser::setMemorySize(uint32_t addressOffset, uint32_t size)
{
    _minAddress = addressOffset;
    _maxAddress = addressOffset+size;
}

void HexFileParser::setAddressGapSize(uint32_t gap)
{
    _addressGapSize = gap;
}

void HexFileParser::setAddressAlignment(uint32_t alignment)
{
    _addressAlignment = alignment;
}

uint32_t HexFileParser::addressFileMinimum()
{
    return _minFileAddress;
}

uint32_t HexFileParser::addressFileMaximum()
{
    return _maxFileAddress;
}

uint32_t HexFileParser::addressBinaryMinimum()
{
    return _minBinaryAddress;
}

uint32_t HexFileParser::addressBinaryMaximum()
{
    return _maxBinaryAddress;
}

uint32_t HexFileParser::errorCount()
{
    return _error.count();
}

QList<HexFileParser::fileError> HexFileParser::errors()
{
    return _error;
}

QString HexFileParser::errorMessage(fileError error)
{
    const QString errorMessages[8] =
    {
        "No Error",
        "Could not open file",
        "Invalid start code",
        "Byte count doesn't match line length",
        "Invalid checksum",
        "Unsupported record type",
        "Address out of range (lower minimum address)",
        "Address out of range (higher maximum address)"
    };

    return errorMessages[error.error];
}

void HexFileParser::saveToFile(QString filePath)
{
    QFile hexFile(filePath);
    hexFile.open(QIODevice::WriteOnly);

    if(hexFile.isOpen())
    {
        QTextStream out(&hexFile);

        uint16_t offsetPrefix = 0;
        foreach (const binaryChunk &data, _binary)
        {
            if(data.offset  > 0xFFFF)
            {
                uint16_t offset = (data.offset>>4)&0xFF00;
                if(offset != offsetPrefix)
                {
                    offsetPrefix = offset;
                    QByteArray line;
                    line.append((uint8_t)0x02);
                    line.append((uint8_t)0x00);
                    line.append((uint8_t)0x00);
                    line.append(recordType::recordType_extendedSegmentAddress);
                    line.append((uint8_t)(offsetPrefix>>8));
                    line.append((uint8_t)offsetPrefix);
                    line.append((uint8_t)_calculateChecksum(line));

                    QString lineSting = ":"+line.toHex();
                    out << lineSting.toUpper() << Qt::endl;
                }
            }

            uint32_t i = 0;
            for(i; i< data.data.count()-16; i+=16)
            {
                QByteArray lineData = data.data.mid(i,16);
                uint16_t lineAddress = (uint16_t)data.offset+i;
                QByteArray line;
                line.append((uint8_t)16); // Lenght
                line.append((uint8_t)(lineAddress>>8));
                line.append((uint8_t)lineAddress);
                line.append(recordType::recordType_data);
                line.append(lineData);
                line.append((uint8_t)_calculateChecksum(line));

                QString lineSting = ":"+line.toHex();
                out << lineSting.toUpper() << Qt::endl;
            }

            uint16_t lineAddress = (uint16_t)data.offset+i;
            QByteArray line;
            line.append((uint8_t)(data.data.count()-i)); // Lenght
            line.append((uint8_t)(lineAddress>>8));
            line.append((uint8_t)lineAddress);
            line.append(recordType::recordType_data);
            line.append(data.data.mid(i));
            line.append((uint8_t)_calculateChecksum(line));

            QString lineSting = ":"+line.toHex();
            out << lineSting.toUpper() << Qt::endl;
        }
        out << ":00000001FF" << Qt::endl; // Add end of file

        out.flush();
        hexFile.close();
    }
}

uint8_t HexFileParser::_calculateChecksum(QByteArray data)
{
    uint8_t output = 0;
    foreach(const uint8_t &byte, data){
        output+= byte;
    }
    output = (0xFF-output)+1;
    return output;
}

QList<HexFileParser::binaryChunk> HexFileParser::binary()
{
    return _binary;
}

QByteArray HexFileParser::extract(uint32_t address, uint32_t size)
{
    QByteArray output;
    foreach (const binaryChunk &data, _binary)
    {
        if(data.offset <= address)
        {
            if(data.data.count() - (address-data.offset) >= size)
            {
                output = data.data.mid(address-data.offset, size);
                break;
            }
        }
    }
    return output;
}

void HexFileParser::replace(uint32_t address, QByteArray data)
{
    for(uint32_t i = 0; i< _binary.count(); i++)
    {
        if(_binary.at(i).offset <= address)
        {
            binaryChunk chunk = _binary.at(i);
            uint32_t index = address - chunk.offset;
            if(chunk.data.count() - index >= data.count())
            {
                chunk.data.replace(index, data.size(), data);
                _binary[i] = chunk;
            }
        }
    }
}

void HexFileParser::insert(binaryChunk data)
{
    // TODO: Check if chunk is already inside binary
    // TODO: Check inside address range
    _binary.append(data);
    std::sort(_binary.begin(), _binary.end(), [](const binaryChunk &a, const binaryChunk &b){ return a.offset < b.offset; } );
}

void HexFileParser::_parseLine(uint32_t lineIndex, QString line)
{
    if(line.at(0) != ':')
    {
        _error.append(fileError{lineIndex,errorType::errorType_invalidStartCode});
        return;
    }

    uint8_t lineByteCount = line.mid(1,2).toUInt(nullptr,16);
    if(lineByteCount*2+11 != line.count())
    {
        _error.append(fileError{lineIndex,errorType::errorType_invalidLineLength});
        return;
    }

    uint8_t lineRecordType = line.mid(7 ,2).toUInt(nullptr,16);
    uint8_t lineAddressHighByte = line.mid(3,2).toUInt(nullptr,16);
    uint8_t lineAddressLowByte = line.mid(5,2).toUInt(nullptr,16);
    uint16_t lineAddress = ((uint16_t)lineAddressHighByte<<8) | lineAddressLowByte;

    QByteArray lineData;
    uint8_t checksum = lineByteCount+lineRecordType+lineAddressHighByte+lineAddressLowByte;
    for(uint32_t i = 0; i < lineByteCount; i++)
    {
        uint8_t byte = line.mid(i*2+9 ,2).toUInt(nullptr,16);
        lineData.append(byte);

        checksum+= byte;
    }
    checksum += line.mid(line.count()-2 ,2).toUInt(nullptr,16);
    if(checksum != 0)
    {
        _error.append(fileError{lineIndex,errorType::errorType_invalidChecksum});
        return;
    }

    switch (lineRecordType) {
        case recordType::recordType_data:

            if(_minAddress > (_high16BitAddress+lineAddress)) {
                _error.append(fileError{lineIndex,errorType::errorType_addressRangeTooLow});
            }

            if(_maxAddress < (_high16BitAddress+lineAddress + lineByteCount)) {
                _error.append(fileError{lineIndex,errorType::errorType_addressRangeTooHigh});
            }

            _binary.append(binaryChunk{_high16BitAddress+lineAddress, lineData});
            break;

        case recordType::recordType_endOfFile:
            return;

        case recordType::recordType_extendedLinearAddress:
            _high16BitAddress = (line.mid(9,4).toUInt(nullptr,16))<<16;
            break;

        case recordType::recordType_extendedSegmentAddress:
            _high16BitAddress = (line.mid(9,4).toUInt(nullptr,16))<<4;
            break;

        case recordType::recordType_startLinearAddress:
        case recordType::recordType_startSegmentAddress:
            break;

        default:
            _error.append(fileError{lineIndex,errorType::errorType_invalidRecordType});
            break;
    }
}

void HexFileParser::_combineBinaryChunks()
{
    std::sort(_binary.begin(), _binary.end(), [](const binaryChunk &a, const binaryChunk &b){ return a.offset < b.offset; } );

    _minFileAddress =_binary.first().offset;
    _maxFileAddress =_binary.last().offset+_binary.last().data.count()-1;

    QList<binaryChunk> combinedBinary;

    QByteArray nextData;
    uint32_t chunkOffse = 0xFFFFFFFF;
    uint32_t nextOffset = _minFileAddress;

    foreach (const binaryChunk &data, _binary)
    {
        if(nextOffset != data.offset) // If data in the address space is missing
        {
            if(data.offset <= nextOffset + _addressGapSize) // if gap is smaller as specefied gap size -> fill in data
            {
               uint32_t gapFillSize = data.offset - nextOffset;
               for(uint32_t i = 0; i< gapFillSize; i++)
               {
                   nextData.append(_fillValue);
               }
               nextOffset += gapFillSize + data.data.count();
            }
            else // Start a new chunk
            {
                if(nextData.count()){
                    combinedBinary.append(_fixChunkAddressAlignment(chunkOffse, nextData));
                }
                nextData.clear();
                chunkOffse = 0xFFFFFFFF;
            }
        }
        else
        {
            nextOffset = data.offset + data.data.count();
        }

        if(chunkOffse == 0xFFFFFFFF)
        {
            nextOffset = data.offset + data.data.count();
            chunkOffse = data.offset;
        }
        nextData.append(data.data);
    }
    combinedBinary.append(_fixChunkAddressAlignment(chunkOffse, nextData));
    _binary = combinedBinary;

    _minBinaryAddress =_binary.first().offset;
    _maxBinaryAddress =_binary.last().offset+_binary.last().data.count()-1;
}

HexFileParser::binaryChunk HexFileParser::_fixChunkAddressAlignment(uint32_t offset, QByteArray data)
{
    // Fill beginning of data chunk
    uint32_t alignedOffset = offset;
    if(offset % _addressAlignment != 0)
    {
        uint32_t fillCount = offset - (offset - (offset % _addressAlignment));
        alignedOffset -= fillCount;
        for(uint32_t i = 0; i< fillCount; i++)
        {
            data.prepend(_fillValue);
        }
    }

     // Fill end of data chunk
    uint32_t chunkEndAddress = offset+data.count();
    if(chunkEndAddress % _addressAlignment != 0)
    {
        uint32_t fillCount = _addressAlignment - (data.count() % _addressAlignment);
        for(uint32_t i = 0; i< fillCount; i++)
        {
            data.append(_fillValue);
        }
    }

    return binaryChunk{alignedOffset, data};
}

//---------------------------------------------------------------------------
bool HexFileParser::load(QString filePath)
{
    _error.clear();
    _binary.clear();

    _high16BitAddress = 0;
    _minFileAddress = 0xFFFFFFFF;
    _maxFileAddress = 0;

    QString line;
    QFile hexFile(filePath);
    hexFile.open(QIODevice::ReadOnly);

    if(hexFile.isOpen())
	{
        uint32_t lineIndex = 0;
        while(!hexFile.atEnd())
		{
           lineIndex++;
           line = hexFile.readLine();
           _parseLine(lineIndex, line.trimmed());
        }
        hexFile.close();

        if(_error.count()  > 0) return false;

        _combineBinaryChunks();
        return true;
    }
    else
    {
        _error.append(fileError{0,errorType::errorType_fileNotOpen});
        return false;
    }
}
//---------------------------------------------------------------------------
