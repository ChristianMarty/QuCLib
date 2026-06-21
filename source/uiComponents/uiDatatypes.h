#ifndef UI_DATATYPE_H
#define UI_DATATYPE_H

#include <stdint.h>

namespace UiDatatypes {

    struct MemoryByte {
        bool read = false;
        uint8_t byte = 0xFF;
    };
};

#endif // UI_DATATYPE_H