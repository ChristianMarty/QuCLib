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
        enum class ErrorType {
            NoError,
            FileNotOpen,
            InvalidStartCode,
            InvalidLineLength,
            InvalidChecksum,
            InvalidRecordType,
            InvalidAddressByte,
            InvalidDataByte,
            AddressRangeTooLow,
            AddressRangeTooHigh
        };

        struct FileError {
            uint32_t lineIndex;
            ErrorType error;
        };

        struct BinaryChunk {
            uint32_t offset;
            QByteArray data;
        };

        struct Range {
            uint32_t minimum;
            uint32_t maximum;
        };


        HexFileParser(void);

        bool load(QString filePath);
        bool saveToFile(QString filePath);
        void clear(void);

        // data outside of the MemorySize range will be discarded
        void setMemorySize(const Range &range);
        void setMemorySize(uint32_t addressOffset, uint32_t size);

        void setAddressGapSize(uint32_t gap);
        void setAddressAlignment(uint32_t alignment);

        QByteArray extract(uint32_t address, uint32_t size);
        void replace(uint32_t address, QByteArray data);
        void insert(const BinaryChunk &data);

        const Range &fileAddressRange(void) const;
        const Range &binaryAddressRange(void) const;
        const Range &memoryAddressRange(void) const;

        QList<BinaryChunk> binary();

        uint32_t errorCount(void) const;
        uint32_t warningCount(void) const;
        const QList<FileError> &errors(void) const;
        const QList<FileError> &warnings(void) const;

        static QString errorMessage(const FileError &error);

    private:
        enum class RecordType: uint8_t {
            Data = 0,
            EndOfFile = 1,
            ExtendedSegmentAddress = 2,
            StartSegmentAddress = 3,
            ExtendedLinearAddress = 4,
            StartLinearAddress = 5
        };

        Range _fileAddress; // address range of the input file
        Range _memorySize; // address range limit for the output
        Range _binaryAddress; // the range of the loaded data

        uint32_t _addressGapSize;
        uint32_t _addressAlignment;
        uint8_t _fillValue;

        void _parseLine(uint32_t lineIndex, QString line);
        void _combineBinaryChunks(void);

        BinaryChunk _fixChunkAddressAlignment(uint32_t offset, QByteArray data);

        uint32_t _high16BitAddress;

        QList<BinaryChunk> _binary;
        QList<FileError> _error;
        QList<FileError> _warning;

        uint8_t _calculateChecksum(QByteArray data);
};

}
//---------------------------------------------------------------------------
#endif
