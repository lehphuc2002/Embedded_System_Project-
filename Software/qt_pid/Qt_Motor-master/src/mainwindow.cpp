#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>
#include <QString>
#include <QList>
#include <QIODevice>
#include <qcustomplot.h>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <QTime>
#include <QTimer>
#include <QVector>
#include <QDateTime>
#include "stdint.h"
#include "string.h"
QByteArray data_read;
QSerialPort *Serialll;
QSerialPortInfo info;
QString dataReadSendGraph;
QList<QSerialPortInfo> list_COM;
QString select_com, select_baud;
QByteArray all_data;
QVector<double> setPointValue(10000);
QByteArray byteArray;
QByteArray frame_rx_fromSTM;
uint16_t frameBuffLenght;
uint8_t frameBuffer[PROTO_BUFF_SIZE_TX] = {0};
uint16_t rx_buff_size = 0;
uint8_t rx_buff[PROTO_DATA_SIZE_RX] = {0};
int8_t check_true;
uint8_t display1 = 1, counterPosi = 0, counterVelo = 0;
uint8_t display2 = 1;
uint8_t checkClear = 0, checkVelo = 0, checkPosi = 0;
uint8_t checkBtnPlot1 = 0;
uint8_t checkBtnPlot2 = 0;
uint8_t checkFirst = 0, checkContinue = 0;
uint32_t counterRealTime = 0;
float setPointPosi;
// bool checkRescale = false;
bool checkContRead = false, btnResetClick = false;
static int checkSizeDataVector, checkSizeValues;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Serialll = new QSerialPort(this);
    // QIODevice::ReadWrite là một cờ được sử dụng để mở kết nối serial với quyền truyền và nhận dữ liệu.
    // Cờ này cho phép ứng dụng có thể gửi và nhận dữ liệu trên kết nối serial.
    //    QString str_1 = "Alo anh em";
    //    qDebug() << str_1;
    //    QString str2 = QString(3, 'a');
    //    qDebug() << str2;
    //    int i = 10;
    //    QString str3 = QString::number(i,16);
    //    qDebug() << str3;
    updateCOM();
    customize_ui();
}

MainWindow::~MainWindow() // Hàm này được gọi khi đối tượng MainWindow bị xóa khỏi bộ nhớ,
                          // thông thường là khi chương trình đóng hoặc khi gọi hàm delete để giải phóng bộ nhớ.
{
    on_pushButton_reset_clicked();
    Serialll->close();
    delete ui;
}

void MainWindow::customize_ui()
{
    ui->lineEdit_Kp->setStyleSheet("border: 1.3px solid blue");
    ui->lineEdit_Ki->setStyleSheet("border: 1.3px solid blue");
    ui->lineEdit_Kd->setStyleSheet("border: 1.3px solid blue");
    //    ui->label_Kp->setStyleSheet("font-weight: bold; color: black");
    //    ui->label_Ki->setStyleSheet("font-weight: bold; color: black");
    //    ui->label_Kd->setStyleSheet("font-weight: bold; color: black");
    ui->PID1->setStyleSheet("font-weight: bold; color: rgb(255, 85, 167)");
    ui->PID2->setStyleSheet("font-weight: bold; color: rgb(255, 85, 167)");
    ui->sendPID->setStyleSheet("font-weight: bold; color: rgb(140, 0, 255)");
    ui->Select_COM->setStyleSheet("font-weight: bold; color: rgb(0, 0, 255)");
    ui->pushButton_disconnect->setStyleSheet("font-weight: bold; color: rgb(255, 0, 0)");
    ui->pushButton_reload->setStyleSheet("font-weight: bold; color: rgb(0, 170, 127)");
    ui->lineEdit_setpoint->setStyleSheet("border: 1.3px solid green");
    //    ui->pushButton_position->setStyleSheet("font-weight: bold; color: rgb(246, 255, 246)");
    //    ui->pushButton_velocity->setStyleSheet("font-weight: bold; color: rgb(246, 255, 246)");
    ui->lineEdit_Kp->setValidator(new QDoubleValidator);
    ui->lineEdit_Ki->setValidator(new QDoubleValidator);
    ui->lineEdit_Kd->setValidator(new QDoubleValidator);
    ui->lineEdit_setpoint->setValidator(new QDoubleValidator);
}

void MainWindow::updateCOM()
{
    ui->comboBox_COM->clear();
    ui->comboBox_Baud->clear();
    list_COM = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &port, list_COM)
    {
        ui->comboBox_COM->addItem(port.portName());
    }
    QList<qint32> baudRates = info.standardBaudRates();
    QList<QString> strBaudRates;
    for (int i = 0; i < baudRates.size(); i++)
    {
        strBaudRates.append(QString::number(baudRates.at(i)));
    }
    ui->comboBox_Baud->addItems(strBaudRates);
}

void MainWindow::on_Select_COM_clicked()
{
    select_com = ui->comboBox_COM->currentText();
    if (select_com.isEmpty())
    {
        QMessageBox::warning(this, "Can't select COM", "Check your COM, it can't be open");
    }
    select_baud = ui->comboBox_Baud->currentText();
    Serialll->setPortName(select_com);
    ui->comboBox_COM->setEnabled(false);
    //       Serialll->setBaudRate(QSerialPort::Baud115200);
    Serialll->setBaudRate(select_baud.toInt());
    ui->comboBox_Baud->setEnabled(false);
    Serialll->setDataBits(QSerialPort::Data8); // sua 8 bit 9 bit gi thi sua o day, cứng sẵn trong code cho nhanh
    Serialll->setParity(QSerialPort::NoParity);
    Serialll->setStopBits(QSerialPort::OneStop);
    Serialll->open(QIODevice::ReadWrite);
    if (Serialll->isOpen())
    {
        ui->label_status_com->setStyleSheet("color: blue;");
        ui->label_status_com->setText("CONNECTED");
        ui->textBrowser_command->append("Connected COM");
        qDebug() << "COM selected!";
    }
    else
    {
        QMessageBox::warning(this, "Can't do it", "Check your COM, it can't be open");
    }
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    if (Serialll->isOpen())
    {
        Serialll->close();
        ui->comboBox_Baud->setEnabled(true);
        ui->comboBox_COM->setEnabled(true);
        ui->label_status_com->setStyleSheet("color: red;");
        ui->label_status_com->setText("DISCONNECT");
        ui->textBrowser_command->append("Disconnect COM");
        qDebug() << "Close port oke";
    }
    else
    {
        qDebug() << "COM is not openning??";
        QMessageBox::warning(this, "Can't close", "COM is not openning");
    }
}

void MainWindow::on_pushButton_readData_clicked()
{
    checkContRead = true;
    if (Serialll->isOpen())
    {
        // connect(Serialll, &QSerialPort::readyRead, this, &MainWindow::readDataFromSTM);
        timer = new QTimer(this);
        timer->setTimerType(Qt::PreciseTimer); // higher priority
        connect(timer, SIGNAL(timeout()), this, SLOT(readDataFromSTM()));
        //        if(checkPosi == 1)
        //        {
        timer->start(20);
        //        }
        //        else if(checkVelo == 1)
        //        {
        //            timer->start(20);
        //        }

        qDebug() << "Read Data okay";
        ui->textBrowser->setTextColor(Qt::blue);
        ui->textBrowser->setText("Ready to read!");
        ui->textBrowser->setText("Data  |  Time");
    }
    else
    {
        qDebug() << "Read Data failed";
        QMessageBox::warning(this, "Can't read data", "Check your COM, it can't be open");
    }
}

void MainWindow::readDataFromSTM()
{

    if (Serialll->bytesAvailable())
    {
        qDebug() << " ";
    }
    else
    {
        qDebug() << "byte deo available";

    }
    SerialProto call_get_data;
    if (Serialll->isOpen() && (Serialll->bytesAvailable() >0) && checkContRead == true)
    {
        data_read.clear();
        data_read = Serialll->readAll();
        //        qDebug() << "data read la: " << data_read;
        call_get_data.call_get_dataFromSTM(); // readAll nay tra ve du lieu kieu Qbytearray nen phai khai bao nhu the
                                              // qDebug() << "qstring data_read[1]: " << (QString("0x%1 ").arg(data_read[1], 2, 16, QChar('0')));
        if ((rx_buff[0] == 0x66) && call_get_data.call_get_dataFromSTM() == Phuc_right)
        {
            display1++;
            if (display1 == 2 || display1 == 200) // de cho no it hien cai nay thoi
            {
                ui->textBrowser_command->append("----------------------");
                ui->textBrowser_command->append("Position");
            }
            //            QString checkmap = QString::number(check_true);
            //            if (call_get_data.call_get_dataFromSTM() >= 0)
            //            {
            //            connect(ui->pushButton_clearGraph, SIGNAL(clicked()), this, SLOT(on_pushButton_clearGraph_clicked()));
            if (checkClear != 1)
            {
                float rx_buffToFloat = *((float *)(rx_buff + 1)); // do byte dau la chon graph vi tri hay van toc nen phai +1 de bo byte do
                                                                  // all_data.append(rx_buffToFloat);
                                                                  // qDebug() << "all_data la: " << all_data;
                ui->textBrowser->setTextColor(Qt::black);
                QString data = QString::number(rx_buffToFloat);
                QDateTime local_time(QDateTime::currentDateTime());
                QString local_timeToString = local_time.toString("yyyy-MM-dd HH:mm:ss.zzz");
                QString data_timenow = data + "   " + local_timeToString;
                ui->textBrowser->append(data_timenow);
                //                all_data.append(data + " ");
                //                qDebug() << "all_data la: " << all_data;
                //                qDebug() << "size cua all_data la: " << all_data.size();
                //                counterPosi++;
                //                values = QString(all_data).split(" ");
                //                qDebug() << "size cua values la: " << values.size();
                //                for (int i = 0; i < values.size(); i++)
                //                {
                //                    value = values[i].toDouble();
                //                    dataPoints.append(value);
                //                }
                //                if (counterPosi = 1)
                //                {
                //
                //----
                if (checkBtnPlot2 == 1)
                {
                    qDebug() << "checkBtnPlot2 la: " << checkBtnPlot2;
                    qDebug() << "size cua values 2 la: " << values.size();
                    while (values.size() >= 800)
                    {
                        values.removeFirst();
                        counterRealTime++;
                    }
                }
                values.append(data);
                //                qDebug() << "values la: " << values;
                //                value = values[jValues++].toDouble();
                //                qDebug() << "jvalues la: " << jValues;
                //                dataPoints.append(value);
                //                for (int i = 0; i < values.size(); i++)
                //                {
                //                    value = values[i].toDouble();
                //                    dataPoints.append(value);
                //                }
                //                qDebug() << "dataPoints la: " << dataPoints;
                //----
                //                qDebug() << "values la: " << values;

                //                    qDebug() << "dataPoints la: " << dataPoints;
                //                }
            }
        }

        //       else if ((QString("0x%1").arg(data_read[1], 2, 16, QChar('0'))) == "0x77" && call_get_data.call_get_dataFromSTM() == Phuc_right)
        else if ((rx_buff[0] == 0x77) && call_get_data.call_get_dataFromSTM() == Phuc_right)
        {
            display2++;
            if (display2 == 2 || display2 == 200)
            {
                ui->textBrowser_command->append("----------------------");
                ui->textBrowser_command->append("Velocity");
            }
            //            QString checkmap = QString::number(check_true);
            //            ui->textBrowser->append(checkmap);
            //            if (call_get_data.call_get_dataFromSTM() >= 0)
            //            {
            //            connect(ui->pushButton_clearGraph, SIGNAL(clicked()), this, SLOT(on_pushButton_clearGraph_clicked()));
            if (checkClear != 1)
            {
                float rx_buffToFloat = *((float *)(rx_buff + 1)); // do byte dau la chon graph vi tri hay van toc nen phai +1 de bo byte do
                                                                  //         all_data.append(rx_buffToFloat);
                                                                  //        qDebug() << "all_data la: " << all_data;
                ui->textBrowser->setTextColor(Qt::black);
                QString data = QString::number(rx_buffToFloat);
                QDateTime local_time(QDateTime::currentDateTime());
                QString local_timeToString = local_time.toString("yyyy-MM-dd HH:mm:ss.zzz");
                QString data_timenow = data + "   " + local_timeToString;
                ui->textBrowser->append(data_timenow);
                //                all_data.append(data + " ");
                if (checkBtnPlot2 == 1)
                {
                    qDebug() << "checkBtnPlot2 la: " << checkBtnPlot2;
                    qDebug() << "size cua values 2 la: " << values.size();
                    while (values.size() >= 400)
                    {
                        values.removeFirst();
                        counterRealTime++;
                    }
                }
                values.append(data);
            }
            //       qDebug() << "all_data la: " << all_data;
        }
        // else if (call_get_data.call_get_dataFromSTM() == Phuc_no_valid)
        // {
        //     ui->textBrowser_command->append("No valid message found");
        // }
        else if (call_get_data.call_get_dataFromSTM() == Phuc_false_CRC)
        {
            ui->textBrowser_command->append("Invalid CRC");
        }
        else if ((call_get_data.call_get_dataFromSTM() == Phuc_buffer_small))
        {
            ui->textBrowser_command->append("Destination buffer too small");
        }
        else if ((call_get_data.call_get_dataFromSTM() == Phuc_false_lenght_data))
        {
            ui->textBrowser_command->append("False lenght data");
        }
    }

    else
    {
        qDebug() << "Check read data from STM";
    }
}

int8_t SerialProto::call_get_dataFromSTM()
{
    SerialProto get_data;
       // qDebug() << "data data_read la: " << data_read.data();
       // qDebug() << "data_read la: " << data_read;
       // qDebug() << QString("dataread[0] = 0x%1").arg(data_read[0], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[1] = 0x%1").arg(data_read[1], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[2] = 0x%1").arg(data_read[2], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[3] = 0x%1").arg(data_read[3], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[4] = 0x%1").arg(data_read[4], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[5] = 0x%1").arg(data_read[5], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[6] = 0x%1").arg(data_read[6], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[7] = 0x%1").arg(data_read[7], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[8] = 0x%*/1").arg(data_read[8], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[9] = 0x%1").arg(data_read[9], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[10] = 0x%1").arg(data_read[10], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[11] = 0x%1").arg(data_read[11], 2, 16, QChar('0'));
       // qDebug() << QString("dataread[12] = 0x%1").arg(data_read[12], 2, 16, QChar('0'));
    //    qDebug() << QString("dataread[13] = 0x%1").arg(data_read[13], 2, 16, QChar('0'));
    //    qDebug() << "size of data_Read" << data_read.size();
    //    qDebug() << QString("rx_buff[0] = 0x%1").arg(data_read[0], 2, 16, QChar('0'));
    check_true = get_data.serialGetData(reinterpret_cast<uint8_t *>(data_read.data()), data_read.size(), rx_buff, &rx_buff_size); // rx_buff se chua data la 0x66/0x77 + now_position/velocity_real
                                                                                                                                  //    qDebug() << "rx buff size la: " << rx_buff_size;
                                                                                                                                  //    for (int i = 0; i < sizeof(rx_buff); i++)
                                                                                                                                  //    {
                                                                                                                                  //        qDebug() << "rx_buff la: " << rx_buff[i];
                                                                                                                                  //    }
    return check_true;
}
void SerialProto::call_create_frameQt()
{
    SerialProto frame_data;
    // frameBuffLenght = byteArray.length() + 4;
    for (int i = 0; i < sizeof(frameBuffer); i++)
    {
        frameBuffer[i] = 0x00;
    }
    frame_data.serialFrameData(reinterpret_cast<uint8_t *>(byteArray.data()), byteArray.length(), frameBuffer, &frameBuffLenght);
}

void MainWindow::updateGraph()
{
    if (checkVelo == 1)
    {
        ui->customPlot1->xAxis->setLabel("Sample Size");
        ui->customPlot1->yAxis->setLabel("Velocity (degrees/min)");
    }
    if (checkPosi == 1)
    {
        ui->customPlot1->xAxis->setLabel("Sample Size");
        ui->customPlot1->yAxis->setLabel("Position (degrees)");
    }
    static QVector<double> x;
    if (checkBtnPlot1 == 1)
    {
        checkBtnPlot2 = 0;

        // ------------------------------------------------------------------------------ real -----------------------
        // Add new data
        //       values = QString(all_data).split(" ");

        checkContinue++;
        if (checkFirst == 0)
        {
            checkFirst = 1;

            for (int i = 0; i < values.size(); i++)
            {
                value = values[i].toDouble();
                dataPoints.append(value);
            }
            checkSizeValues = values.size();
            dataPointsVector = QVector<double>::fromList(dataPoints);
            //        dataPointsVector.removeLast();
            //        static QVector<double> x(dataPointsVector.size());
            x.resize(dataPointsVector.size());
            qDebug() << "Size cua dataPointsVector la: " << dataPointsVector.size();
            qDebug() << "Data cua dataPointsVector la: " << dataPointsVector.data();
            //   connect(ui->pushButton_clearGraph, SIGNAL(clicked()), this, SLOT(on_pushButton_clearGraph_clicked()));
            //        QElapsedTimer timer_elapse;
            //        timer_elapse.start();
            checkSizeDataVector = dataPointsVector.size();
            for (qint64 i = 0; i < dataPointsVector.size(); i++)
            {
                //            x[i] = timer_elapse.elapsed() + i*0.01;
                //    x[i] = time.elapsed()/1000.0 + i * 0.01; // elapsed la thoi gian da troi qua tu khi timer dc start , cong i*0.05 la cong don tich luy may cai thoi gian cũ thoi
                x[i] = i;
                //            if (dataPointsVector.size() > 1500)
                //            {
                //                timer->stop();
                //            }
            }
            //         qDebug() << "checkSizeValues(top) la: " << checkSizeValues;
            //         qDebug() << "dataPointsVector top la: " << dataPointsVector;
        }
        if (checkContinue >= 2)
        {
            //            qDebug() << "checkContinue la: " << checkContinue;
            //            qDebug() << "checkSizeValues(bottom) la: " << checkSizeValues;
            //            qDebug() << "values.size()(bottom) - checkSizeValues la : " << values.size() - checkSizeValues;
            checkContinue = 2;
            for (int i = checkSizeValues; i < (values.size()); i++)
            {
                value = values[i].toDouble();
                dataPoints.append(value);
            }
            checkSizeValues = values.size();
            dataPointsVector = QVector<double>::fromList(dataPoints);
            //            dataPointsVector.removeLast();
            //            qDebug() << "checkSizeDataVector la: " << checkSizeDataVector;
            //            qDebug() << "dataPointsVector.size()(bottom) - checkSizeDataVector la: " << dataPointsVector.size() - checkSizeDataVector;
            x.resize(dataPointsVector.size());
            for (qint64 i = checkSizeDataVector; i < (dataPointsVector.size()); i++)
            {
                x[i] = i;
                if (dataPointsVector.size() > 1000)
                {
                    timer->stop();
                    checkContRead = false;
                    checkBtnPlot1 = 0;
                }
            }
            checkSizeDataVector = dataPointsVector.size();
        }
        // test ----------
        //        static QTime time(QTime::currentTime());
        //        double key = time.elapsed()/1000.0;
        //        static double lastPointKey = 0;
        //        if(key - lastPointKey > 0.01)
        //        {
        //            x.append(key);
        //            lastPointKey = key;
        //        }
        //        ui->customPlot1->graph(0)->setData(x, dataPointsVector);
        // test ----------
        //       qDebug() << "dataPointsVector khi ve la: " << dataPointsVector;
        ui->customPlot1->graph(0)->addData(x, dataPointsVector);
        ui->customPlot1->graph(1)->setData(x, setPointValue);
        //        ui->customPlot1->xAxis->setRange(0,100);
        //        ui->customPlot1->yAxis->setRange(-100,100);
        // Rescale and redraw graph
        ui->customPlot1->rescaleAxes();
        ui->customPlot1->replot();
        ui->customPlot1->update();
        //       dataPoints.clear();
        //      ui->customPlot1->graph(0)->data()->clear();
    }
}
// ------------------------------------------------------------------------------ real ---------

void MainWindow::updateGraph_realTime()
{
    if (btnResetClick == true)
    {
        timer_realTime->stop();
        ui->customPlot2->replot();
        ui->customPlot2->update();
        checkBtnPlot2 = 0;
    }
    if (checkVelo == 1)
    {
        ui->customPlot2->xAxis->setLabel("Sample Size");
        ui->customPlot2->yAxis->setLabel("Velocity (degrees/min)");
    }
    if (checkPosi == 1)
    {
        ui->customPlot2->xAxis->setLabel("Sample Size");
        ui->customPlot2->yAxis->setLabel("Position (degrees)");
    }
    if (checkBtnPlot2 == 1)
    {
        checkBtnPlot1 = 0;
        static QVector<double> x2;
        qDebug() << "Values real time la: " << values;
        // ---

        for (int i = 0; i < values.size(); i++)
        {
            value = values[i].toDouble();
            dataPoints.append(value);
        }
        //        qDebug() << "dataPoints real time la: " << dataPoints;
        //        qDebug() << "size dataPoints2 la: " << dataPoints.size();
        dataPointsVector = QVector<double>::fromList(dataPoints);
        //        qDebug() << "dataPointsVector real time la: " << dataPointsVector;
        x2.resize(dataPointsVector.size());
        for (qint64 i = 0; i < dataPointsVector.size(); i++)
        {
            x2[i] = i + counterRealTime;
        }
        ui->customPlot2->graph(0)->addData(x2, dataPointsVector);
        ui->customPlot2->graph(1)->setData(x2, setPointValue);
        //        ui->customPlot2->yAxis2->setRange(ui->lineEdit_setpoint->text().toDouble() - 80, ui->lineEdit_setpoint->text().toDouble() + 80);
        //        ui->customPlot2->xAxis2->rescale();
        if (checkPosi == 1)
        {
            ui->customPlot2->xAxis->rescale();
            if (setPointPosi <= 500 && setPointPosi >= -500)
            {
                ui->customPlot2->yAxis->setRange(setPointPosi - 200, setPointPosi + 200);
            }
            else
            {
                ui->customPlot2->yAxis->setRange(setPointPosi - 800, setPointPosi + 800);
            }
        }
        if (checkVelo == 1)
        {
            ui->customPlot2->xAxis->rescale();
            ui->customPlot2->yAxis->setRange(0, 350);
        }
        ui->customPlot2->replot();
        ui->customPlot2->update();
        //        if(btnResetClick == false)
        //        {
        dataPoints.clear();
        ui->customPlot2->graph(0)->data()->clear();
        //        }

        //        qDebug() << "Size cua dataPointsVector2 la: %d" << dataPointsVector.size();
    }
}

void MainWindow::on_sendPID_clicked()
{
    float Kp = ui->lineEdit_Kp->text().toFloat(); // 1.568 --> 4 byte : Kp = 4 byte
    float Ki = ui->lineEdit_Ki->text().toFloat();
    float Kd = ui->lineEdit_Kd->text().toFloat();
    char choose_PID = 0x11;
    byteArray.clear();
    byteArray.append(choose_PID);
    byteArray.append(reinterpret_cast<const char *>(&Kp), sizeof(Kp));
    byteArray.append(reinterpret_cast<const char *>(&Ki), sizeof(Ki));
    byteArray.append(reinterpret_cast<const char *>(&Kd), sizeof(Kd));
    // test ------------------
    //    byteArray.clear();
    //    byteArray.append(0x53);
    //    byteArray.append(0x02);
    //    byteArray.append(0x42);
    //    byteArray.append(0x03);
    //    byteArray.append(0x05);
    //  ----------------------
    SerialProto frame_call;
    frame_call.call_create_frameQt();
    QByteArray send_frame = QByteArray((char *)frameBuffer, frameBuffLenght);
    qDebug() << "send_frame tao frame thanh: " << send_frame;
    //    qDebug() << QString("send_frame[0] = 0x%1").arg(send_frame[0], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[1] = 0x%1").arg(send_frame[1], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[2] = 0x%1").arg(send_frame[2], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[3] = 0x%1").arg(send_frame[3], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[4] = 0x%1").arg(send_frame[4], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[5] = 0x%1").arg(send_frame[5], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[6] = 0x%1").arg(send_frame[6], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[7] = 0x%1").arg(send_frame[7], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[8] = 0x%1").arg(send_frame[8], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[9] = 0x%1").arg(send_frame[9], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[10] = 0x%1").arg(send_frame[10], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[11] = 0x%1").arg(send_frame[11], 2, 16, QChar('0'));
    //    qDebug() << QString("send_frame[12] = 0x%1").arg(send_frame[12], 2, 16, QChar('0'));
    qDebug() << "do dai frame tu ham` la: " << frameBuffLenght;
    if (Serialll->isOpen())
    {
        Serialll->write(send_frame);
        ui->textBrowser_command->append("Kp: " + QString::number(Kp) + ", Ki: " + QString::number(Ki) + ", Kd: " + QString::number(Kd));
        qDebug() << "Send PID ok";
    }
    else
    {
        qDebug() << "Can't send PID";
        QMessageBox::warning(this, "Can't send PID", "Check your COM, it can't be open");
    }
}

void MainWindow::on_pushButton_clear_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_pushButton_position_clicked()
{
    checkPosi = 1;
    checkVelo = 0;
    //    checkRescale = true;
    setPointValue.fill(0);
    double setPoint = ui->lineEdit_setpoint->text().toDouble();
    setPointValue.fill(setPoint);
    setPointPosi = ui->lineEdit_setpoint->text().toFloat();
    float add_play = 0; // add choi de tao thanh 12 byte thoi :))
    char choose_posi = 0x22;
    byteArray.clear();
    byteArray.append(choose_posi);
    byteArray.append(reinterpret_cast<const char *>(&setPointPosi), sizeof(setPointPosi));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    SerialProto frame_call;
    frame_call.call_create_frameQt();
    QByteArray send_frame = QByteArray((char *)frameBuffer, frameBuffLenght);
    if (Serialll->isOpen())
    {
        Serialll->write(send_frame);
        ui->textBrowser_command->append("SetPoint Posi: " + QString::number(setPoint));
        qDebug() << "Send Position ok";
    }
    else
    {
        qDebug() << "Can't send posi ";
        QMessageBox::warning(this, "Can't take position", "Check your COM, it can't be open");
    }
}

void MainWindow::on_pushButton_velocity_clicked()
{
    checkPosi = 0;
    checkVelo = 1;
    //    checkRescale = true;
    setPointValue.fill(0);
    double setPoint = ui->lineEdit_setpoint->text().toDouble();
    setPointValue.fill(setPoint);
    float setpoint2 = ui->lineEdit_setpoint->text().toFloat();
    float add_play = 0; // add choi de tao thanh 12 byte thoi :))
    char choose_velo = 0x33;
    byteArray.clear();
    byteArray.append(choose_velo);
    byteArray.append(reinterpret_cast<const char *>(&setpoint2), sizeof(setpoint2));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    SerialProto frame_call;
    frame_call.call_create_frameQt();
    QByteArray send_frame = QByteArray((char *)frameBuffer, frameBuffLenght);
    if (Serialll->isOpen())
    {
        Serialll->write(send_frame);
        ui->textBrowser_command->append("SetPoint Velo: " + QString::number(setPoint));
        qDebug() << "Done send velo";
    }
    else
    {
        qDebug() << "Can't send velo ";
        QMessageBox::warning(this, "Can't take velocity", "Check your COM, it's can't be open");
    }
}

void MainWindow::on_pushButton_reset_clicked()
{
    if (checkBtnPlot1 == 1)
    {
        timer->stop();
        checkBtnPlot1 = 0;
    }
    if (checkBtnPlot2 == 1)
    {
        timer_realTime->stop();
        checkBtnPlot2 = 0;
    }
    checkContRead = false;
    btnResetClick = true;
    float add_play = 0; // add choi de tao thanh 12 byte thoi :))
    char choose_reset = 0x44;
    byteArray.clear();
    byteArray.append(choose_reset);
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    SerialProto frame_call;
    frame_call.call_create_frameQt();
    QByteArray send_frame = QByteArray((char *)frameBuffer, frameBuffLenght);
    Serialll->write(send_frame);
    QMessageBox::information(this, "Exit", "Done");
}

void MainWindow::on_pushButton_run_clicked()
{
    checkContRead = true;
    btnResetClick = false;
    if (Serialll->isOpen())
    {
        float add_play = 0; // add choi de tao thanh 12 byte thoi :))
        char choose_run = 0x55;
        byteArray.clear();
        byteArray.append(choose_run);
        byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
        byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
        byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
        SerialProto frame_call;
        frame_call.call_create_frameQt();
        QByteArray send_frame = QByteArray((char *)frameBuffer, frameBuffLenght);
        Serialll->write(send_frame);
    }
    else if ((ui->lineEdit_Kd->text().isEmpty() && ui->lineEdit_Ki->text().isEmpty() && ui->lineEdit_Kp->text().isEmpty()) || ui->lineEdit_setpoint->text().isEmpty())
    {
        QMessageBox::warning(this, "Can't RUN", "Check if you have enter data of PID or setpoint already?");
    }
    else
    {
        QMessageBox::warning(this, "Can't RUN", "Check your COM, it can't be open");
    }
}

void MainWindow::on_pushButton_plot_clicked()
{ /* NOTE: DO DANG DUNG CHUNG DATAPOINTSVECTOR NEN KHI VE XONG GRAPH REALTIME OR LIMIT TIME THI KHI NHAN VE GRAPH KHAC THI SE GẶP DỮ LIỆU CŨ CỦA
  DATAPOINTSVECTOR NEN NÓ TỰ VẼ LẠI LUON */
    checkClear = 0;
    checkBtnPlot1 = 1;
    ui->customPlot1->addGraph();                          // tao them 1 1
    ui->customPlot1->graph(0)->setPen(QPen(Qt::blue, 2)); // Màu đường line vs độ dày 2 pixel
                                                          //    ui->customPlot1->xAxis->setLabel("Time (s)");
    //        // thiet lap doan hien thi tren x, y
    ui->customPlot1->xAxis->setRange(0, 200);
    ui->customPlot1->yAxis->setRange(-100, 100);
    ui->customPlot1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->customPlot1->addGraph();
    ui->customPlot1->graph(1)->setPen(QPen(Qt::red, 2));
    timer = new QTimer(this);
    timer->setTimerType(Qt::CoarseTimer); // priority lower
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGraph()));
    //    timer->start(22); // set thoi gian ve 30ms, lay mau la 10ms nhung ve la 30ms =))
    //    if(checkVelo == 1)
    //    {
    //        timer->start(42);
    //    }
    //    if(checkPosi == 1)
    //    {
    timer->start(22); // set thoi gian ve 30ms, lay mau la 10ms nhung ve la 30ms =))
    //    }
    //    else
    //    {
    //        timer_realTime->start(32);
    //    }
}

void MainWindow::on_pushButton_clearGraph_clicked()
{
    checkClear = 1;
    //    timer->start(0);
    //    timer->stop();
    //    time.elapsed(); // new add 8/4, cho thoi gian dung lai, tra lai gia tri thoi gian va set lai thoi gian = 0 luon
    //    dataPoints.clear();
    //    //   ui->customPlot1->clearGraphs();
    //    ui->customPlot1->clearGraphs(); // new add 8/4
    //    ui->customPlot1->clearItems();
    //    ui->customPlot1->graph(0)->data()->clear();
    //    ui->customPlot1->replot();
    // //   ui->customPlot1->update(); // new add 8/4
}

void MainWindow::on_pushButton_plot_2_clicked()
{
    checkClear = 0;
    checkBtnPlot2 = 1;
    ui->customPlot2->addGraph();                           // tao them 1 line
    ui->customPlot2->graph(0)->setPen(QPen(Qt::green, 2)); // Màu đường line vs độ dày 2 pixel
    //        // thiet lap doan hien thi tren x, y
    //    ui->customPlot2->xAxis->setRange(0, 1000);
    //   ui->customPlot1->yAxis->setRange(-100, 100);
    ui->customPlot2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->customPlot2->addGraph();
    ui->customPlot2->graph(1)->setPen(QPen(Qt::red, 2));
    timer_realTime = new QTimer(this);
    timer_realTime->setTimerType(Qt::CoarseTimer);
    connect(timer_realTime, SIGNAL(timeout()), this, SLOT(updateGraph_realTime()));
    //    if(checkVelo == 1)
    //    {
    //        timer_realTime->start(42);
    //    }
    //    else if(checkPosi == 1)
    //    {
    timer_realTime->start(22); // set thoi gian ve 30ms, lay mau la 10ms nhung ve la 30ms =))
    //    }
    //    else
    //    {
    //        timer_realTime->start(32);
    //    }
}

void MainWindow::on_pushButton_reload_clicked()
{
    updateCOM();
    ui->textBrowser_command->append("Done Reload");
}

void MainWindow::on_pushButton_clear_2_clicked()
{
    ui->textBrowser_command->clear();
}

void MainWindow::on_actionSave_txt_triggered()
{
    QString fileGraphLimit = "D:\\Qt Creator\\againSerialPort\\file saved\\graph_limit.jpg";
    QString fileGraphRealTime = "D:\\Qt Creator\\againSerialPort\\file saved\\graph_realTime.jpg";
    QFile fileGraph1(fileGraphLimit);
    QFile fileGraph2(fileGraphRealTime);
    if (fileGraph1.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (checkBtnPlot1 == 1)
        {
            ui->customPlot1->saveJpg(fileGraphLimit);
            qDebug() << "Done save file graph limit!";
        }
    }
    if (fileGraph2.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (checkBtnPlot2 == 1)
        {
            ui->customPlot2->saveJpg(fileGraphRealTime);
            qDebug() << "Done save file real time!";
        }
    }
    else
    {
        qDebug() << "Can't save file graph!";
    }
    fileGraph1.close();
    fileGraph2.close();
    QString fileTxt = "D:\\Qt Creator\\againSerialPort\\file saved\\data.txt";
    QFile fileData(fileTxt);

    if (fileData.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&fileData);
        QString text = ui->textBrowser->toPlainText();
        stream << text << Qt::endl;
        qDebug() << "Done save file txt!";
    }
    else
    {
        qDebug() << "Can't save file txt!";
    }

    fileData.close();
    QMessageBox::warning(this, "Save file", "Done saved!");
}

void MainWindow::on_PID1_clicked()
{
    float add_play = 0; // add choi de tao thanh 12 byte thoi :))
    char choose_PID = 0x88;
    float PID1 = 1;
    byteArray.clear();
    byteArray.append(choose_PID);
    byteArray.append(reinterpret_cast<const char *>(&PID1), sizeof(PID1));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    SerialProto frame_call;
    frame_call.call_create_frameQt();
    QByteArray send_frame = QByteArray((char *)frameBuffer, frameBuffLenght);
    if (Serialll->isOpen())
    {
        Serialll->write(send_frame);
        ui->textBrowser_command->append("Choose PID1");
        qDebug() << "Done choose PID1";
    }
    else
    {
        qDebug() << "Can't choose PID1";
        QMessageBox::warning(this, "Can't choose PID1", "Check your COM, it's can't be open");
    }
}

void MainWindow::on_PID2_clicked()
{
    float add_play = 0; // add choi de tao thanh 12 byte thoi :))
    char choose_PID = 0x88;
    float PID2 = 2;
    byteArray.clear();
    byteArray.append(choose_PID);
    byteArray.append(reinterpret_cast<const char *>(&PID2), sizeof(PID2));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    byteArray.append(reinterpret_cast<const char *>(&add_play), sizeof(add_play));
    SerialProto frame_call;
    frame_call.call_create_frameQt();
    QByteArray send_frame = QByteArray((char *)frameBuffer, frameBuffLenght);
    if (Serialll->isOpen())
    {
        Serialll->write(send_frame);
        ui->textBrowser_command->append("Choose PID2");
        qDebug() << "Done choose PID2";
    }
    else
    {
        qDebug() << "Can't choose PID2";
        QMessageBox::warning(this, "Can't choose PID2", "Check your COM, it's can't be open");
    }
}
