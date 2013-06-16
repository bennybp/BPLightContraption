/*! \file
 *  \brief     Microcontroller serial communications functions 
 *  \details   Prototypes in serial.h
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#include <avr/io.h>

#include "bits.h"
#include "serial.h"

void Serial_init(void)
{
    /* Synchronous mode */
    /*bit_set(UCSR0C, UMSEL00);*/

    /* Set baud rate */
    UBRR0H = (uint8_t)(MYUBRR>>8);
    UBRR0L = (uint8_t)MYUBRR;

    /* Enable receiver and transmitter */
    bit_set(UCSR0B, RXEN0);
    bit_set(UCSR0B, TXEN0);

    /* Set frame format: 8data, 1stop bit */
    bit_set(UCSR0C, UCSZ00);
    bit_set(UCSR0C, UCSZ01);

    /* enable the interrupts */
    bit_set(UCSR0B, RXCIE0);
}


uint8_t Serial_receive(void)
{
    /* Wait for data to be received */
    while ( !(bit_get(UCSR0A, RXC0)) );

    /* Get and return received data from buffer */
    return UDR0;
}


void Serial_flush(void)
{
    /* disable interrupts */
    bit_clear(UCSR0B, RXCIE0);

    /* disable the receiver and transmitter */
    bit_clear(UCSR0B, RXEN0);
    bit_clear(UCSR0B, TXEN0);

    /* wait for any transmissions to finish */
    while(bit_get(UCSR0A, TXC0));

    /* re-enable transmitter & receiver */
    bit_set(UCSR0B, RXEN0);
    bit_set(UCSR0B, TXEN0);

    /* re-enable interrupts */
    bit_set(UCSR0B, RXCIE0);
}


uint8_t Serial_needsreading(void)
{
    return bit_get(UCSR0A, RXC0);
}


void Serial_send( uint8_t c )
{
    /* Wait for empty transmit buffer */
    while ( ! (bit_get(UCSR0A, UDRE0)) );

    /* Put data into buffer, sends the data */
    UDR0 = c;
}


void Serial_send1( uint8_t c)
{
    Serial_send( 1 );
    Serial_send( c );
}


void Serial_send2( uint8_t c1 , uint8_t c2)
{
    Serial_send( 2 );
    Serial_send( c1 );
    Serial_send( c2 );
}


void Serial_send3( uint8_t c1 , uint8_t c2, uint8_t c3)
{
    Serial_send( 3 );
    Serial_send( c1 );
    Serial_send( c2 );
    Serial_send( c3 );
}


void Serial_send4( uint8_t c1 , uint8_t c2, uint8_t c3,
                   uint8_t c4)
{
    Serial_send( 4 );
    Serial_send( c1 );
    Serial_send( c2 );
    Serial_send( c3 );
    Serial_send( c4 );
}


void Serial_send5( uint8_t c1 , uint8_t c2, uint8_t c3,
                   uint8_t c4, uint8_t c5)
{
    Serial_send( 5 );
    Serial_send( c1 );
    Serial_send( c2 );
    Serial_send( c3 );
    Serial_send( c4 );
    Serial_send( c5 );
}


void Serial_send6( uint8_t c1 , uint8_t c2, uint8_t c3,
                   uint8_t c4, uint8_t c5, uint8_t c6)
{
    Serial_send( 6 );
    Serial_send( c1 );
    Serial_send( c2 );
    Serial_send( c3 );
    Serial_send( c4 );
    Serial_send( c5 );
    Serial_send( c6 );
}


void Serial_sendarr( const uint8_t * arr, uint8_t len)
{
    uint8_t i;
    Serial_send(len);
    for(i = 0; i < len; i++)
        Serial_send(arr[i]);
}

