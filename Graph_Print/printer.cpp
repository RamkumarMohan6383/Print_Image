/*****************************************************************
* Thermal Printer Interface Library based on QT for the Rasp-Pi
* Based on the Arduino Library from Adafruit
* using QExtSerialPort
* Autor: Tobias Floery
* E-Mail: tobias@floery.net
***************************************************************/

#include "printer.h"
#include <QImage>
#include <iostream>
#include <unistd.h>
#include <QDebug>
#include <QPixmap>
Printer::Printer(QObject *parent) :
    QObject(parent)
{
}

// opens the serial port specified py path

bool Printer::open(QString path) {

    port = new QextSerialPort(path);

    if (!port->open(QIODevice::WriteOnly))
        return false;
    // set options
    port->setBaudRate(BAUD9600);
    port->setDataBits(DATA_8);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);

    usleep(10000);
    return true;
}

// close the serial port
void Printer::close() {
    port->close();
}

// write single byte
void Printer::write(quint8 byte) {
    port->write((const char*)&byte, 1);
}

// write a string
void Printer::write(QString str)
{
    port->write(str.toUtf8());
}

void Printer::writes(char SOMETHING)
{
    port->write(&SOMETHING);
}


// initialize the printer
void Printer::init() {
    reset();
    setStatus(true);
    setControlParameter();
    setPrintDensity();
    setSleepTime();
    setCodeTable();
    setCharacterSet();
    setBarcodePrintReadable();
}

// reset the printer
void Printer::reset() {
    write(27);
    write(64);
    usleep(50000);
}

// sets the printer online (true) or ofline (false)
void Printer::setStatus(bool state) {
    write(27);
    write(61);
    write(state);
}

// set control parameters: heatingDots, heatingTime, heatingInterval
void Printer::setControlParameter(quint8 heatingDots, quint8 heatingTime, quint8 heatingInterval) {
    write(27);
    write(55);
    write(heatingDots);
    write(heatingTime);
    write(heatingInterval);
}

// set sleep Time in seconds, time after last print the printer should stay awake
void Printer::setSleepTime(quint8 seconds) {
    write(27);
    write(56);
    write(seconds);
    usleep(50000);
    write(0xFF);
}

// set double width mode: on=true, off=false
void Printer::setDoubleWidth(bool state) {
    write(27);
    write(state?14:20);

}


// set the print density and break time
void Printer::setPrintDensity(quint8 printDensity, quint8 printBreakTime) {
    write(18);
    write(35);
    write((printBreakTime << 5) | printDensity);
}

// set the used character set
void Printer::setCharacterSet(CharacterSet set) {
    write(27);
    write(82);
    write(set);
}

// set the used code table
void Printer::setCodeTable(CodeTable table) {
    write(27);
    write(116);
    write(table);
}

// feed single line
void Printer::feed(void) {
    write(10);
}

// feed <<lines>> lines
void Printer::feed(quint8 lines) {
    write(27);
    write(74);
    write(lines);
}

// set line spacing
void Printer::setLineSpacing(quint8 spacing) {
    write(27);
    write(51);
    write(spacing);
}

// set Align Mode: LEFT, MIDDLE, RIGHT
void Printer::setAlign(AlignMode align) {
    write(27);
    write(97);
    write(align);
}

// set how many blanks should be kept on the left side
void Printer::setLeftBlankCharNums(quint8 space) {
    if (space >= 47) space = 47;

    write(27);
    write(66);
    write(space);
}

// set Bold Mode: on=true, off=false
void Printer::setBold(bool state) {
    write(27);
    write(32);
    write((quint8) state);
    write(27);
    write(69);
    write((quint8) state);
}

// set Reverse printing Mode
void Printer::setReverse(bool state) {
    write(29);
    write(66);
    write((quint8) state);
}

// set Up/Down Mode
void Printer::setUpDown(bool state) {
    write(27);
    write(123);
    write((quint8) state);
}

// set Underline printing
void Printer::setUnderline(bool state) {
    write(27);
    write(45);
    write((quint8) state);
}

// enable / disable the key on the frontpanel
void Printer::setKeyPanel(bool state) {
    write(27);
    write(99);
    write(53);
    write((quint8) state);
}

// where should a readable barcode code be printed
void Printer::setBarcodePrintReadable(PrintReadable n) {
    write(29);
    write(72);
    write(n);
}

// sets the height of the barcode in pixels
void Printer::setBarcodeHeight(quint8 height) {
    if (height <= 1) height = 1;

    write(29);
    write(104);
    write(height);
}

// sets the barcode line widths (only 2 or 3)
void Printer::setBarCodeWidth(quint8 width) {
    if (width <= 2) width=2;
    else if (width >= 3) width=3;

    write(29);
    write(119);
    write(width);
}

// prints a barcode
void Printer::printBarcode(QString data, BarcodeType type) {
    write(29);
    write(107);
    write(type);
    write(data);
    write(0);
}


bool Printer::isPrinterReady()
{
    return port->isOpen() && port->isWritable();
}

void Printer::printImage(const QImage &image)
{
    if (!isPrinterReady())
    {
        qDebug() << "Printer is not ready.";
        return;
    }
    qreal scaleFactor = 2.0; // Increase DPI by a factor of 2
          qreal darknessFactor =1.0; // Adjust darkness level (experiment with this value)
          // Scale the image for higher DPI (dots per inch)
          QImage highDpiImage = image;

          // Disable device pixel ratio scaling (anti-aliasing)
          highDpiImage.setDevicePixelRatio(1);

              QImage monochromeImage = highDpiImage.convertToFormat(QImage::Format_Mono);
          highDpiImage = monochromeImage.scaled(monochromeImage.width() * scaleFactor, monochromeImage.height() * scaleFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Calculate the width of the image in bytes
    int bytesPerLine = (monochromeImage.width() + 7) / 8;

    // Send the ESC/POS command to print the image
    QByteArray printCommand;
    printCommand.append(char(0x1D));  // ESC
    printCommand.append(char(0x76));  // 'v'
    printCommand.append(char(0x30));  // '0'
    printCommand.append(char(0x00));  // Mode (0 for normal)

    // Append the image width and height
    printCommand.append(char(bytesPerLine & 0xFF));
    printCommand.append(char((bytesPerLine >> 8) & 0xFF));
    printCommand.append(char(monochromeImage.height() & 0xFF));
    printCommand.append(char((monochromeImage.height() >> 8) & 0xFF));

    // Send the print command
    port->write(printCommand);

    // Send the image data
    QByteArray imageData;
    for (int y = 0; y < monochromeImage.height(); ++y)
    {
        for (int x = 0; x < bytesPerLine; ++x)
        {
            uchar byte = 0;
            for (int bit = 0; bit < 8; ++bit)
            {
                int pixelX = x * 8 + bit;
                if (pixelX < monochromeImage.width())
                {
                    bool isBlackPixel = monochromeImage.pixel(pixelX, y) == qRgb(0, 0, 0);
                    //byte |= (isBlackPixel ? 0x01 : 0x07) << bit;
                    byte |= (isBlackPixel ? 0x01 : 0x00) << (7 - bit); // Corrected bit order

                }
            }
            byte *= darknessFactor; // Adjust this factor for darkness level

            imageData.append(byte);
        }
    }

    // Send the image data
    port->write(imageData);
    write("\n"); // Print a newline to advance to the next line
}
