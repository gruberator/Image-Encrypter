/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include <iostream>
#include "imageviewer.h"

using namespace std;

ImageViewer::ImageViewer(){
    widgetArea = new QWidget;
    widgetArea->setBackgroundRole(QPalette::Light);
    widgetArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    encodingToolbox = new EncodingToolBox(widgetArea);
    imageStorageLabel = new QLabelClickable();
    imageStorageLabel->setBackgroundRole(QPalette::Base);
    imageStorageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageStorageLabel->setScaledContents(true);
    connect(imageStorageLabel, SIGNAL(clicked()), this, SLOT(openStorage()));
    connect(imageStorageLabel, SIGNAL(doubleClicked()), this, SLOT(saveStorageTo()));

    scrollArea = encodingToolbox->scrollStorageArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageStorageLabel);

    imageMessageLabel = new QLabelClickable;
    imageMessageLabel->setBackgroundRole(QPalette::Base);
    imageMessageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageMessageLabel->setScaledContents(true);
    connect(imageMessageLabel, SIGNAL(clicked()), this, SLOT(openMessage()));
    connect(imageMessageLabel, SIGNAL(doubleClicked()), this, SLOT(saveMessageTo()));

    scrollMessageArea = encodingToolbox->scrollMessageArea;
    scrollMessageArea->setBackgroundRole(QPalette::Dark);
    scrollMessageArea->setWidget(imageMessageLabel);

    setCentralWidget(widgetArea);
    createActions();
    createMenus();

    QRect dim = QGuiApplication::primaryScreen()->availableGeometry();
    dim.setWidth(dim.width()*6/7);
    dim.setHeight(dim.height()*8/9);

    widgetArea->resize(dim.width(), dim.height());
    encodingToolbox->resize(dim.width(), dim.height());

    resize(dim.width(), dim.height());
}

bool ImageViewer::loadFile(const QString &fileName, int fileType)
{
    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
        setWindowFilePath(QString());
        imageStorageLabel->setPixmap(QPixmap());
        imageStorageLabel->adjustSize();
        return false;
    }
    if(fileType == EncodingToolBox::STORING_FILE) {
        imageEncryption = new QImage(image);
        imageStorageLabel->setPixmap(QPixmap::fromImage(image));
        encodingToolbox->loadedNewStorageImage();
    } else if (fileType == EncodingToolBox::MESSAGE_FILE) {
        imageMessage = new QImage(image);
        imageMessageLabel->setPixmap(QPixmap::fromImage(image));
        encodingToolbox->loadedNewMessageImage();
    }
    scaleFactor = 1.0;

    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageStorageLabel->adjustSize();
    if (!fitToWindowAct->isChecked())
        imageMessageLabel->adjustSize();
    return true;
}

void ImageViewer::openStorage()
{
    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog(this, tr("Open Storage File"),
                       picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.first());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first(), EncodingToolBox::STORING_FILE)) {}
}

void ImageViewer::openMessage()
{
    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog(this, tr("Open Message File"),
                       picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.first());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first(), EncodingToolBox::MESSAGE_FILE)) {}
}

void ImageViewer::print()
{
    Q_ASSERT(imageStorageLabel->pixmap());
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageStorageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageStorageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageStorageLabel->pixmap());
    }
#endif
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    imageStorageLabel->adjustSize();
    imageMessageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    scrollMessageArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        normalSize();
    }
    updateActions();
}


void ImageViewer::about()
{
    QMessageBox::about(this, tr("About Image Encrypted"),
                       tr("<p>The <b>Image Encrypter</b> is program for stenography.</p>"
                          "<p>Its purpose is to hide Message image into Storage image. "
                          " User picks how many bits are used for message image and if "
                          " message image needs to be stored as gray scale "
                          " image or color image."
                          " App is encoding Message image on the Least Significant Bits of the Message image.</p>"
                          " <p>Left image is Storing image, right Message image, which will be hidden in storage image</p>"
                          " <p><b>NOTE</b> Storing image needs to be in <b>BMP</b> format as any compression will distort hidden message<\p>"
                          " <p>Shortcuts"
                          " <b>Left Mouse Button Click<\b> on left/right field - open image\n"
                          " <b>Left Mouse Button Double Click<\b> on left/right field - save image\n<\p>"));
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    QSize newSize = event->size();
    newSize.setWidth( newSize.width() - 10);
    newSize.setHeight(newSize.height() - 30);
    encodingToolbox->resize(newSize);
}

void ImageViewer::createActions()
{
    openAct = new QAction(tr("&Open Storing File..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openStorage()));

    openMsgAct = new QAction(tr("&Open Message File..."), this);
    openMsgAct->setShortcut(tr("Ctrl+P"));
    connect(openMsgAct, SIGNAL(triggered()), this, SLOT(openMessage()));

    saveStorageAct = new QAction(tr("&Save Storage File..."), this);
    saveStorageAct->setShortcut(tr("Ctrl+S"));
    connect(saveStorageAct, SIGNAL(triggered()), this, SLOT(saveStorageTo()));

    saveMessagegAct = new QAction(tr("&Save Message File..."), this);
    saveMessagegAct->setShortcut(tr("Ctrl+D"));
    connect(saveMessagegAct, SIGNAL(triggered()), this, SLOT(saveMessageTo()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setEnabled(false);
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

    aboutAct = new QAction(tr("&About Image Encrypter"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void ImageViewer::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addSection("Open");
    fileMenu->addAction(openAct);
    fileMenu->addAction(openMsgAct);
    fileMenu->addSection("Save");
    fileMenu->addAction(saveStorageAct);
    fileMenu->addAction(saveMessagegAct);
    fileMenu->addSeparator();
    fileMenu->addAction(printAct);
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(helpMenu);
}

void ImageViewer::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageStorageLabel->pixmap());
    scaleFactor *= factor;
    imageStorageLabel->resize(scaleFactor * imageStorageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageViewer::saveStorageTo()
{
    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    QFileDialog dialog(this, tr("Choose Path to save Storage file"), QDir::currentPath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first(),EncodingToolBox::STORING_FILE)) {}
}

void ImageViewer::saveMessageTo()
{
    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    QFileDialog dialog(this, tr("Choose Path to save Message file"), QDir::currentPath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first(),EncodingToolBox::MESSAGE_FILE)) {}
}

bool ImageViewer::saveFile(const QString &path, quint32 type)
{
    cout << "saveFile " << path.toStdString() << endl;
    if(EncodingToolBox::STORING_FILE == type) {
        QLabel *labelStorage = static_cast<QLabel*> (scrollArea->widget());
        const QPixmap *existingStorageArea = labelStorage->pixmap();
        QImage img = existingStorageArea->toImage();
        img.save(path,"BMP");
        return true;
    } else if (EncodingToolBox::MESSAGE_FILE == type) {
        QLabel *labelMessage = static_cast<QLabel*> (scrollMessageArea->widget());
        const QPixmap *existingMessageArea = labelMessage->pixmap();
        QImage img = existingMessageArea->toImage();
        img.save(path,"BMP");
        return true;
    }
    return false;
}

