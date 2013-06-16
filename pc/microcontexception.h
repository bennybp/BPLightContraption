/*! \file
 *  \brief     Exception thrown by the MCInterface class
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef MICROCONTEXCEPTION_H
#define MICROCONTEXCEPTION_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <exception>
#include <stdexcept>

using namespace std;


//! An exception thrown from the MCInterfaceException class
class MCInterfaceException : public runtime_error
{
private:
    int _mcerror;    //!< An error code
    int _mcerrorcmd; //!< Command that the microcontroller had a problem with (returned by the MC)
    int _mcerrorid;  //!< Powerunit ID that had a problem (returned by the MC)
    QSerialPort::SerialPortError _sperror; //!< The current status of the serial port

   public:
    //! Basic constructor
    MCInterfaceException(const QString & desc,
                       int mcerror,
                       int mcerrorcmd,
                       int mcerrorid,
                       QSerialPort::SerialPortError sperror);

    //! Constructor with just the microcontroller error component
    MCInterfaceException(const QString & desc,
                       int mcerror,
                       QSerialPort::SerialPortError sperror);

    //! Returns the current error code
    int GetMCError(void) const;

    //! Returns the IDd the microcontroller had a problem with
    /*!
     *  This code is returned by the microcontroller itself, indicating a
     *  problem with this power unit ID
     */
    int GetMCErrorID(void) const;

    //! Returns the command the microcontroller had a problem with
    /*!
     *  This code is returned by the microcontroller itself, indicating a
     *  problem with this command
     */
    int GetMCErrorCMD(void) const;

    //! Returns the current serial port error. See the QSerialPort documentation
    QSerialPort::SerialPortError GetSPError(void) const;

    //! Returns a string describing the current serial port error
    const char * GetSPErrorText(void) const;
};

#endif // POWERUNITEXCEPTION_H

