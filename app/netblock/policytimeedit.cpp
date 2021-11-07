#include "policytimeedit.h"
#include <QDebug>

PolicyTimeEdit::PolicyTimeEdit(QWidget *parent) : QTimeEdit(parent)
{
    setDisplayFormat("hh:mm");
    setWrapping(true);
}

void PolicyTimeEdit::stepBy(int steps)
{
    if(currentSection() == MinuteSection){
        setTime(time().addSecs(steps * 600));
        return;
    }

    QTimeEdit::stepBy(steps);
}
