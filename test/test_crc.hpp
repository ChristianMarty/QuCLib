#include <catch2/catch.hpp>
#include "../source/crc.h"

using namespace QuCLib;

TEST_CASE( "Test crc16", "[crc16]" ) {

    SECTION("Test 1") {
        QByteArray input = QByteArray("\x01\x02\x03\x04", 4);
        REQUIRE(Crc::crc16(input) ==  0x89C3 );
    }

    SECTION("Test 2") {
        QByteArray input = QByteArray("\xFF\xFF\x00\x00\x00\x00\x00\x00", 8);
        REQUIRE(Crc::crc16(input) ==  0x00 );
    }
}
