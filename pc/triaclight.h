/*! \file
 *  \brief     Main Qt GUI class
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef TRIACLIGHT_H
#define TRIACLIGHT_H

#include <QMainWindow>
#include <QVector>
#include <QLCDNumber>
#include <QTimer>
#include <QStandardItemModel>

#include "microcont.h"
#include "microcontexception.h"
#include "powerunit_gui.h"

namespace Ui {
class BPLightContraption;
}

//! The main window of the BPLightContraption program
class BPLightContraption : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit BPLightContraption(QWidget *parent = 0);
    ~BPLightContraption();

    //! Displays a message box with exception information
    void ExceptionBox(const MCInterfaceException & e);

private slots:
    //! Called when the button to open the port is clicked
    void OpenPort(void);

    //! Called when the button to close the port is clicked
    void ClosePort(void);

    //! Called when the button to reset the port is clicked
    void ResetPort(void);

    //! Called when the button to force an update is pressed
    void UpdateInfo(void);
    
private:
    Ui::BPLightContraption *ui;

    //! Used to refresh the information at certain intervals
    QTimer * updatetimer;

    //! Microcontrollers used by this program
    QSharedPointer<MCInterface> mc;

    //! Power units controlled by this program
    QVector<QSharedPointer<PUInterfaceGUI> > pus;

    //! Information about all the dimmers
    QStandardItemModel *dimmerData;

    //! Converts two separate bytes into a 16 bit value and displays it on a QLCDNumber
    double Display16BitValue(QLCDNumber * display, quint8 low, quint8 high);

    //! Converts two separate bytes into a 16 bit value
    double Convert16BitValue(quint8 low, quint8 high);

    //! Resets all the displays to zero
    void ZeroDisplays(void);
};

#endif // TRIACLIGHT_H

