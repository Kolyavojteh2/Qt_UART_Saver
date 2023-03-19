#include <QApplication>

#include <QtSerialPort/QSerialPort>
#include <QString>

#include <QPlainTextEdit>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <fstream>

const int UART_BufferSize = 4096;
const char* UART_DefaultDevice = "/dev/ttyUSB0";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *w = new QWidget;

    QPlainTextEdit *plainText = new QPlainTextEdit;
    plainText->setReadOnly(true);

    QPushButton *button_clear = new QPushButton("&Clear");

    QPushButton *button_saveToFile = new QPushButton("&Save file");

    QVBoxLayout *box = new QVBoxLayout;
    box->addWidget(plainText);
    box->addWidget(button_clear);
    box->addWidget(button_saveToFile);
    w->setLayout(box);

    QSerialPort serial;
    serial.setPortName(UART_DefaultDevice);
    serial.setBaudRate(QSerialPort::BaudRate::Baud115200);
    serial.setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    serial.setParity(QSerialPort::Parity::NoParity);
    serial.setDataBits(QSerialPort::DataBits::Data8);
    serial.setStopBits(QSerialPort::StopBits::OneStop);
    serial.setReadBufferSize(UART_BufferSize);

    QObject::connect(button_clear, &QPushButton::clicked,
                     [&]() {
        plainText->clear();
    });

    if (!serial.open(QIODevice::OpenModeFlag::ReadWrite)) {
        plainText->appendPlainText("Error serial connection not established!");
    } else {
        QObject::connect(&serial, &QSerialPort::readyRead,
                         [&]() {
            auto buffer = serial.readAll();

            while (buffer.contains((char)0xFF)) {
                buffer.remove(buffer.indexOf((char)0xFF), 1);
            }

            while (buffer.contains('\0')) {
                buffer.remove(buffer.indexOf('\0'), 1);
            }

            if (buffer.isEmpty()) {
                return;
            }

            plainText->moveCursor(QTextCursor::End);
            plainText->insertPlainText(QString::fromStdString(buffer.toStdString()));
        });
    }

    QObject::connect(button_saveToFile, &QPushButton::clicked,
                     [&]() {
        std::ofstream file;
        file.open("receivedFile", std::ios_base::out);

        if (!file.is_open()) {
            plainText->appendPlainText("Error file not opened!\n");
            return;
        }

        file << plainText->toPlainText().toStdString();

        file.flush();
        file.close();
    });

    w->resize(800, 500);
    w->show();

    return app.exec();
}
