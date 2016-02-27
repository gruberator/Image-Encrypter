#include "qlabelclickable.h"

void QLabelClickable::timerEnded()
{
    timer.stop();
    emit clicked();
}

void QLabelClickable::mousePressEvent(QMouseEvent *event)
{
    timer.start(doubleClickTime);
}

void QLabelClickable::mouseDoubleClickEvent(QMouseEvent *event)
{
    timer.stop();
    emit doubleClicked();
}

void QLabelClickable::init()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerEnded()));
}
