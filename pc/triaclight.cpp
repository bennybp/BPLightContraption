/*! \file
 *  \brief     Main Qt GUI class
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#include "triaclight.h"
#include "commands-text.h"
#include "microcont.h"
#include "ui_triaclight.h"

#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QDebug>
#include <QTimer>
#include <QStandardItemModel>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
//QT_USE_NAMESPACE_SERIALPORT


BPLightContraption::BPLightContraption(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::BPLightContraption),mc(new MCInterface)
{
    ui->setupUi(this);

    int count = 0;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->serialPortCombo->addItem(info.portName());
        QString tooltip("Manufacturer: ");
        if(info.manufacturer() != "")
            tooltip.append(info.manufacturer());
        else
            tooltip.append("None");

        tooltip.append("\nDescription: ");

        if(info.description() != "")
            tooltip.append(info.description());
        else
            tooltip.append("None");
        ui->serialPortCombo->setItemData(count++, tooltip, Qt::ToolTipRole);
    }


    pus.push_back(QSharedPointer<PUInterfaceGUI>(new PUInterfaceGUI(PU_LIGHT1, ConvertPUID(PU_LIGHT1), mc, this)));
    pus.push_back(QSharedPointer<PUInterfaceGUI>(new PUInterfaceGUI(PU_LIGHT2, ConvertPUID(PU_LIGHT2), mc, this)));
    pus.push_back(QSharedPointer<PUInterfaceGUI>(new PUInterfaceGUI(PU_RECEPTACLE, ConvertPUID(PU_RECEPTACLE), mc, this)));

    pus[0]->AttachToGui(ui->l1label, ui->buttonl1_on, ui->buttonl1_off, ui->l1levelslider);
    pus[1]->AttachToGui(ui->l2label, ui->buttonl2_on, ui->buttonl2_off, ui->l2levelslider);
    pus[2]->AttachToGui(ui->relabel, ui->buttonre_on, ui->buttonre_off, ui->relevelslider);

    connect(ui->serialPortOpenButton, SIGNAL(clicked()), this, SLOT(OpenPort()));
    connect(ui->serialPortCloseButton, SIGNAL(clicked()), this, SLOT(ClosePort()));
    connect(ui->serialPortReset, SIGNAL(clicked()), this, SLOT(ResetPort()));

    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(UpdateInfo()));
    ui->updateButton->setEnabled(false);

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));


    dimmerData = new QStandardItemModel(DIMMER_COUNT,3,this);
    dimmerData->setHorizontalHeaderItem(0, new QStandardItem(QString("Power Unit")));
    dimmerData->setHorizontalHeaderItem(1, new QStandardItem(QString("Level")));
    dimmerData->setHorizontalHeaderItem(2, new QStandardItem(QString("Compare Val")));

    ui->dimmerTable->setModel(dimmerData);

    for(int i = 0; i < DIMMER_COUNT; i++)
    {
        ui->dimmerTable->setRowHeight(i,20);
        for(int j = 0; j < 3; j++)
            dimmerData->setItem(i,j, new QStandardItem());
    }


    ZeroDisplays();

    ui->statusBar->showMessage("Disconnected");

    // Start a timer
    updatetimer = new QTimer();
    connect(updatetimer, SIGNAL(timeout()), this, SLOT(UpdateInfo()));
}

BPLightContraption::~BPLightContraption()
{
    try {
    ClosePort();
    delete dimmerData;
    delete ui;
    delete updatetimer;
    }
    catch(...)
    {
        //eat exceptions
    }
}

void BPLightContraption::OpenPort(void)
{
    try {
        QString port = ui->serialPortCombo->currentText();
        ui->statusBar->showMessage(QString("Connecting to %1").arg(port));

        mc->OpenPort(port);

        ui->statusBar->showMessage(QString("Connected to %1").arg(port));
        ui->updateButton->setEnabled(true);

        for_each(pus.begin(), pus.end(), [](QSharedPointer<PUInterfaceGUI> & spu) { spu->SyncGUI(); });

        UpdateInfo();

        updatetimer->start(1000);
    }
    catch(const MCInterfaceException & ex)
    {
        ExceptionBox(ex);
        ui->statusBar->showMessage("Disconnected");
    }
}

void BPLightContraption::ZeroDisplays(void)
{
    ui->freqrawRisingDisplay->display(0);
    ui->freqrawFallingDisplay->display(0);
    ui->freqrawAvgDisplay->display(0);

    ui->freqRisingDisplay->display(0);
    ui->freqFallingDisplay->display(0);
    ui->freqAvgDisplay->display(0);

    for(int i = 0; i < DIMMER_COUNT; i++)
    {
        for(int j = 0; j < 3; j++)
            dimmerData->item(i,j)->setText("");
    }
}

void BPLightContraption::ClosePort(void)
{
    mc->ClosePort();

    ui->updateButton->setEnabled(false);
    ui->statusBar->showMessage("Disconnected");

    ZeroDisplays();

    for_each(pus.begin(), pus.end(), [](QSharedPointer<PUInterfaceGUI> & spu) { spu->Reset(); });

    updatetimer->stop();
}

void BPLightContraption::ResetPort(void)
{
    if(mc->IsOpen())
    {
        ClosePort();
        OpenPort();
    }
}

double BPLightContraption::Convert16BitValue(quint8 low, quint8 high)
{
    return low + (high << 8);
}

double BPLightContraption::Display16BitValue(QLCDNumber * display, quint8 low, quint8 high)
{
    double v = Convert16BitValue(low, high);
    display->display(v);
    return v;
}

void BPLightContraption::UpdateInfo(void)
{
    try {

    // stop the timer to prevent backups
    updatetimer->stop();

    if(!mc->IsOpen())
        return;

    QByteArray info = mc->RetrieveInfo();

    //qDebug() << "Byte Array:\n";
    //for_each(info.begin(), info.end(), [](quint8 v) { qDebug() << v << "\n";});
    //qDebug() << "\n";

    double fallingfreq = Display16BitValue(ui->freqrawFallingDisplay, info[0], info[1]);
    double risingfreq = Display16BitValue(ui->freqrawRisingDisplay, info[2], info[3]);
    double averagefreq = (fallingfreq + risingfreq)/2.0;
    ui->freqrawAvgDisplay->display(averagefreq);

    fallingfreq =  MICROCONTROLLER_FCPU/(fallingfreq*16.0);
    risingfreq  =  MICROCONTROLLER_FCPU/(risingfreq*16.0);
    averagefreq = MICROCONTROLLER_FCPU/(averagefreq*16.0);
    ui->freqFallingDisplay->display(fallingfreq);
    ui->freqRisingDisplay->display(risingfreq);
    ui->freqAvgDisplay->display(averagefreq);

    int off = 4;

    for(int i = 0; i < DIMMER_COUNT; i++)
    {
        if((qint8)info[off+4*i] == 0u)
        {
            dimmerData->item(i,0)->setText("None");
            dimmerData->item(i,1)->setText("-");
            dimmerData->item(i,2)->setText("-");
        }
        else
        {
            dimmerData->item(i,0)->setText(ConvertPUID(info[off+4*i]));
            dimmerData->item(i,1)->setText(QString("%1%").arg(quint16(info[off+1+4*i])));
            dimmerData->item(i,2)->setText(QString("%1").arg(Convert16BitValue(info[off+2+4*i],info[off+3+4*i])));
        }
    }


    off = 4+4*DIMMER_COUNT;
    //  Now the power units
    /*for(int i = 0; i < PU_COUNT; i++)
    {
        // ignore the id at off+3*i
        pus[i]->SyncState(info[off+1+3*i], info[off+2+3*i]);
    }*/

    //restart the timer
    updatetimer->start(1000);

    }
    catch (const MCInterfaceException & ex)
    {
        ExceptionBox(ex);
    }
}

void BPLightContraption::ExceptionBox(const MCInterfaceException & e)
{
    QString errstr;
    QTextStream err(&errstr);
    err << e.what() << "\n";
    err << "\nuC Error: " << e.GetMCError() << " (" << ConvertMCError(e.GetMCError()) << ")";
    err << "\nuC Error Cmd: " << e.GetMCErrorCMD();
    err << "\nuC Error ID: " << e.GetMCErrorID();
    err << "\nSP Error: " << e.GetSPError() << " (" << e.GetSPErrorText() << ")";

    QMessageBox::information(this, "BPLightContraption Error", errstr);
}

