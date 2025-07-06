#include "hexFileParser.h"
#include <QTextStream>
using namespace QuCLib;

HexFileParser::HexFileParser(void)
{
    clear();
}

void HexFileParser::clear()
{
    _memorySize.minimum = 0;
    _memorySize.maximum = 0xFFFFFFFF;
    _addressGapSize = 16;
    _addressAlignment = 1;
    _fillValue = 0xFF;
    _binary.clear();
}

void HexFileParser::setMemorySize(const Range &range)
{
    _memorySize = range;
}

void HexFileParser::setMemorySize(uint32_t addressOffset, uint32_t size)
{
    _memorySize.minimum = addressOffset;
    _memorySize.maximum = addressOffset+size;
}

void HexFileParser::setAddressGapSize(uint32_t gap)
{
    _addressGapSize = gap;
}

void HexFileParser::setAddressAlignment(uint32_t alignment)
{
    _addressAlignment = alignment;
}

uint32_t HexFileParser::errorCount() const
{
    return _error.count();
}

uint32_t HexFileParser::warningCount() const
{
    return _warning.count();
}

const QList<HexFileParser::FileError> &HexFileParser::errors() const
{
    return _error;
}

const QList<HexFileParser::FileError> &HexFileParser::warnings() const
{
    return _warning;
}

QString HexFileParser::errorMessage(const FileError &error)
{
    switch(error.error){
        case ErrorType::NoError: return "No Error";
        case ErrorType::FileNotOpen: return "Could not open file";
        case ErrorType::InvalidStartCode: return "Invalid start code";
        case ErrorType::InvalidLineLength: return "Byte count doesn't match line length";
        case ErrorType::InvalidChecksum: return "Invalid checksum";
        case ErrorType::InvalidRecordType: return "Unsupported record type";
        case ErrorType::InvalidAddressByte: return "Invalid address";
        case ErrorType::InvalidDataByte: return "Invalid data";
        case ErrorType::AddressRangeTooLow: return "Address out of range (lower minimum address)";
        case ErrorType::AddressRangeTooHigh: return "Address out of range (higher maximum address)";
    }

    return "Unknwon Error";
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

QList<HexFileParser::BinaryChunk> HexFileParser::binary()
{
    return _binary;
}

QByteArray HexFileParser::extract(uint32_t address, uint32_t size)
{
    QByteArray output;
    for(const BinaryChunk &data: _binary){
        if(data.offset <= address){
            if(data.data.length() - (address-data.offset) >= size){
                output = data.data.mid(address-data.offset, size);
                break;
            }
        }
    }
    return output;
}

void HexFileParser::replace(uint32_t address, QByteArray data)
{
    for(uint32_t i = 0; i< _binary.length(); i++)
    {
        if(_binary.at(i).offset <= address)
        {
            BinaryChunk chunk = _binary.at(i);
            uint32_t index = address - chunk.offset;
            if(chunk.data.length() - index >= data.length())
            {
                chunk.data.replace(index, data.size(), data);
                _binary[i] = chunk;
            }
        }
    }
}

void HexFileParser::insert(const BinaryChunk &data)
{
    // TODO: Check if chunk is already inside binary
    // TODO: Check inside address range
    _binary.append(data);
    std::sort(_binary.begin(), _binary.end(), [](const BinaryChunk &a, const BinaryChunk &b){ return a.offset < b.offset; } );
}

const HexFileParser::Range &HexFileParser::fileAddressRange() const
{
    return _fileAddress;
}

const HexFileParser::Range &HexFileParser::binaryAddressRange() const
{
    return _binaryAddress;
}

const HexFileParser::Range &HexFileParser::memoryAddressRange() const
{
    return _memorySize;
}

bool HexFileParser::load(QString filePath)
{
    _error.clear();
    _binary.clear();

    _high16BitAddress = 0;
    _fileAddress.minimum = 0xFFFFFFFF;
    _fileAddress.maximum = 0;

    QString line;
    QFile hexFile(filePath);
    hexFile.open(QIODevice::ReadOnly);

    if(hexFile.isOpen()){
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
    }else{
        _error.append(FileError{0,ErrorType::FileNotOpen});
        return false;
    }
}

bool HexFileParser::saveToFile(QString filePath)
{
    QFile hexFile(filePath);
    hexFile.open(QIODevice::WriteOnly);

    if(!hexFile.isOpen()) return false;

    QTextStream out(&hexFile);

    uint16_t offsetPrefix = 0;
    for(const BinaryChunk &data: _binary)
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
                line.append((uint8_t)RecordType::ExtendedSegmentAddress);
                line.append((uint8_t)(offsetPrefix>>8));
                line.append((uint8_t)offsetPrefix);
                line.append((uint8_t)_calculateChecksum(line));

                QString lineSting = ":"+line.toHex();
                out << lineSting.toUpper() << Qt::endl;
            }
        }

        uint32_t i;
        for(i= 0; i< data.data.length()-16; i+=16)
        {
            QByteArray lineData = data.data.mid(i,16);
            uint16_t lineAddress = (uint16_t)data.offset+i;
            QByteArray line;
            line.append((uint8_t)16); // Lenght
            line.append((uint8_t)(lineAddress>>8));
            line.append((uint8_t)lineAddress);
            line.append((uint8_t)RecordType::Data);
            line.append(lineData);
            line.append((uint8_t)_calculateChecksum(line));

            QString lineSting = ":"+line.toHex();
            out << lineSting.toUpper() << Qt::endl;
        }

        uint16_t lineAddress = (uint16_t)data.offset+i;
        QByteArray line;
        line.append((uint8_t)(data.data.length()-i)); // Lenght
        line.append((uint8_t)(lineAddress>>8));
        line.append((uint8_t)lineAddress);
        line.append((uint8_t)RecordType::Data);
        line.append(data.data.mid(i));
        line.append((uint8_t)_calculateChecksum(line));

        QString lineSting = ":"+line.toHex();
        out << lineSting.toUpper() << Qt::endl;
    }
    out << ":00000001FF" << Qt::endl; // Add end of file

    out.flush();
    hexFile.close();

    return true;
}

void HexFileParser::_parseLine(uint32_t lineIndex, QString line)
{
    if(line.at(0) != ':')
    {
        _error.append(FileError{lineIndex,ErrorType::InvalidStartCode});
        return;
    }

    bool ok;
    uint8_t lineByteCount = line.mid(1,2).toUInt(&ok,16);
    if(!ok || lineByteCount*2+11 != line.length()){
        _error.append(FileError{lineIndex,ErrorType::InvalidLineLength});
        return;
    }

    RecordType lineRecordType = (RecordType)line.mid(7 ,2).toUInt(&ok,16);
    if(!ok){
        _error.append(FileError{lineIndex,ErrorType::InvalidRecordType});
        return;
    }

    uint8_t lineAddressHighByte = line.mid(3,2).toUInt(&ok,16);
    if(!ok){
        _error.append(FileError{lineIndex,ErrorType::InvalidAddressByte});
        return;
    }

    uint8_t lineAddressLowByte = line.mid(5,2).toUInt(&ok,16);
    if(!ok){
        _error.append(FileError{lineIndex,ErrorType::InvalidAddressByte});
        return;
    }
    uint16_t lineAddress = ((uint16_t)lineAddressHighByte<<8) | lineAddressLowByte;

    QByteArray lineData;
    uint8_t checksum = lineByteCount + (uint8_t)lineRecordType + lineAddressHighByte+lineAddressLowByte;
    for(uint32_t i = 0; i < lineByteCount; i++)
    {
        uint8_t byte = line.mid(i*2+9 ,2).toUInt(&ok,16);
        if(!ok){
            _error.append(FileError{lineIndex,ErrorType::InvalidDataByte});
            return;
        }
        lineData.append(byte);

        checksum+= byte;
    }
    checksum += line.mid(line.length()-2 ,2).toUInt(&ok,16);
    if(!ok || checksum != 0){
        _error.append(FileError{lineIndex,ErrorType::InvalidChecksum});
        return;
    }

    switch (lineRecordType) {
        case RecordType::Data:{
            uint32_t lineStartAddress = _high16BitAddress+lineAddress;
            uint32_t lineEndAddress = _high16BitAddress+lineAddress + lineByteCount-1;

            if(_fileAddress.minimum > lineStartAddress) _fileAddress.minimum = lineStartAddress;
            if(_fileAddress.maximum < lineEndAddress) _fileAddress.maximum = lineEndAddress;

            if(_memorySize.minimum > lineStartAddress) {
                _warning.append(FileError{lineIndex,ErrorType::AddressRangeTooLow});
                return;
            }

            if(_memorySize.maximum < lineEndAddress) {
                _warning.append(FileError{lineIndex,ErrorType::AddressRangeTooHigh});
                return;
            }

            _binary.append(BinaryChunk{_high16BitAddress+lineAddress, lineData});
            break;
        }

        case RecordType::EndOfFile:
            return;

        case RecordType::ExtendedLinearAddress:
            _high16BitAddress = (line.mid(9,4).toUInt(&ok,16))<<16;
            if(!ok){
                _error.append(FileError{lineIndex,ErrorType::InvalidAddressByte});
                return;
            }
            break;

        case RecordType::ExtendedSegmentAddress:
            _high16BitAddress = (line.mid(9,4).toUInt(&ok,16))<<4;
            if(!ok){
                _error.append(FileError{lineIndex,ErrorType::InvalidAddressByte});
                return;
            }
            break;

        case RecordType::StartLinearAddress:
        case RecordType::StartSegmentAddress:
            break;

        default:
            _error.append(FileError{lineIndex,ErrorType::InvalidRecordType});
            break;
    }
}

void HexFileParser::_combineBinaryChunks()
{
    std::sort(_binary.begin(), _binary.end(), [](const BinaryChunk &a, const BinaryChunk &b){ return a.offset < b.offset; } );

    QList<BinaryChunk> combinedBinary;

    QByteArray nextData;
    uint32_t chunkOffse = 0xFFFFFFFF;
    uint32_t nextOffset = _binary.first().offset;

    for(const BinaryChunk &data: _binary)
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
               nextOffset += gapFillSize + data.data.length();
            }
            else // Start a new chunk
            {
                if(nextData.length()){
                    combinedBinary.append(_fixChunkAddressAlignment(chunkOffse, nextData));
                }
                nextData.clear();
                chunkOffse = 0xFFFFFFFF;
            }
        }
        else
        {
            nextOffset = data.offset + data.data.length();
        }

        if(chunkOffse == 0xFFFFFFFF)
        {
            nextOffset = data.offset + data.data.length();
            chunkOffse = data.offset;
        }
        nextData.append(data.data);
    }
    combinedBinary.append(_fixChunkAddressAlignment(chunkOffse, nextData));
    _binary = combinedBinary;

    _binaryAddress.minimum =_binary.first().offset;
    _binaryAddress.maximum =_binary.last().offset+_binary.last().data.length()-1;
}

HexFileParser::BinaryChunk HexFileParser::_fixChunkAddressAlignment(uint32_t offset, QByteArray data)
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
    uint32_t chunkEndAddress = offset+data.length();
    if(chunkEndAddress % _addressAlignment != 0)
    {
        uint32_t fillCount = _addressAlignment - (data.length() % _addressAlignment);
        for(uint32_t i = 0; i< fillCount; i++)
        {
            data.append(_fillValue);
        }
    }

    return BinaryChunk{alignedOffset, data};
}
