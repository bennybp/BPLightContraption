/*! \file
 *  \brief     A class describing individual, controllable power units
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <exception>

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "powerunit.h"
#include "microcont.h"
#include "microcontexception.h"
#include "commands.h"

using namespace std;

PUInterface::PUInterface(char id, const QString &desc, QSharedPointer<MCInterface> mc)
        : _id(id),_desc(desc),_mc(mc)
{
    Reset();
}

PUInterface::~PUInterface()
{
}

quint8 PUInterface::GetLevel(void)
{
    return _level;
}

QString PUInterface::GetDescription(void)
{
    return _desc;
}

char PUInterface::GetID(void)
{
    return _id;
}


bool PUInterface::Toggle(void)
{
    if(_state == PUSTATE_ON)
        TurnOff();
    else if(_state == PUSTATE_OFF)
        TurnOn();

    return _state;
}

quint8 PUInterface::SetLevel(quint8 level)
{
    quint8 command[4];
    command[0] = '\\';
    command[1] = COM_LEVEL;
    command[2] = _id;
    command[3] = level;

    _mc->SendCommand(command, 4, 0);

    _level = level;


    if(level >= 100)
        _state = PUSTATE_ON;
    else if(level == 0)
        _state = PUSTATE_OFF;
    else
        _state = PUSTATE_DIM;


    return _level; // for now. Maybe some more complex stuff in the
    //  future with what comes back from the microcontroller
}


void PUInterface::TurnOff(void)
{
    quint8 command[3];
    command[0] = '\\';
    command[1] = COM_OFF;
    command[2] = _id;

    _mc->SendCommand(command, 3, 0);

    _level = 0;
    _state = PUSTATE_OFF;
}

void PUInterface::TurnOn(void)
{
    quint8 command[3];
    command[0] = '\\';
    command[1] = COM_ON;
    command[2] = _id;

    _mc->SendCommand(command, 3, 0);

    _level = 100;
    _state = PUSTATE_ON;
}

void PUInterface::Reset()
{
    _level = 0;
    _state = PUSTATE_OFF;
}

bool PUInterface::MCIsOpen(void)
{
    return _mc->IsOpen();
}

quint8 PUInterface::GetState(void)
{
    return _state;
}

void PUInterface::SyncState(char state, quint8 level)
{
    _state = state;
    _level = level;
}

