/*! \file
 *  \brief     A class for the GUI controlling of a power unit
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef POWERUNIT_GUI_H
#define POWERUNIT_GUI_H

#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QMessageBox>
#include <QSharedPointer>

#include "powerunit.h"
#include "microcontexception.h"

//! A GUI interface to a power unit
class PUInterfaceGUI : public PUInterface
{
    Q_OBJECT;

private:
    QLabel * _label;              //!< A label for the power unit
    QSlider * _levelslider;       //!< A slider for displaying and controlling the dimmer level
    QPushButton * _onbutton;      //!< A button that will turn the power unit on
    QPushButton *_offbutton;      //!< A button that will turn the power unit off
    QWidget * _parent;            //!< A parent widget

    Q_DISABLE_COPY(PUInterfaceGUI)

    //! Displays a message box with exception information
    void ExceptionBox(const MCInterfaceException & e);

private slots:
    //! Called when the dimmer level slider is changed
    /*!
     *  \param[in] val The new level
     *  \throw MCInterfaceException There is a problem communicating this command
     *         to the microcontroller
     */
    void LevelSliderChange(int val);

    //! Called when an event should turn the power unit on
    /*!
     *  \throw MCInterfaceException There is a problem communicating this command
     *         to the microcontroller
     */
    void TurnOn(void);

    //! Called when an event should turn the power unit on
    /*!
     *  \throw MCInterfaceException There is a problem communicating this command
     *         to the microcontroller
     */
    void TurnOff(void);

public:

    //! Create an object with the given information
    /*!
     * \param id The numerical ID of the power unit (see commands.h)
     * \param desc A text description of this unit
     * \param mc The microcontroller controlling this unit
     * \param parent A parent widget
     */
    PUInterfaceGUI(quint8 id, const QString & desc, QSharedPointer<MCInterface> mc, QWidget *parent);

    ~PUInterfaceGUI();

    //! Attaches this PUInterfaceGUI object to the interface elements
    void AttachToGui(QLabel * label, QPushButton * onbutton, QPushButton * offbutton,
                     QSlider * levelslider);

    //! Synchronizes the GUI elements with the state of the underlying PUInterface object
    /*!
     *  This does not obtain any information from the microcontroller
     */
    void SyncGUI(void);

    //! Resets the state of the power unit
    /*!
     *  This does not send or receive any information from the microcontroller
     */
    void Reset(void);

    //! Changes the state and dimmer level to the passed values
    /*!
     *  This does not send or receive any information from the microcontroller
     */
    void SyncState(char state, quint8 level);
};

#endif // MICROCONT_GUI_H

