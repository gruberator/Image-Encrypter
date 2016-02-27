#ifndef IMAGE_ENCRYPTER_QLABELCLICKABLE_H
#define IMAGE_ENCRYPTER_QLABELCLICKABLE_H

#include <QLabel>
#include <QTimer>

class QLabelClickable: public QLabel
{
    Q_OBJECT

public:
    explicit QLabelClickable(QWidget *parent=0, Qt::WindowFlags f=0):QLabel(parent, f), doubleClickTime(300){init();}
    explicit QLabelClickable(const QString &text, QWidget *parent=0, Qt::WindowFlags f=0):QLabel(text, parent, f), doubleClickTime(300){init();}
    virtual ~QLabelClickable(){}
private slots:
    void timerEnded();
signals:
    void clicked();
    void doubleClicked();
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
private:
    void init();
    QTimer timer; // to detect double click instead of click
    int doubleClickTime; //in ms

};

#endif // IMAGE_ENCRYPTER_QLABELCLICKABLE_H
