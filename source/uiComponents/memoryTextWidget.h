#ifndef MEMORY_TEXT_WIDGET_H
#define MEMORY_TEXT_WIDGET_H

#include <QTextEdit>

class MemoryTextWidget : public QTextEdit
{
public:
    explicit MemoryTextWidget(QWidget *parent = nullptr);

    struct MemoryByte {
        bool read = false;
        uint8_t byte;
    };

    void updateMemory(const QList<MemoryByte> &data, uint16_t appStart);

};

#endif // MEMORY_TEXT_WIDGET_H
