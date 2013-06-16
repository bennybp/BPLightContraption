/*! \file
 *  \brief     A class for the GUI controlling of a power unit
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#include "powerunit_gui.h"
#include "commands-text.h"

#include <QObject>
#include <QTextStream>
#include <QSharedPointer>

PUInterfaceGUI::PUInterfaceGUI(quint8 id, const QString & desc, QSharedPointer<MCInterface> mc, QWidget * parent)
        : PUInterface(id, desc, mc)
{
    _label = NULL;
    _onbutton = _offbutton = NULL;
    _levelslider = NULL;
    _parent = parent;
}

PUInterfaceGUI::~PUInterfaceGUI()
{

}

void PUInterfaceGUI::AttachToGui(QLabel * label, QPushButton * onbutton, QPushButton * offbutton,
                               QSlider * levelslider)
{
    _label = label;
    _onbutton = onbutton;
    _offbutton = offbutton;
    _levelslider = levelslider;

    label->setText(QString(GetDescription()));
    connect(onbutton, SIGNAL(clicked()), this, SLOT(TurnOn()));
    connect(offbutton, SIGNAL(clicked()), this, SLOT(TurnOff()));
    connect(levelslider, SIGNAL(valueChanged(int)), this, SLOT(LevelSliderChange(int)));

    SyncGUI();
}

void PUInterfaceGUI::TurnOn(void)
{
    try {
        PUInterface::TurnOn();
    }
    catch (const MCInterfaceException & ex)
    {
        ExceptionBox(ex);
    }

    SyncGUI();
}

void PUInterfaceGUI::TurnOff(void)
{
    try {
        PUInterface::TurnOff();
    }
    catch (const MCInterfaceException & ex)
    {
        ExceptionBox(ex);
    }

    SyncGUI();
}

void PUInterfaceGUI::LevelSliderChange(int val)
{
    try {
        SetLevel(val);
    }
    catch (const MCInterfaceException & ex)
    {
        // revert the slider to the old value
        // which should still be in _level
        // This would cause it to emit a changed signal
        //_levelslider->setValue(GetLevel());
        ExceptionBox(ex);
    }

    SyncGUI();
}


void PUInterfaceGUI::SyncGUI(void)
{
    QString label = QString("%1\n%2").arg(GetDescription(),ConvertPUState(GetState()));

    if(GetState() == PUSTATE_DIM)
        label.append(QString(" (%1%)").arg(GetLevel()));

    _label->setText(label);

    _levelslider->setValue(GetLevel());
    _levelslider->setEnabled(MCIsOpen());
    _onbutton->setEnabled(MCIsOpen());
    _offbutton->setEnabled(MCIsOpen());
}

void PUInterfaceGUI::Reset(void)
{
    // Avoid emitting a 'changed' signal
    disconnect(_levelslider, SIGNAL(valueChanged(int)), this, SLOT(LevelSliderChange(int)));

    PUInterface::Reset();
    SyncGUI();

    connect(_levelslider, SIGNAL(valueChanged(int)), this, SLOT(LevelSliderChange(int)));
}

void PUInterfaceGUI::SyncState(char state, quint8 level)
{
    PUInterface::SyncState(state, level);
    SyncGUI();
}

void PUInterfaceGUI::ExceptionBox(const MCInterfaceException & e)
{
    QString errstr;
    QTextStream err(&errstr);
    err << e.what() << "\n";
    err << "\nuC Error: " << e.GetMCError() << " (" << ConvertMCError(e.GetMCError()) << ")";
    err << "\nuC Error Cmd: " << e.GetMCErrorCMD() << " (" << ConvertCommandID(e.GetMCErrorCMD()) << ")";
    err << "\nuC Error ID: " << e.GetMCErrorID() << " (" << ConvertPUID(e.GetMCErrorID()) << ")";
    err << "\nSP Error: " << e.GetSPError() << " (" << e.GetSPErrorText() << ")";

    QMessageBox::information(_parent, "PUInterface Error", errstr);
}

