#include <catch2/catch.hpp>
#include "../hexFileParser.h"

TEST_CASE( "HEX File Parser", "[hexFileParser]" ) {

    SECTION("Valid file") {
        HexFileParser parser;
        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_1.hex");

        REQUIRE(parser.errorCount() == 0);

        REQUIRE(parser.addressFileMinimum() == 0x00010000);
        REQUIRE(parser.addressFileMaximum() == 0x0001125B);
        REQUIRE(parser.addressBinaryMinimum() == 0x00010000);
        REQUIRE(parser.addressBinaryMaximum() == 0x0001125B);
    }

    SECTION("Invalide checksum file") {
        HexFileParser parser;
        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_invalid_checksum.hex");

        REQUIRE(parser.errorCount() == 1);
        REQUIRE(parser.errors().at(0).error == parser.errorType::errorType_invalidChecksum);
    }

    SECTION("Address range split and gap fill") {
        HexFileParser parser;
        parser.setAddressGapSize(16);

        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_with_gaps.hex");

        REQUIRE(parser.errorCount() == 0);
        REQUIRE(parser.binary().count() == 2);

        QByteArray pass0;
        pass0.append(QByteArray("\xDD\xCC\xBB\xAA\x00\x00\xFF\xEE\x01\x00\x44\x4D\x58\x00\x00\x00", 16));
        pass0.append(QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 16));
        pass0.append(QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
        QByteArray pass1;
        pass1.append(QByteArray("\x98\x0B\x01\x20\xB1\x05\x01\x00\xAD\x05\x01\x00\xAD\x05\x01\x00", 16));
        pass1.append(QByteArray("\xAD\x05\x01\x00\xAD\x05\x01\x00\xAD\x05\x01\x00\x00\x00\x00\x00", 16));

        REQUIRE(parser.binary().at(0).data == pass0);
        REQUIRE(parser.binary().at(1).data == pass1);

        REQUIRE(parser.binary().at(0).offset == 0x00010000);
        REQUIRE(parser.binary().at(1).offset == 0x00010064);
    }

    SECTION("Address alignment") {
        HexFileParser parser;
        parser.setAddressGapSize(16);
        parser.setAddressAlignment(16);

        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_with_gaps.hex");

        REQUIRE(parser.errorCount() == 0);
        REQUIRE(parser.binary().count() == 2);

        QByteArray pass0;
        pass0.append(QByteArray("\xDD\xCC\xBB\xAA\x00\x00\xFF\xEE\x01\x00\x44\x4D\x58\x00\x00\x00", 16));
        pass0.append(QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 16));
        pass0.append(QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
        QByteArray pass1;
        pass1.append(QByteArray("\xFF\xFF\xFF\xFF", 4));
        pass1.append(QByteArray("\x98\x0B\x01\x20\xB1\x05\x01\x00\xAD\x05\x01\x00\xAD\x05\x01\x00", 16));
        pass1.append(QByteArray("\xAD\x05\x01\x00\xAD\x05\x01\x00\xAD\x05\x01\x00\x00\x00\x00\x00", 16));
        pass1.append(QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 12));

        REQUIRE(parser.binary().at(0).data == pass0);
        REQUIRE(parser.binary().at(1).data == pass1);

        REQUIRE(parser.binary().at(0).offset == 0x00010000);
        REQUIRE(parser.binary().at(1).offset == 0x00010060);
    }

    SECTION("Data outside addess range (Too low)") {
        HexFileParser parser;
        parser.setMemorySize(0x00010001, 0xFFFF);

        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_with_gaps.hex");

        REQUIRE(parser.errorCount() == 1);
        REQUIRE(parser.errors().at(0).error == parser.errorType::errorType_addressRangeTooLow);
    }

    SECTION("Data outside addess range (Too high)") {
        HexFileParser parser;
        parser.setMemorySize(0x00010000, 0x83);

        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_with_gaps.hex");

        REQUIRE(parser.errorCount() == 1);
        REQUIRE(parser.errors().at(0).error == parser.errorType::errorType_addressRangeTooHigh);
    }

    SECTION("Data size") {
        HexFileParser parser;
        parser.setAddressGapSize(32);
        parser.setAddressAlignment(16);

        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_with_gaps.hex");

        REQUIRE(parser.errorCount() == 0);

        REQUIRE(parser.addressFileMinimum() == 0x00010000);
        REQUIRE(parser.addressFileMaximum() == 0x00010083);
        REQUIRE(parser.addressBinaryMinimum() == 0x00010000);
        REQUIRE(parser.addressBinaryMaximum() == 0x0001008F);
    }

    SECTION("Extract data") {
        HexFileParser parser;
        parser.setAddressGapSize(32);
        parser.setAddressAlignment(16);

        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_with_gaps.hex");

        REQUIRE(parser.errorCount() == 0);

        QByteArray pass1 = QByteArray("\x98\x0B\x01\x20\xB1\x05\x01\x00\xAD\x05\x01\x00\xAD\x05\x01\x00\xAD\x05", 18);

        REQUIRE(parser.extract(0x00010064,18) == pass1);
    }

    SECTION("Replace data") {
        HexFileParser parser;
        parser.setAddressGapSize(32);
        parser.setAddressAlignment(16);

        parser.load("C:\\Users\\Christian\\Raumsteuerung\\pc\\QCodebase\\_test\\hexFileParser\\test_file_with_gaps.hex");

        REQUIRE(parser.errorCount() == 0);

        parser.replace(0x00010074, QByteArray("\xAB\xCD\xEF\xAA", 4));

        QByteArray pass0;
        pass0.append(QByteArray("\xDD\xCC\xBB\xAA\x00\x00\xFF\xEE\x01\x00\x44\x4D\x58\x00\x00\x00", 16));
        pass0.append(QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 16));
        pass0.append(QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
        QByteArray pass1;
        pass1.append(QByteArray("\xFF\xFF\xFF\xFF", 4));
        pass1.append(QByteArray("\x98\x0B\x01\x20\xB1\x05\x01\x00\xAD\x05\x01\x00\xAD\x05\x01\x00", 16));
        pass1.append(QByteArray("\xAB\xCD\xEF\xAA\xAD\x05\x01\x00\xAD\x05\x01\x00\x00\x00\x00\x00", 16));
        pass1.append(QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 12));

        REQUIRE(parser.binary().at(0).data == pass0);
        REQUIRE(parser.binary().at(1).data == pass1);
    }
}
