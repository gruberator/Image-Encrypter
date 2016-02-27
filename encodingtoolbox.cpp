#include "encodingtoolbox.h"
#include "ui_encodingtoolbox.h"
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <QtConcurrent/QtConcurrent>

#include "stenographyutils.h"

using namespace std;

#define MAX_BITS_ALLOWED 24

EncodingToolBox::EncodingToolBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EncodingToolBox)
{
    ui->setupUi(this);
    setupGraySlider();

    scrollStorageArea = ui->scrollStorageArea;
    scrollMessageArea = ui->scrollMessageArea;
    imageMessageLoaded = false;
    imageStorageLoaded = false;
    transformToGray = true;
    ui->buttonGrayTransform->setToolTip(QString(tr("Toggle between message encrypted as gray or color picture")));

    connect(ui->buttonEncrypt, SIGNAL(clicked()), this, SLOT(encryptMessageToStorageSlot()));
    connect(ui->buttonDecrypt, SIGNAL(clicked()), this, SLOT(decryptMessageFromStorageSlot()));
    connect(ui->buttonDecreaseBitCount, SIGNAL(clicked()), this, SLOT(buttonDecreaseBitsPushed()));
    connect(ui->buttonIncreaseBitCount, SIGNAL(clicked()), this, SLOT(buttonIncreaseBitsPushed()));
    connect(ui->buttonDefaultImage, SIGNAL(clicked()), this, SLOT(buttonDefaultPushed()));
    connect(ui->sliderGreyScale, SIGNAL(valueChanged(int)), this, SLOT(sliderUpdated(int)));
    connect(ui->buttonGrayTransform, SIGNAL(clicked()), this, SLOT(grayOrColorScaleButtonPushed()));
    connect(ui->checkBoxEncryptDecrypt, SIGNAL(clicked()), this, SLOT(updateEDCheckbox()));
    connect(&futureEncryptedWatcher, SIGNAL(finished()), this, SLOT(imageWasEncrypted()));
    connect(&futureDecryptedWatcher, SIGNAL(finished()), this, SLOT(imageWasDecrypted()));

    // TODO implement obfuscation/encryption of message before putting it to storage image
    ui->label->hide();
    ui->editTextPassword->hide();
}

EncodingToolBox::~EncodingToolBox()
{
    delete ui;
}

void EncodingToolBox::toggleEncryptDecryptAction()
{

    if(ui->checkBoxEncryptDecrypt->isChecked())
        encryptMessageToStorageSlot();
    else
        decryptMessageFromStorageSlot();
}

void EncodingToolBox::grayOrColorScaleButtonPushed()
{
    if(transformToGray) transformToGray = false;
    else                transformToGray = true;

    if(transformToGray) {
        ui->sliderGreyScale->setMinimum(1);
        ui->buttonGrayTransform->setText(QString(tr("hide COLOR message")));
    } else {
        ui->sliderGreyScale->setMinimum(3);
        ui->buttonGrayTransform->setText(QString(tr("hide GRAY message")));
    }

    toggleEncryptDecryptAction();
}

void EncodingToolBox::encryptMessageToStorageSlot()
{
    QLabel *labelMessage = static_cast<QLabel*> (scrollMessageArea->widget());
    const QPixmap *existingMessageArea = labelMessage->pixmap();
    QLabel *labelStorage = static_cast<QLabel*> (scrollStorageArea->widget());
    const QPixmap *existingStorageArea = labelStorage->pixmap();

    if(existingMessageArea != NULL) {
        if(existingStorageArea != NULL) {
            QImage imgMessage = existingMessageArea->toImage();
            if(!imageMessageLoaded) {
                imageMessage = imgMessage;
                imageMessageLoaded = true;
            } else {
                imgMessage = imageMessage;
            }
            QImage imgStorage = existingStorageArea->toImage();
            if(!imageStorageLoaded) {
                imageStorage = imgStorage;
                imageStorageLoaded = true;
            } else {
                imgStorage = imageStorage;
            }
            bool matchSizeToStorage = ui->checkMatchSizeToStorage->isChecked();
            quint32 bitSliderValue = ui->sliderGreyScale->value();
            if(futureDecryptedImages.isRunning()) {
                futureDecryptedImages.cancel();
            }
            if(futureEncryptedImages.isRunning()) {
                futureEncryptedImages.cancel();
            }
            encryption_tools::Encryption_details encryptionDetails(bitSliderValue, transformToGray, ui->editTextPassword->toPlainText());
            futureEncryptedImages = QtConcurrent::run(encryption_tools::encryptMessage,
                                                      imgMessage, imgStorage, matchSizeToStorage, encryptionDetails);
            futureEncryptedWatcher.setFuture(futureEncryptedImages);
        }
    }
}

void EncodingToolBox::decryptMessageFromStorageSlot()
{
    QLabel *labelStorage = static_cast<QLabel*> (scrollStorageArea->widget());
    const QPixmap *existingStorageArea = labelStorage->pixmap();
    if(existingStorageArea != NULL) {
        QImage imgStorage = existingStorageArea->toImage();
        if(!imageStorageLoaded) {
            imageStorage = imgStorage;
            imageStorageLoaded = true;
        } else {
            imgStorage = imageStorage;
        }
        quint32 bitSliderValue = ui->sliderGreyScale->value();
        encryption_tools::Encryption_details encryptionDetails(bitSliderValue, transformToGray, ui->editTextPassword->toPlainText());
        futureDecryptedImages = QtConcurrent::run(encryption_tools::decryptMessage,imgStorage, encryptionDetails);
        futureDecryptedWatcher.setFuture(futureDecryptedImages);
    }
}

void EncodingToolBox::buttonDefaultPushed()
{
    if(imageMessageLoaded) {
        QLabel *labelMessage = static_cast<QLabel*> (scrollMessageArea->widget());
        labelMessage->setPixmap(QPixmap::fromImage(imageMessage));
    }
    if(imageStorageLoaded) {
        QLabel *labelStorage = static_cast<QLabel*> (scrollStorageArea->widget());
        labelStorage->setPixmap(QPixmap::fromImage(imageStorage));
    }
}

void EncodingToolBox::buttonDecreaseBitsPushed()
{
    int bits = ui->sliderGreyScale->value();
    if(bits>0) {
        --bits;
    }
    ui->sliderGreyScale->setValue(bits);

    toggleEncryptDecryptAction();
}

void EncodingToolBox::buttonIncreaseBitsPushed()
{
    int bits = ui->sliderGreyScale->value();
    if(bits < MAX_BITS_ALLOWED) {
        ++bits;
    }
    ui->sliderGreyScale->setValue(bits);

    toggleEncryptDecryptAction();
}

void EncodingToolBox::sliderUpdated(int value)
{
    QString msg = (tr("Selected number of bits: "));
    ui->labelBitCount->setText(msg + QString::number(value));

    quint32 r,g,b;
    encryption_tools::calculateNumberOfRGBPerMessageChannel(ui->sliderGreyScale->value(), r, g, b);
    msg = "RGB bits number taken from storage: " + QString::number(r) + " " + QString::number(g) + " " + QString::number(b) + "\n";
    for(quint32 i = 0 ; i < r ; ++i)
        msg += 'r';
    for(quint32 i = 0 ; i < g ; ++i)
        msg += 'g';
    for(quint32 i = 0 ; i < b ; ++i)
        msg += 'b';
    ui->label->setText(msg);

    if(ui->checkBoxEncrypt->isChecked()){
        if(ui->checkBoxEncryptDecrypt->isChecked())
            encryptMessageToStorageSlot();
        else
            decryptMessageFromStorageSlot();
    }
}

void EncodingToolBox::imageWasEncrypted()
{
    QVector<QImage> vectorImage = futureEncryptedImages.result();
    if(vectorImage.size() == 2){
        QLabel *labelMessage = static_cast<QLabel*> (scrollMessageArea->widget());
        QLabel *labelStorage = static_cast<QLabel*> (scrollStorageArea->widget());
        labelMessage->setPixmap(QPixmap::fromImage(vectorImage[0]));
        labelStorage->setPixmap(QPixmap::fromImage(vectorImage[1]));
    }
}

void EncodingToolBox::imageWasDecrypted()
{
    QVector<QImage> vectorImage = futureDecryptedImages.result();
    if(vectorImage.size() == 2){
        QLabel *labelMessage = static_cast<QLabel*> (scrollMessageArea->widget());
        QLabel *labelStorage = static_cast<QLabel*> (scrollStorageArea->widget());
        labelMessage->setPixmap(QPixmap::fromImage(vectorImage[0]));
        labelStorage->setPixmap(QPixmap::fromImage(vectorImage[1]));
    }
}

void EncodingToolBox::updateEDCheckbox()
{
    if(ui->checkBoxEncryptDecrypt->isChecked()){
        ui->checkBoxEncryptDecrypt->setText("Encrypt, check to Decrypt");
    } else {
        ui->checkBoxEncryptDecrypt->setText("Decrypt, check to Encrypt");
    }
}

void EncodingToolBox::setupGraySlider()
{
    ui->sliderGreyScale->setMinimum(1);
    ui->sliderGreyScale->setMaximum(MAX_BITS_ALLOWED);
    ui->sliderGreyScale->setTickPosition(QSlider::TicksAbove);
    ui->sliderGreyScale->setSingleStep(1);
    ui->sliderGreyScale->setPageStep(3);
    ui->sliderGreyScale->setValue(3);
}

QVector<QVector<quint32> > EncodingToolBox::calculateHistogram(const QImage &image)
{
    QVector<quint32> rHistogram;
    QVector<quint32> gHistogram;
    QVector<quint32> bHistogram;
    for(quint16 i = 0 ; i < 256 ; ++i ) {
        rHistogram.push_back(0);
        gHistogram.push_back(0);
        bHistogram.push_back(0);
    }
    for(qint32 x = 0 ; x < image.width() ; ++x) {
        for(qint32 y = 0 ; y < image.height() ; ++y) {
            QRgb col = image.pixel(x,y);
            rHistogram[qRed(col)  ] += 1;
            gHistogram[qGreen(col)] += 1;
            bHistogram[qBlue(col) ] += 1;
        }
    }

    QVector< QVector<quint32> > rgbHistogram;
    rgbHistogram.push_back(rHistogram);
    rgbHistogram.push_back(gHistogram);
    rgbHistogram.push_back(bHistogram);
    return rgbHistogram;
}

