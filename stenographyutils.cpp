#include "stenographyutils.h"
#include <QCryptographicHash>
#include <math.h>

void obfuscateMessage(QImage &imgMessage, encryption_tools::Encryption_details encryptionDetails)
{
    // TODO add proper obfuscation of the message image which will be done before hiding picture
}

void deobfuscateMessage(QImage &imgMessage, encryption_tools::Encryption_details encryptionDetails)
{
    // TODO implement
}

QColor transformPixelToGrayScale(QRgb pixel, QVector<quint32> &grayScale)
{
    int r,g,b, gray;
    r = (pixel >> 16) & 0xFF;
    g = (pixel >> 8) & 0xFF;
    b = pixel & 0xFF;
    gray = (r+b+g)/3;
    QVector<quint32>::iterator low;
    low = std::lower_bound (grayScale.begin(), grayScale.end(),gray);
    quint32 dist = low - grayScale.begin();
    if(dist < grayScale.size()) {
        gray = grayScale[dist];
    } else {
        gray = grayScale[grayScale.size()-1];
    }
    QColor color;
    color.setRgb(gray,gray,gray);
    return color;
}


QColor transformPixelToColorScale(QRgb pixel,
                                                    QVector<quint32> &rsteps,
                                                    QVector<quint32> &gsteps,
                                                    QVector<quint32> &bsteps)
{
    int r,g,b;
    r = qRed(pixel);
    g = qGreen(pixel);
    b = qBlue(pixel);

    QVector<quint32>::iterator low;
    low = std::lower_bound (rsteps.begin(), rsteps.end(),r);
    quint32 dist = low - rsteps.begin();
    if(low != rsteps.end()) {
        if(dist < rsteps.size()) {
            r = rsteps[dist];
        } else {
            r = rsteps[rsteps.size()-1];
        }
    } else {
        r = 0;
    }
    low = std::lower_bound (gsteps.begin(), gsteps.end(),g);
    dist = low - gsteps.begin();
    if(low != gsteps.end()) {
        if(dist < gsteps.size()) {
            g = gsteps[dist];
        } else {
            g = gsteps[gsteps.size()-1];
        }
    } else {
        g = 0;
    }
    low = std::lower_bound (bsteps.begin(), bsteps.end(),b);
    if(low != gsteps.end()) {
        dist = low - bsteps.begin();
        if(dist < bsteps.size()) {
            b = bsteps[dist];
        } else {
            b = bsteps[bsteps.size()-1];
        }
    } else {
        b = 0;
    }
    QColor color;
    color.setRgb(r,g,b);
    return color;
}

void calculateScaleSteps(quint32 steps, QVector<quint32> &grayScaleStepsVector)
{
    grayScaleStepsVector.clear();
    double delta = 255 / pow(2,steps);
    for(quint32 i = 0 ; i < pow(2,steps) -1 ; ++i) {
        grayScaleStepsVector.push_back((quint32)((i+1)*delta));
    }
}

quint32 getColorScaleIndex(quint32 color, QVector<quint32> &colorScale)
{
    QVector<quint32>::iterator low;
    low = std::lower_bound (colorScale.begin(), colorScale.end(),color);
    quint32 dist = low - colorScale.begin();
    if(dist >= colorScale.size()) {
        dist = colorScale.size()-1;
    }
    return dist;
}

QVector<QImage> encryption_tools::encryptMessage(QImage imgMessage, QImage imgStorage,
                                                 bool matchSizeToStorage, encryption_tools::Encryption_details encryptionDetails)
{
    // match size of images
    bool transformToGray = encryptionDetails.grayMessage;
    quint32 bitsForMessage = encryptionDetails.bitsPerMessage;
    if(matchSizeToStorage) {
        imgMessage = imgMessage.scaled(imgStorage.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    } else {
        imgStorage = imgStorage.scaled(imgMessage.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    // decide steps for color transformation
    QVector<quint32> rChannelMessageSteps, gChannelMessageSteps,bChannelMessageSteps,grayScaleSteps;
    quint32 rSteps,gSteps,bSteps; // number of steps per channel
    calculateScaleSteps(bitsForMessage, grayScaleSteps);

    // info about division would need to be encrypted to the hidden message
    calculateNumberOfRGBPerMessageChannel(bitsForMessage, rSteps, gSteps, bSteps);
    calculateScaleSteps(rSteps, rChannelMessageSteps);
    calculateScaleSteps(gSteps, gChannelMessageSteps);
    calculateScaleSteps(bSteps, bChannelMessageSteps);

    // Change quality of message file to given bit count
    // transform colors of message images to show how this picture will look after encryption
    QColor color;
    for(int x = 0 ; x < imgMessage.width(); ++x) {
        for(int y = 0 ; y < imgMessage.height(); ++y) {
            QRgb col = imgMessage.pixel(x,y);
            if(transformToGray) {
                color = transformPixelToGrayScale(col, grayScaleSteps);
            } else {
                color = transformPixelToColorScale(col, rChannelMessageSteps, gChannelMessageSteps, bChannelMessageSteps);
            }
            imgMessage.setPixel(QPoint(x,y), color.rgba());
        }
    }
    obfuscateMessage(imgMessage, encryptionDetails);

    quint32 r,g,b;
    calculateNumberOfRGBPerMessageChannel(bitsForMessage, r, g, b);
    quint32 rMask, gMask, bMask;
    rMask = (0xFF << r) & 0xFF;
    gMask = (0xFF << g) & 0xFF;
    bMask = (0xFF << b) & 0xFF;

    for(int x = 0 ; x < imgStorage.width(); ++x) {
        for(int y = 0 ; y < imgStorage.height(); ++y) {
            QRgb col = imgStorage.pixel(x,y);
            quint32 rChannel, gChannel, bChannel;
            rChannel = qRed(col)   & rMask;
            gChannel = qGreen(col) & gMask;
            bChannel = qBlue(col)  & bMask;
            QRgb msgCol = imgMessage.pixel(x,y);
            // reuse bit mechanism, change color value here
            if(transformToGray) {
                quint32 colorScaleIndexToEncode = getColorScaleIndex(msgCol&0xFF, grayScaleSteps);
                quint32 rChMask = (((0xFF<<r)^0xFF)&0xFF) << (g+b);
                quint32 gChMask = (((0xFF<<g)^0xFF)&0xFF) << b;
                quint32 bChMask = ((0xFF<<b)^0xFF)&0xFF;
                // setup channel mask for message
                rChannel |= (colorScaleIndexToEncode & rChMask) >> (g+b);
                gChannel |= (colorScaleIndexToEncode & gChMask) >> b;
                bChannel |= (colorScaleIndexToEncode & bChMask);
            } else {
                quint32 valR = getColorScaleIndex(qRed(msgCol), rChannelMessageSteps);
                quint32 valG = getColorScaleIndex(qGreen(msgCol), gChannelMessageSteps);
                quint32 valB = getColorScaleIndex(qBlue(msgCol), bChannelMessageSteps);
                // valR valG valB has the same size as the encoded message, or should be TEST IT!
                rChannel |= valR;
                gChannel |= valG;
                bChannel |= valB;
            }
            rChannel &= 0xFF;
            gChannel &= 0xFF;
            bChannel &= 0xFF;
            color.setRgb(rChannel, gChannel, bChannel);
            imgStorage.setPixel(QPoint(x,y), color.rgba());
        }
    }

    QVector< QImage > encryptedImages;
    encryptedImages.push_back(imgMessage);
    encryptedImages.push_back(imgStorage);
    return encryptedImages;
}

QVector<QImage> encryption_tools::decryptMessage(QImage imgStorage, encryption_tools::Encryption_details encryptionDetails)
{
    QImage imgMessage(imgStorage);
    // decide steps for color transformation
    QVector<quint32> rsteps, gsteps,bsteps,grayScaleSteps;
    quint32 r,g,b;
    quint32 bitsForMessage = encryptionDetails.bitsPerMessage;
    bool transformToGray = encryptionDetails.grayMessage;

    if(transformToGray) {
        calculateScaleSteps(bitsForMessage, grayScaleSteps);
    } else {
        calculateNumberOfRGBPerMessageChannel(bitsForMessage, r, g, b);
        calculateScaleSteps(r, rsteps);
        calculateScaleSteps(g, gsteps);
        calculateScaleSteps(b, bsteps);
    }

    calculateNumberOfRGBPerMessageChannel(bitsForMessage, r, g, b);
    quint32 rMask, gMask, bMask;
    rMask = ((0xFF << r)^0xFF) & 0xFF;
    gMask = ((0xFF << g)^0xFF) & 0xFF;
    bMask = ((0xFF << b)^0xFF) & 0xFF;
    for(int x = 0 ; x < imgStorage.width(); ++x) {
        for(int y = 0 ; y < imgStorage.height(); ++y) {
            QRgb col = imgStorage.pixel(x,y);
            quint32 rChannel, gChannel, bChannel;
            // those data needs to be taken to message color palette
            rChannel = qRed(col) & rMask;
            gChannel = qGreen(col) & gMask;
            bChannel = qBlue(col) & bMask;

            QColor messagePixelColor = Qt::black;
            if(transformToGray) {
                quint32 grayScaleIndex = (rChannel << (g+b)) | (gChannel << b) | (bChannel);
                quint32 color = Qt::black;
                if(grayScaleIndex < grayScaleSteps.size()) {
                    color = grayScaleSteps[grayScaleIndex];
                }
                messagePixelColor.setRgb(color, color, color);
                imgMessage.setPixel(x,y,messagePixelColor.rgba());
            } else {
                quint32 rIndex = rChannel;
                quint32 gIndex = gChannel;
                quint32 bIndex = bChannel;
                if(rIndex < rsteps.size()) {
                    messagePixelColor.setRed(rsteps[rIndex]);
                }
                if(gIndex < gsteps.size()) {
                    messagePixelColor.setGreen(gsteps[gIndex]);
                }
                if(bIndex < bsteps.size()) {
                    messagePixelColor.setBlue(bsteps[bIndex]);
                }
                imgMessage.setPixel(x,y,messagePixelColor.rgba());
            }
            // clear message from storage
            QColor storagePixelColor(Qt::black);
            rChannel = qRed(col)  & ((rMask^0xFF)&0xFF);
            gChannel = qGreen(col)& ((gMask^0xFF)&0xFF);
            bChannel = qBlue(col) & ((bMask^0xFF)&0xFF);
            storagePixelColor.setRgb(rChannel, gChannel, bChannel);
            imgStorage.setPixel(x,y,storagePixelColor.rgba());
        }
    }
    deobfuscateMessage(imgMessage, encryptionDetails);
    QVector< QImage > encryptedImages;
    encryptedImages.push_back(imgMessage);
    encryptedImages.push_back(imgStorage);
    return encryptedImages;
}

void encryption_tools::calculateNumberOfRGBPerMessageChannel(quint32 bitsForMessage, quint32 &r, quint32 &g, quint32 &b)
{
    r = bitsForMessage / 3;
    g = bitsForMessage / 3;
    b = bitsForMessage / 3;
    bitsForMessage -= 3*(bitsForMessage/3);
    while(bitsForMessage>0) {
        if(bitsForMessage%3 == 0) {
            ++r;
        } else if(bitsForMessage%3 == 1) {
            ++g;
        } else if(bitsForMessage%3 == 2) {
            ++b;
        }
        --bitsForMessage;
    }
}
