#ifndef IMAGE_ENCRYPTER_ENCODINGTOOLBOX_H
#define IMAGE_ENCRYPTER_ENCODINGTOOLBOX_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QScrollArea>
#include <QMessageBox>
#include <QVector>
#include <QFuture>
#include <QFutureWatcher>

using namespace std;

namespace Ui {
class EncodingToolBox;
}

/**
 * @brief The EncodingToolBox class
 *        is a ui class responsible for handling encryption/decryption of images
 */
class EncodingToolBox : public QWidget
{
    Q_OBJECT
public:
    enum FileType {
        STORING_FILE,
        MESSAGE_FILE
    };
    explicit EncodingToolBox(QWidget *parent = 0);
    ~EncodingToolBox();
    QScrollArea *scrollStorageArea;
    QScrollArea *scrollMessageArea;

    void loadedNewMessageImage() {
        imageMessageLoaded = false;
    }
    void loadedNewStorageImage() {
        imageStorageLoaded = false;
    }

private slots:
    void grayOrColorScaleButtonPushed();
    void encryptMessageToStorageSlot();
    void decryptMessageFromStorageSlot();
    void buttonDefaultPushed();
    void buttonDecreaseBitsPushed();
    void buttonIncreaseBitsPushed();
    void sliderUpdated(int value); //grey scale slider changed state
    void imageWasEncrypted();
    void imageWasDecrypted();

    void updateEDCheckbox();

private:
    void toggleEncryptDecryptAction();
    QImage imageMessage; // holds modified image
    QImage imageStorage; // holds modified image
    bool imageMessageLoaded;
    bool imageStorageLoaded;
    bool transformToGray; // or colors

    void  setupGraySlider();
    Ui::EncodingToolBox *ui;
    QVector<QVector<quint32> > calculateHistogram(const QImage &image);
    void setupHistogramWidget(QVector<QVector<quint32> > &histogram);
    QFuture< QVector <QImage> > encryptedImages;
    QFutureWatcher< QVector<QImage> > futureEncryptedWatcher;
    QFutureWatcher< QVector<QImage> > futureDecryptedWatcher;
    QFuture< QVector<QImage> > futureEncryptedImages;
    QFuture< QVector<QImage> > futureDecryptedImages;
};
#endif // IMAGE_ENCRYPTER_ENCODINGTOOLBOX_H
