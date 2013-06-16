/*! \file
 *  \brief     Exception thrown by the MCInterface class
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

#include "microcontexception.h"
#include "commands-text.h"

using namespace std;

MCInterfaceException::MCInterfaceException(const QString & desc,
                                                  int mcerror,
                                                  int mcerrorcmd,
                                                  int mcerrorid,
                                                  QSerialPort::SerialPortError sperror)
    : runtime_error(desc.toStdString())
{
    _mcerror = mcerror;
    _mcerrorid = mcerrorid;
    _mcerrorcmd = mcerrorcmd;
    _sperror = sperror;
}

MCInterfaceException::MCInterfaceException(const QString & desc,
                                                  int mcerror,
                                                  QSerialPort::SerialPortError sperror)
    : runtime_error(desc.toStdString())
{
    _mcerror = mcerror;
    _mcerrorid = _mcerrorcmd = -1;
    _sperror = sperror;
}

int MCInterfaceException::GetMCError(void) const
{
    return _mcerror;
}

int MCInterfaceException::GetMCErrorID(void) const
{
    return _mcerrorid;
}

int MCInterfaceException::GetMCErrorCMD(void) const
{
    return _mcerrorcmd;
}

QSerialPort::SerialPortError MCInterfaceException::GetSPError(void) const
{
    return _sperror;
}

const char * MCInterfaceException::GetSPErrorText(void) const
{
    switch (_sperror)
    {
    case QSerialPort::NoError:
        return "No error";
    case QSerialPort::DeviceNotFoundError:
                return "No such device";
    case QSerialPort::PermissionError:
                return "Permission Denied: Possibly already opened by another process";
    case QSerialPort::OpenError:
                return "Device already opened";
    case QSerialPort::ParityError:
                return "Hardware detected a parity error";
    case QSerialPort::FramingError:
                return "Framing Error";
    case QSerialPort::BreakConditionError:
                return "Break Condition Error";
    case QSerialPort::WriteError:
                return "I/O Error occurred while writing the data";
    case QSerialPort::ReadError:
                return "I/O Error occurred while reading the data";
    case QSerialPort::ResourceError:
                return "I/O error: resource unavailable";
    case QSerialPort::UnsupportedOperationError:
                return "Unsupported operation error";
    case QSerialPort::UnknownError:
                return "Unsupported operation error";
    default:
        return "Unknown error";
    }
}
