/*! \file
 *  \brief     Converts commands, powerunit ids, errors, etc (integers) to text
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */


#ifndef COMMANDSTEXT_H
#define COMMANDSTEXT_H

#include "commands.h"

//! Returns the name of a PowerUnit given its ID
inline const char * ConvertPUID(char id)
{
    switch (id)
    {
    case PU_LIGHT1:
        return "Light 1";
    case PU_LIGHT2:
        return "Light 2";
    case PU_RECEPTACLE:
        return "Receptacle";
    }

    return "Unknown";
}

//! Returns a description of an error from the microcontroller
inline const char * ConvertMCError(char error)
{
    switch (error)
    {
    case RES_SUCCESS:
        return "No error";
    case RES_INVALID_START:
        return "Invalid start to command";
    case RES_INVALID_COM:
        return "Invalid command";
    case RES_INVALID_ID:
        return "Invalid unit id";
    case RES_NODIMMER:
        return "No dimmer available";
    case RES_NOGENCLOCK:
        return "No general-purpose clock available";
    case RES_FAILURE:
        return "Other failure";
    }
    return "Unknown error";
}

//! Returns the text of a command given its ID
inline const char * ConvertCommandID(char cmd)
{
    switch (cmd)
    {
    case COM_NOTHING:
        return "Nothing/Ping";
    case COM_ON:
        return "Turn on";
    case COM_OFF:
        return "Turn off";
    case COM_LEVEL:
        return "Change level";
    case COM_INFO:
        return "Get info";
    }
    return "Unknown";
}

//! Returns the text of a PowerUnit state
inline const char * ConvertPUState(char state)
{
    switch (state)
    {
    case PUSTATE_OFF:
        return "Off";
    case PUSTATE_ON:
        return "On";
    case PUSTATE_DIM:
        return "Dimming";
    }
    return "Unknown";
}

#endif

