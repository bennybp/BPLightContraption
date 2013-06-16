/*! \file
 *  \brief     Class for controlling the microcontroller
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#include "powerunit_gui.h"
#include "commands.h"
#include "microcont.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

MCInterface::MCInterface()
{
    _mcerror = _mcerrorcmd = _mcerrorid = -1;
}

QSerialPort::SerialPortError MCInterface::GetSPError(void) const
{
    return _sp.error();
}

int MCInterface::GetMCError(void) const
{
    return _mcerror;
}

int MCInterface::GetMCErrorID(void) const
{
    return _mcerrorid;
}

int MCInterface::GetMCErrorCMD(void) const
{
    return _mcerrorcmd;
}

void MCInterface::OpenPort(const QString & port)
{
    if(_sp.isOpen())
        ClosePort();

    //_sp.setBaudRate(QSerialPort::Baud2400);
    //_sp.setBaudRate(QSerialPort::Baud9600);
    _sp.setBaudRate(QSerialPort::Baud38400);
    //_sp.setBaudRate(QSerialPort::Baud57600);
    _sp.setStopBits(QSerialPort::OneStop);
    _sp.setParity(QSerialPort::NoParity);
    _sp.setDataBits(QSerialPort::Data8);

    QSerialPortInfo qi(port);

    _sp.setPort(qi);

    if(!_sp.open(QIODevice::ReadWrite))
    {
        _sp.close();
        ThrowException("Unable to open port");
    }

    QByteArray idstring = SendCommand(NULL, 0, 3, 2000);

    if(idstring.size() != 3 || idstring[0] != 'B' || idstring[1] != 'e' || idstring[2] != 'n')
    {
        _sp.close();
        ThrowException(QString("Invalid initial connection response: ").append(idstring));
    }
}


void MCInterface::ClosePort(void)
{
    if(_sp.isOpen())
        _sp.close();
}

void MCInterface::ResetPort(void)
{
    ClosePort();
    OpenPort(_sp.portName());
}



QByteArray MCInterface::SendCommand(const quint8 * command, int len, unsigned int expectedreslen, int timeout)
{
    _mcerror = _mcerrorcmd = _mcerrorid = -1;

    if(!(_sp.isOpen()))
        ThrowException("Port not opened");

    if(len > 0)
    {
        if(_sp.write((const char *)command, len) != len)
            ThrowException("Unable to write command");
    }

    QByteArray res;
    while((unsigned int)res.size() < (4+expectedreslen))
    {

        if(!( _sp.waitForReadyRead(timeout)))
            ThrowException("Timeout waiting for response");

        res.push_back(_sp.readAll());
    }

    _mcerror = res[1];
    _mcerrorcmd = res[2];
    _mcerrorid = res[3];

    if(_mcerror != RES_SUCCESS)
        ThrowException("MCInterface error");

    if((unsigned int)res[0] != (3+expectedreslen))
        ThrowException(QString("Unexpected response size: %1 instead of %2").arg((int)res[0]).arg(3+expectedreslen));

    return res.mid(4, res.size()-4);
}

void MCInterface::ThrowException(const QString & desc) const
{
    throw MCInterfaceException(desc, _mcerror, _mcerrorcmd, _mcerrorid, GetSPError());
}

QByteArray MCInterface::RetrieveInfo(void)
{
    uint8_t infocmd[2] = {'\\',COM_INFO};
    return SendCommand(infocmd, 2, INFO_SIZE);
}


bool MCInterface::IsOpen(void)
{
    return _sp.isOpen();
}

