#include "memoryTextWidget.h"

MemoryTextWidget::MemoryTextWidget(QWidget *parent)
    :QTextEdit(parent)
{
    QFont font1;
    font1.setFamilies({QStringLiteral("Consolas")});
    font1.setKerning(false);
    setFont(font1);
    setReadOnly(true);
}

void MemoryTextWidget::updateMemory(const QList<UiDatatypes::MemoryByte> &data, uint32_t appStart)
{
    clear();
    append(QStringLiteral("                   00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F"));
    append(QStringLiteral("                   ................................................"));

    for(uint16_t i = 0; i<data.count(); i+=16){
        QString line = QString::number(i).rightJustified(5,' ')+QStringLiteral(" / ");
        line += QString::number(i,16).toUpper().rightJustified(4,'0').prepend(QStringLiteral("0x"));
        line += QStringLiteral("  : ");
        for(uint16_t j = 0; j<16; j++){

            uint16_t offset = i+j;
            if(offset >= data.count()) break;

            if(appStart == offset){
                append(QStringLiteral("--------------  :  APP Start --------------------------------------"));
            }

            UiDatatypes::MemoryByte byte = data.at(offset);
            if(byte.read){
                line +=QStringLiteral(" ")+QString::number(byte.byte,16).toUpper().rightJustified(2,'0');
            }else{
                line +=QStringLiteral(" --");
            }
            if(j == 7)line += QStringLiteral(" ");
        }
        append(line);
    }
}
