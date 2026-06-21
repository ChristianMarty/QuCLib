#ifndef MEMORY_TEXT_WIDGET_H
#define MEMORY_TEXT_WIDGET_H

#include <QTextEdit>
#include "uiDatatypes.h"

class MemoryTextWidget : public QTextEdit
{
public:
    explicit MemoryTextWidget(QWidget *parent = nullptr);

    void updateMemory(const QList<UiDatatypes::MemoryByte> &data, uint32_t appStart = 0xFFFFFFFF);
};

#endif // MEMORY_TEXT_WIDGET_H
