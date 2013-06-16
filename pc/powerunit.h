/*! \file
 *  \brief     A class describing individual, controllable power units
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef POWERUNIT_H
#define POWERUNIT_H

#include <QString>
#include <QSharedPointer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <microcont.h>

using namespace std;

//! A general interface to a power unit
/*!
 *  In general, if any operation goes wrong, this
 *  class will throw a MCInterfaceException
 */
class PUInterface : public QObject
{
    Q_OBJECT;

public:
   //! Constructor
   /*!
   *   Creates an interface to a power unit given an id, description, and
   *   an pointer to the micrcontroller it's connected to
   */
   PUInterface(char id, const QString &desc, QSharedPointer<MCInterface> mc);

   ~PUInterface();

   //! Returns the current dimmer level
   quint8 GetLevel(void);

   //! Returns the description of the power unit
   QString GetDescription(void);

   //! Returns the ID of the power unit
   char GetID(void);

   //! Returns the state of the power unit (see commands.h)
   quint8 GetState(void);

   //! Syncs the state of the class with the given state and dimmer level
   void SyncState(char state, quint8 level);

   //! Toggles the power unit between on and off
   /*!
    *
    *  This has no effect if the state is not PUSTATE_ON or PUSTATE_OFF
    *
    *  \throw MCInterfaceException There is a problem communicating this command
    *         to the microcontroller
    */
   bool Toggle(void);

   //! Dims the powerunit to the given level
   /*!
    *  \return The level to which it was actually set
    *  \throw MCInterfaceException There is a problem communicating this command
    *         to the microcontroller
    */
   quint8 SetLevel(quint8 level);

   //! Turns off the power unit
   /*!
    *  \throw MCInterfaceException There is a problem communicating this command
    *         to the microcontroller
    */
   void TurnOff(void);

   //! Turns on the power unit
   /*!
    *  \throw MCInterfaceException There is a problem communicating this command
    *         to the microcontroller
    */
   void TurnOn(void);


   //! Resets the power unit state
   /*!
    *  This does not actually communicate with the microcontroller
    */
   void Reset(void);


   //! Returns true if the microcontroller controlling this power unit is open
   bool MCIsOpen(void);


    private:
        char _id; //!< The ID given to this power unit
        QString _desc; //!< A text description of the power unit
        quint8 _state; //!< The current state of the power unit (PUSTATE_XXX)
        quint8 _level; //!< The current dimmer level

        QSharedPointer<MCInterface> _mc; //!< The microcontroller interface controlling this power unit

        Q_DISABLE_COPY(PUInterface)
};

#endif

