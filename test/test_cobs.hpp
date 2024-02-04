//#ifdef TEST_RUN

#include <catch2/catch.hpp>
#include "../cobs.h"

using namespace QuCLib;

TEST_CASE( "Test Cobs_encode", "[Cobs_encode]" ) {

    SECTION( "Encode valide frame" ) {
        QByteArray input = QByteArray("\x01\x00\x17\x43", 4);
        QByteArray pass = QByteArray("\x02\x01\x03\x17\x43\x00", 6);

        QByteArray output = Cobs::encode(input);
        
        REQUIRE(output == pass);
    }

  SECTION( "Encode empty frame" ) {
        QByteArray input = QByteArray("", 0);
        QByteArray pass = QByteArray("\x01\x00", 2);

        QByteArray output = Cobs::encode(input);

        REQUIRE(output == pass);
    }

   SECTION( "Encode only one 0 frame" ) {
        QByteArray input = QByteArray("\x00", 1);
        QByteArray pass = QByteArray("\x01\x01\x00", 3);

        QByteArray output = Cobs::encode(input);

        REQUIRE(output == pass);
    }

   SECTION( "Encode only 0 frame" ) {
        QByteArray input = QByteArray("\x00\x00\x00\x00\x00", 5);
        QByteArray pass = QByteArray("\x01\x01\x01\x01\x01\x01\x00", 7);

        QByteArray output = Cobs::encode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Encode short frame without 0" ) {
        QByteArray input = QByteArray("\x77", 1);
        QByteArray pass = QByteArray("\x02\x77\x00", 3);

        QByteArray output = Cobs::encode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Encode long frame without 0"  ) {
        QByteArray input = QByteArray("\x77\x66\x55\x44", 4);
        QByteArray pass = QByteArray("\x05\x77\x66\x55\x44\x00", 6);

        QByteArray output = Cobs::encode(input);

        REQUIRE(output == pass);
    }
}


TEST_CASE( "Test Cobs_decode", "[Cobs_decode]" ) {

    SECTION("Decode valide frame") {
        QByteArray input = QByteArray("\x02\x01\x03\x17\x23\x00", 6);
        QByteArray pass = QByteArray("\x01\x00\x17\x23",4);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION("Decode frame with leading 0") {
        QByteArray input = QByteArray("\x00\x00\x00\x02\x01\x03\x17\x23\x00", 9);
        QByteArray pass = QByteArray("\x01\x00\x17\x23",4);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

   SECTION( "Decode empty frame" ) {
       QByteArray input = QByteArray("\x01\x00", 2);
       QByteArray pass = QByteArray("",0);

       QByteArray output = Cobs::decode(input);

       REQUIRE(output == pass);
    }

    SECTION( "Decode frame with one 0" ) {
        QByteArray input = QByteArray("\x01\x01\x00", 3);
        QByteArray pass = QByteArray("\x00",1);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

     SECTION( "Decode frame with one byte" ) {
        QByteArray input = QByteArray("\x02\x77\x00", 3);
        QByteArray pass = QByteArray("\x77",1);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode frame without 0" ) {
        QByteArray input = QByteArray("\x05\x77\x66\x55\x44\x00", 6);
        QByteArray pass = QByteArray("\x77\x66\x55\x44",4);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode invalide frame without end 0" ) {
        QByteArray input = QByteArray("\x05\x77\x66\x55\x44\x09", 6);
        QByteArray pass = QByteArray("",0);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode no data" ) {
        QByteArray input = QByteArray("", 0);
        QByteArray pass = QByteArray("",0);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode no data but 0" ) {
        QByteArray input = QByteArray("\x00", 1);
        QByteArray pass = QByteArray("",0);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode no data but 0" ) {
        QByteArray input = QByteArray("\x00\x00\x00", 3);
        QByteArray pass = QByteArray("",0);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode data with multiple frames" ) {
        QByteArray input = QByteArray("\x03\x77\x66\x00\x44\x00", 6);
        QByteArray pass = QByteArray("\x77\x66",2);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode data with multiple frames" ) {
        QByteArray input = QByteArray("\x03\x77\x66\x02\x99\x00\x01\x00", 8);
        QByteArray pass = QByteArray("\x77\x66\x00\x99",4);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }

    SECTION( "Decode data with framing error" ) {
        QByteArray input = QByteArray("\x03\x77\x66\x02\x00\x44\x00", 7);
        QByteArray pass = QByteArray("",0);

        QByteArray output = Cobs::decode(input);

        REQUIRE(output == pass);
    }
}


TEST_CASE( "Test Cobs stream decoder", "[Cobs_decodeStream]" ) {

    Cobs Cobs;

    SECTION("Decode valide frame") {
        QByteArray input = QByteArray("\x02\x01\x03\x17\x23\x00", 6);
        QByteArray pass = QByteArray("\x01\x00\x17\x23",4);

        Cobs.clear();
        QByteArrayList output = Cobs.streamDecode(input);

        REQUIRE(output.count() == 1);
        REQUIRE(output[0] == pass);
    }

    SECTION("Decode frame with leading 0") {
        QByteArray input = QByteArray("\x00\x00\x00\x02\x01\x03\x17\x23\x00", 9);
        QByteArray pass = QByteArray("\x01\x00\x17\x23",4);

        Cobs.clear();
        QByteArrayList output = Cobs.streamDecode(input);

        REQUIRE(output.count() == 1);
        REQUIRE(output[0] == pass);
    }

    SECTION( "Decode empty frame" ) {
        QByteArray input = QByteArray("\x01\x00", 2);

        Cobs.clear();
        QByteArrayList output = Cobs.streamDecode(input);

        REQUIRE(output.count() == 0);
     }

     SECTION( "Decode frame with one 0" ) {
         QByteArray input = QByteArray("\x01\x01\x00", 3);
         QByteArray pass = QByteArray("\x00",1);

         Cobs.clear();
         QByteArrayList output = Cobs.streamDecode(input);

         REQUIRE(output.count() == 1);
         REQUIRE(output[0] == pass);
     }

      SECTION( "Decode frame with one byte" ) {
         QByteArray input = QByteArray("\x02\x77\x00", 3);
         QByteArray pass = QByteArray("\x77",1);

         Cobs.clear();
         QByteArrayList output = Cobs.streamDecode(input);

         REQUIRE(output.count() == 1);
         REQUIRE(output[0] == pass);
     }

     SECTION( "Decode frame without 0" ) {
          QByteArray input = QByteArray("\x05\x77\x66\x55\x44\x00", 6);
          QByteArray pass = QByteArray("\x77\x66\x55\x44",4);

          Cobs.clear();
          QByteArrayList output = Cobs.streamDecode(input);

          REQUIRE(output.count() == 1);
          REQUIRE(output[0] == pass);
      }

      SECTION( "Decode invalide frame without end 0" ) {
          QByteArray input = QByteArray("\x05\x77\x66\x55\x44\x09", 6);

          Cobs.clear();
          QByteArrayList output = Cobs.streamDecode(input);

          REQUIRE(output.count() == 0);
      }

      SECTION( "Decode no data" ) {
          QByteArray input = QByteArray("", 0);

          Cobs.clear();
          QByteArrayList output = Cobs.streamDecode(input);

          REQUIRE(output.count() == 0);
      }

      SECTION( "Decode no data but 0" ) {
          QByteArray input = QByteArray("\x00", 1);

          Cobs.clear();
          QByteArrayList output = Cobs.streamDecode(input);

          REQUIRE(output.count() == 0);
      }

      SECTION( "Decode no data but 0" ) {
          QByteArray input = QByteArray("\x00\x00\x00", 3);

          Cobs.clear();
          QByteArrayList output = Cobs.streamDecode(input);

          REQUIRE(output.count() == 0);
      }

      SECTION( "Decode data with multiple frames" ) {
          QByteArray input = QByteArray("\x03\x77\x66\x00\x02\x44\x00", 7);
          QByteArray pass1 = QByteArray("\x77\x66",2);
          QByteArray pass2 = QByteArray("\x44",1);

          Cobs.clear();
          QByteArrayList output = Cobs.streamDecode(input);

          REQUIRE(output.count() == 2);
          REQUIRE(output[0] == pass1);
          REQUIRE(output[1] == pass2);
      }

      SECTION( "Decode data with multiple frames in multiple batches" ) {
          QByteArray input1 = QByteArray("\x03\x77\x66\x00\x02", 5);
          QByteArray input2 = QByteArray("\x44\x00", 2);
          QByteArray pass1 = QByteArray("\x77\x66",2);
          QByteArray pass2 = QByteArray("\x44",1);

          Cobs.clear();

          QByteArrayList output1 = Cobs.streamDecode(input1);
          REQUIRE(output1.count() == 1);
          REQUIRE(output1[0] == pass1);

          QByteArrayList output2 = Cobs.streamDecode(input2);
          REQUIRE(output2.count() == 1);
          REQUIRE(output2[0] == pass2);
      }
}

//#endif
