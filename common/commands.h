/*! \file
 *  \brief     Definition of commands, errors, etc, for the microcontroller
 *  \details   This file is shared between the microcontroller source and GUI source and
 *             therefore is written in strict C code
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef COMMANDS_H
#define COMMANDS_H

/* Number of dimmers and powerunits */
#define DIMMER_COUNT 6
#define PU_COUNT 3
#define INFO_SIZE (4+4*DIMMER_COUNT+3*PU_COUNT) 

/* IDs for the power units */
/* These always start at 1 */
#define PU_LIGHT1      1
#define PU_LIGHT2      2
#define PU_RECEPTACLE  3

/* States  of the powerunits */
#define PUSTATE_OFF    1
#define PUSTATE_ON     2
#define PUSTATE_DIM    3

/* General commands */
#define COM_NOTHING  0
#define COM_INFO     1
#define COM_ON       2
#define COM_OFF      3
#define COM_LEVEL    4


/* Responses & error codes */
#define RES_SUCCESS       0
#define RES_INVALID_START 1
#define RES_INVALID_COM   2
#define RES_INVALID_ID    3
#define RES_NODIMMER      4
#define RES_NOGENCLOCK    5
#define RES_FAILURE       126



#endif

