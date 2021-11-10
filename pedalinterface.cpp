#include "pedalinterface.h"
#include "ui_pedalinterface.h"
#include <QSerialPortInfo>
#include <QThread>
#include <iostream>

PedalInterface::PedalInterface(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PedalInterface)
{
    ui->setupUi(this);
    pedalPort = new QSerialPort(this);
    uint vid, pid;
    while(pedalPort->portName() == NULL){
        const auto infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &info : infos) {
            vid = info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16).toInt() : 0;
            pid = info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16).toInt() : 0;
            if (vid == 6969 && pid == 2)  {
                pedalPort->setPortName(info.portName());
                pedalPort->setBaudRate(2000000);
                pedalPort->setDataBits(QSerialPort::Data8);
                pedalPort->setParity(QSerialPort::NoParity);
                pedalPort->setStopBits(QSerialPort::OneStop);
                pedalPort->setFlowControl(QSerialPort::NoFlowControl);
                pedalPort->open(QIODevice::ReadWrite);
            }
        }
    }
    pedalPort->write("da0");
    //    QObject::connect(pedalPort, SIGNAL(readyRead()), this, SLOT(readSerial()));
}

PedalInterface::~PedalInterface()
{
    delete ui;
    pedalPort->close();
}

bool PedalInterface::readSerial(){
    QString message;
    while(!pedalPort->waitForReadyRead()){}
    message = pedalPort->readAll();
    if (message.startsWith("Lower") || message.startsWith("Upper")) {
        //        std::cout << message.toStdString() << std::endl;
        QStringList messageList = message.split(" ");
        char level = messageList.at(0).at(0).toLatin1(), selection = messageList.at(1).at(0).toLatin1();
        double value;
        try {
            value = std::stoi(messageList.at(3).toStdString());
            if((int)value > 0){
                switch (selection) {
                case 'c':
                    clutchLimits[(level == 'L' ? 0 : 1)] += clutchLimits[(level == 'L' ? 0 : 1)] == -1 ? ((int)value + 1) : 0;
//                    std::cout << (int)value << std::endl;
                    //                clutchLimitsSetting[(level == 'L' ? 0 : 1)] = (int)value;
                    break;
                case 'b':
                    brakeLimits[(level == 'L' ? 0 : 1)] +=  brakeLimits[(level == 'L' ? 0 : 1)] == -1 ? (value + 1) : 0;
                    //                brakeLimitsSetting[(level == 'L' ? 0 : 1)] = value;
                    break;
                case 'g':
                    gasLimits[(level == 'L' ? 0 : 1)] += gasLimits[(level == 'L' ? 0 : 1)] == -1 ? ((int)value + 1) : 0;
                    //                gasLimitsSetting[(level == 'L' ? 0 : 1)] = (int)value;
                    break;
                default:
                    break;
                }
            }
            updateUI();
            return true;
        }  catch (const std::exception& e) {
//            std::cout << e.what() << std::endl;
            return false;
        }
    } else {
        if (message.startsWith("Set")) {
//            std::cout << message.toStdString() << std::endl;
            return true;
        }
    }
}



void PedalInterface::on_ReadValuesButton_clicked()
{
//    std::cout << "/////////////////////////////////" << std::endl;
//    std::cout << clutchLimitsSetting[0] << " ";// << std::endl;
//    std::cout << clutchLimitsSetting[1] << std::endl;
//    std::cout << brakeLimitsSetting[0] << " ";// << std::endl;
//    std::cout << brakeLimitsSetting[1] << std::endl;
//    std::cout << gasLimitsSetting[0] << " ";// << std::endl;
//    std::cout << gasLimitsSetting[1] << std::endl;
//    std::cout << "/////////////////////////////////" << std::endl;
    resetInterfaceLimits();
    while (checkLimits()) {
        if(clutchLimits[0] == -1){pedalPort->write("dcl");
        readSerial();}
        if(clutchLimits[1] == -1){pedalPort->write("dch");
        readSerial();}
        if(brakeLimits[0] == -1){pedalPort->write("dbl");
        readSerial();}
        if(brakeLimits[1] == -1){pedalPort->write("dbh");
        readSerial();}
        if(gasLimits[0] == -1){pedalPort->write("dgl");
        readSerial();}
        if(gasLimits[1] == -1){pedalPort->write("dgh");
        readSerial();}
    }
    clutchLimitsSetting[1] = clutchLimits[1];
    brakeLimitsSetting[1]  = brakeLimits[1];
    gasLimitsSetting[1]    = gasLimits[1];
    updateUI();
    clutchLimitsSetting[0] = clutchLimits[0];
    brakeLimitsSetting[0]  = brakeLimits[0];
    gasLimitsSetting[0]    = gasLimits[0];
    updateUI();
//    std::cout << clutchLimitsSetting[0] << " ";// << std::endl;
//    std::cout << clutchLimitsSetting[1] << std::endl;
//    std::cout << brakeLimitsSetting[0] << " ";// << std::endl;
//    std::cout << brakeLimitsSetting[1] << std::endl;
//    std::cout << gasLimitsSetting[0] << " ";// << std::endl;
//    std::cout << gasLimitsSetting[1] << std::endl;
//    std::cout << "/////////////////////////////////" << std::endl;
}


void PedalInterface::updateUI() {
    ui->ClutchLowSpinBox_Read->setValue(clutchLimits[0]);
    ui->ClutchHighSpinBox_Read->setValue(clutchLimits[1]);
    ui->BrakeLowSpinBox_Read->setValue(brakeLimits[0]);
    ui->BrakeHighSpinBox_Read->setValue(brakeLimits[1]);
    ui->GasLowSpinBox_Read->setValue(gasLimits[0]);
    ui->GasHighSpinBox_Read->setValue(gasLimits[1]);
    ui->ClutchLowSpinBox->setValue(clutchLimitsSetting[0]);
    ui->ClutchHighSpinBox->setValue(clutchLimitsSetting[1]);
    ui->BrakeLowSpinBox->setValue(brakeLimitsSetting[0]);
    ui->BrakeHighSpinBox->setValue(brakeLimitsSetting[1]);
    ui->GasLowSpinBox->setValue(gasLimitsSetting[0]);
    ui->GasHighSpinBox->setValue(gasLimitsSetting[1]);
}


bool PedalInterface::checkLimits() {
    if(clutchLimits[0] == -1 || clutchLimits[1] == -1 ||
       gasLimits[0] == -1 || gasLimits[1] == -1 ||
       brakeLimits[0] == -1 || brakeLimits[1] == -1)
    {
//        std::cout << "still checking limits" << std::endl;
        return true;
    }
    else return false;
}

void PedalInterface::on_SetValuesButton_clicked()
{
//        std::cout << clutchLimitsSetting[0] << " ";// << std::endl;
//        std::cout << clutchLimitsSetting[1] << std::endl;
//        std::cout << brakeLimitsSetting[0] << " ";// << std::endl;
//        std::cout << brakeLimitsSetting[1] << std::endl;
//        std::cout << gasLimitsSetting[0] << " ";// << std::endl;
//        std::cout << gasLimitsSetting[1] << std::endl;
    pedalPort->write(("cl" + std::to_string(clutchLimitsSetting[0])).c_str());
    readSerial();
    pedalPort->write(("ch" + std::to_string(clutchLimitsSetting[1])).c_str());
    readSerial();
    pedalPort->write(("bl" + std::to_string(brakeLimitsSetting[0])).c_str());
    readSerial();
    pedalPort->write(("bh" + std::to_string(brakeLimitsSetting[1])).c_str());
    readSerial();
    pedalPort->write(("gl" + std::to_string(gasLimitsSetting[0])).c_str());
    readSerial();
    pedalPort->write(("gh" + std::to_string(gasLimitsSetting[1])).c_str());
    readSerial();
    on_ReadValuesButton_clicked();
    updateUI();
}


void PedalInterface::resetInterfaceLimits() {
    clutchLimits[0] = -1;
    clutchLimits[1] = -1;
    brakeLimits[0] = -1;
    brakeLimits[1] = -1;
    gasLimits[0] = -1;
    gasLimits[1] = -1;
}

void PedalInterface::on_ClutchLowSlider_valueChanged(int value)
{
    if (ui->ClutchHighSlider->value() < value) ui->ClutchHighSlider->setValue(value);
    clutchLimitsSetting[0] = value;
}

void PedalInterface::on_ClutchHighSlider_valueChanged(int value)
{
//    if (ui->ClutchLowSlider->value() > value) ui->ClutchLowSlider->setValue(value);
    clutchLimitsSetting[1] = value;
}

void PedalInterface::on_BrakeLowSlider_valueChanged(int value)
{
//    if (ui->BrakeHighSlider->value() < value) ui->BrakeHighSlider->setValue(value);
    brakeLimitsSetting[0] = value;
}

void PedalInterface::on_BrakeHighSlider_valueChanged(int value)
{
//    if (ui->BrakeLowSlider->value() > value) ui->BrakeLowSlider->setValue(value);
    brakeLimitsSetting[1] = value;
}

void PedalInterface::on_GasLowSlider_valueChanged(int value)
{
//    if (ui->GasHighSlider->value() < value) ui->GasHighSlider->setValue(value);
    gasLimitsSetting[0] = value;
}

void PedalInterface::on_GasHighSlider_valueChanged(int value)
{
//    if (ui->GasLowSlider->value() > value) ui->GasLowSlider->setValue(value);
    gasLimitsSetting[1] = value;
}

void PedalInterface::on_ClutchLowSpinBox_valueChanged(int arg1)
{
    clutchLimitsSetting[0] = arg1;
}

void PedalInterface::on_ClutchHighSpinBox_valueChanged(int arg1)
{
    clutchLimitsSetting[1] = arg1;
}

void PedalInterface::on_BrakeLowSpinBox_valueChanged(int arg1)
{
    brakeLimitsSetting[0] = arg1;
}

void PedalInterface::on_BrakeHighSpinBox_valueChanged(int arg1)
{
    brakeLimitsSetting[1] = arg1;
}

void PedalInterface::on_GasLowSpinBox_valueChanged(int arg1)
{
    gasLimitsSetting[0] = arg1;
}

void PedalInterface::on_GasHighSpinBox_valueChanged(int arg1)
{
    gasLimitsSetting[1] = arg1;
}

