/*! \file
 *  \brief     Microcontroller serial communications functions 
 *  \details   Definitions in serial.c
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef SERIAL_H
#define SERIAL_H

/*! \def MYUBRR
    \brief Integer representing the baud rate. See the
           ATMega1280 data sheet  
*/

/* Set MYUBRR to one of the below corresponding
   to the baud rate you want */

/* 2400 baud */
/*#define MYUBRR 416*/

/* 9600 baud */
/*#define MYUBRR 103*/

/* 28.8k baud */
/*#define MYUBRR 34*/

/* 38.4k baud */
#define MYUBRR 25


/*! \brief Initializes the serial port

    Currently, the port is set to 8N1 and the baud rate specified
    by MYUBRR. This function also enables the receiver interrupt.
*/
void Serial_init(void);


/*! \brief Flushes the serial port receive buffer 

    It will block until it gets something to receive
*/
void Serial_flush(void);


/*! \brief Retrieve a byte from the receive register

    Will block until there is something to be retrieved
*/
uint8_t Serial_receive(void);


/*! \brief Returns 1 if there is something to be 
           read from the serial port, zero otherwise */
uint8_t Serial_needsreading(void);



/*! \brief Sends a byte through the serial port */
void Serial_send( uint8_t c );

/*! \brief Sends 2 bytes through the serial port */
void Serial_send2( uint8_t c1, uint8_t c2 );

/*! \brief Sends 3 bytes through the serial port */
void Serial_send3( uint8_t c1, uint8_t c2, uint8_t c3 );

/*! \brief Sends 4 bytes through the serial port */
void Serial_send4( uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 );

/*! \brief Sends 5 bytes through the serial port */
void Serial_send5( uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 , uint8_t c5);

/*! \brief Sends 6 bytes through the serial port */
void Serial_send6( uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 , uint8_t c5, uint8_t c6);


/*! \brief Sends an array of bytes through the serial port

    \param[in] arr Array of bytes to send
    \param[in] len Number of bytes to send
*/
void Serial_sendarr( const uint8_t * arr, uint8_t len);

#endif
