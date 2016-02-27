#ifndef IMAGE_ENCRYPTER_STENOGRAPHY_UTILS_H
#define IMAGE_ENCRYPTER_STENOGRAPHY_UTILS_H

#include <QString>
#include <QImage>
#include <QColor>
#include <QVector>

namespace encryption_tools {

struct Encryption_details {
    quint32 bitsPerMessage;
    bool grayMessage; // if false then store hidden image as color
    QString password;
    Encryption_details(quint32 bitsPerMessage, bool grayMessage, const QString &password=""):bitsPerMessage(bitsPerMessage),
        grayMessage(grayMessage),password(password){}
    Encryption_details(QString password):password(password){}
};

void calculateNumberOfRGBPerMessageChannel(quint32 bitsForMessage, quint32 &r, quint32 &g, quint32 &b);
QVector<QImage> encryptMessage(QImage imgMessage, QImage imgStorage, bool matchSizeToStorage, Encryption_details encryptionDetails);
QVector<QImage> decryptMessage(QImage imgStorage, Encryption_details encryptionDetails);
}


#endif // IMAGE_ENCRYPTER_STENOGRAPHY_UTILS_H
