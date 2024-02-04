//---------------------------------------------------------------------------

#ifndef HexFileParser_H
#define HexFileParser_H

#include <QByteArray>
#include <QString>
#include <QFile>

namespace QuCLib {

//---------------------------------------------------------------------------
class HexFileParser
{
	public:
        enum errorType {
            errorType_noError,
            errorType_fileNotOpen,
            errorType_invalidStartCode,
            errorType_invalidLineLength,
            errorType_invalidChecksum,
            errorType_invalidRecordType,
            errorType_addressRangeTooLow,
            errorType_addressRangeTooHigh
        };

        struct fileError {
            uint32_t lineIndex;
            errorType error;
        };

        struct binaryChunk {
            uint32_t offset;
            QByteArray data;
        };

        enum recordType {
            recordType_data = 0,
            recordType_endOfFile = 1,
            recordType_extendedSegmentAddress = 2,
            recordType_startSegmentAddress = 3,
            recordType_extendedLinearAddress = 4,
            recordType_startLinearAddress = 5
        };

        HexFileParser(void);

        void clear(void);

        void setMemorySize(uint32_t addressOffset, uint32_t size);

        bool load(QString filePath);

        void setAddressGapSize(uint32_t gap);
        void setAddressAlignment(uint32_t alignment);

        QByteArray extract(uint32_t address, uint32_t size);
        void replace(uint32_t address, QByteArray data);
        void insert(binaryChunk data);

        uint32_t addressFileMinimum(void);
        uint32_t addressFileMaximum(void);

        uint32_t addressBinaryMinimum(void);
        uint32_t addressBinaryMaximum(void);

        QList<binaryChunk> binary();

        uint32_t errorCount(void);
        QList<fileError> errors(void);
        static QString errorMessage(fileError error);

        void saveToFile(QString filePath);

    private:
        uint32_t _minFileAddress;
        uint32_t _maxFileAddress;

        uint32_t _minBinaryAddress;
        uint32_t _maxBinaryAddress;

        uint32_t _minAddress;
        uint32_t _maxAddress;

        uint32_t _addressGapSize;
        uint32_t _addressAlignment;
        uint8_t _fillValue;

        void _parseLine(uint32_t lineIndex, QString line);
        void _combineBinaryChunks(void);

        binaryChunk _fixChunkAddressAlignment(uint32_t offset, QByteArray data);

        uint32_t _high16BitAddress;

        QList<binaryChunk> _binary;
        QList<fileError> _error;

        uint8_t _calculateChecksum(QByteArray data);
};

}
//---------------------------------------------------------------------------
#endif
