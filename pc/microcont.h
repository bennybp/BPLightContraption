/*! \file
 *  \brief     Class for controlling the microcontroller
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef MICROCONT_H
#define MICROCONT_H

#include <QSharedPointer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#define MICROCONTROLLER_FCPU 16000000ul

//! This class represents a microcontroller
/*!
 *  This command is mostly used to connect and send commands.
 */
class MCInterface : public QObject
{
public:

    //! Initializes the interface
    MCInterface();

    //! Opens the specified port
    /*!
     *  After opening the port, it waits for the identification string.
     *  If something goes wrong, it throws a MCInterfaceException (through ThrowException())
     */
    void OpenPort(const QString &port);

    //! Closes the serial connection with the microcontroller
    void ClosePort(void);

    //! Resets the port (closes and then reopens the port)
    void ResetPort(void);

    //! Returns true if the port is opened
    bool IsOpen(void);


    //! Sends a command to the microcontroller
    /*!
     *  Commands are stored in an array of bytes. The reason for this
     *  is that it is a bit cleaner to write the commands, ie
     *
     *  \code
     *  uint8_t infocmd[2] = {'\\',COM_INFO};
     *  \endcode
     *
     *  \param command Array of bytes to send
     *  \param len The length of the command to send
     *  \param expectedreslen The length of the result expected (not including the 3 header bytes)
     *  \throw MCInterfaceException An error occurred during sending
     *         the command or receiving the response
     */
    QByteArray SendCommand(const quint8 * command, int len, unsigned int expectedreslen, int timeout = 500);


    //! Returns the current error state of the microcontroller
    /*!
     * See ConvertMCError()
     */
    int GetMCError(void) const;


    //! Returns the id returned from the microcontroller during an error
    /*!
     *  The ID should represent which PowerUnit had a problem
     */
    int GetMCErrorID(void) const;

    //! Returns the command returned from the microcontroller during an error
    /*!
     *  The ID should represent which command had a problem
     */
    int GetMCErrorCMD(void) const;

    //! Returns the error code from the serial port
    /*!
     *  See documentation for QSterialPort
     */
    QSerialPort::SerialPortError GetSPError(void) const;


    //! Gets state info from the microcontroller
    /*!
     *  Information, including dimmer levels, timestamps, etc, are
     *  stored in a specific way in a QByteArray
     *
     *  \throw MCInterfaceException An error occurred during sending
     *         the command or receiving the response
     */
    QByteArray RetrieveInfo(void);

private:

    //! Disables copying of this class
    Q_DISABLE_COPY(MCInterface);

    //! Serial port object
    QSerialPort _sp;

    //! Microcontroller error flag
    int _mcerror;

    //! Command returned by the microcontroller during an error
    int _mcerrorcmd;

    //! ID of the power unit, etc, causing the error
    int _mcerrorid;


    //! Throws an exception using the current error numbers
    /*!
     *  This also taks a description
     */
    void ThrowException(const QString & desc) const;
};



#endif // MICROCONT_H
