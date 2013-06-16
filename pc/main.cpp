/*! \file
 *  \brief     Main file for the BPLightContraption project
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#include "triaclight.h"
#include "powerunit.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    BPLightContraption w;
    w.show();

    return a.exec();
}

