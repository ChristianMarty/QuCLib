TEMPLATE = app
QT += gui

CONFIG += c++11

isEmpty(CATCH_INCLUDE_DIR): CATCH_INCLUDE_DIR=$$(CATCH_INCLUDE_DIR)
!isEmpty(CATCH_INCLUDE_DIR): INCLUDEPATH *= $${CATCH_INCLUDE_DIR}

isEmpty(CATCH_INCLUDE_DIR): {
    message("CATCH_INCLUDE_DIR is not set, assuming Catch2 can be found automatically in your system")
}

SOURCES += \
	main.cpp     \
    quclibtest.cpp \
    ../source/crc.cpp \
    ../source/hexFileParser.cpp \
    ../source/cobs.cpp

HEADERS += \
    ../source/cobs.h \
    ../source/crc.h \
    ../source/hexFileParser.h \
    catch2/catch.hpp \
    catch2/catch_reporter_automake.hpp \
    catch2/catch_reporter_sonarqube.hpp \
    catch2/catch_reporter_tap.hpp \
    catch2/catch_reporter_teamcity.hpp \
    hexFileParser/test_hexFileParser.hpp \
    test_cobs.hpp \
    test_crc.hpp
