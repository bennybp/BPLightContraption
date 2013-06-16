/*! \file
 *  \brief     Main functions for the microcontroller
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#include <avr/io.h>
#include <avr/interrupt.h>
/*#include <avr/pgmspace.h>*/
#include <util/delay.h>

#include "serial.h"
#include "commands.h"
#include "bits.h"


#define NULL 0x0

/*! \brief The size of the command received buffer (in bytes) 

    Note that the buffer is index using a uint8_t. Therefore,
    BUFSIZE must be less than or equal to 256
*/
#define BUFSIZE 64


/*! \brief Pulse width required to turn on the triac, in microseconds 

    Should be as short as possible
*/
#define PULSE_WIDTH 3 /* microseconds */


/*! \brief The number of clock ticks for half of a 60Hz sine wave */
#define ONETWENTYHERTZ 16667ul


/*! \brief Integers representing the fraction of 2^16 for the
           different levels.

   The level (as a %) is given as the index. This is
   normalized so that the precentage represent the percentage
   output power, rather than the percent phase-shift for the
   sine wave.

   For a given percentage, the level is stored as the fraction of
   2^16, rather than the fraction of 100. This gets multiplied
   by ONETWENTYHERTZ and then divided by 2^16, allowing only
   bit shift operations and not floating point math.

   \note I don't have much in ram at the moment.
         this may be moved to PROGMEM eventually
   \warning I don't remember exactly how I did this... 
*/
 
const uint16_t levelarray[101] =
{
    65535,57934,55907,54462,53297,52300,51419,50621,49889,49208,
    48568,47964,47390,46841,46314,45807,45317,44842,44381,43933,
    43495,43068,42650,42240,41838,41443,41055,40672,40295,39923,
    39556,39193,38834,38479,38127,37778,37432,37089,36748,36409,
    36072,35737,35403,35071,34740,34410,34080,33752,33424,33096,
    32768,32440,32112,31784,31456,31126,30796,30465,30133,29799,
    29464,29127,28788,28447,28104,27758,27409,27057,26702,26343,
    25980,25613,25241,24864,24481,24093,23698,23296,22886,22468,
    22041,21603,21155,20694,20219,19729,19222,18695,18146,17572,
    16968,16328,15647,14915,14117,13236,12239,11074,9629,7602,
    00000
};

struct DimmerClock;

/*! \brief A struct representing a controllable power output

    For now, its either one of the lights or the receptacle.
    The combination of portreg and portbit represents the
    output pin that this dimmer controls (for example, 
    portreg = &PORTG  and portbit = 1 means this
    DimmerClock controls the output on PG1.
 */
struct PowerUnit
{
    /*! \brief A numeric ID for this unit */
    uint8_t id;                           

    /*< \brief The current state of the PowerUnit */
    uint8_t state;                        

    /*! \brief Pointer to the register that this PowerUnit uses */
    volatile uint8_t * portreg;           

    /*! \brief The bit on the portreg register that the PowerUnit controls */
    uint8_t portbit;

    /*! \brief Pointer to the dimmer that this is connected to */
    volatile struct DimmerClock * dimmer;
};


/*! \brief A struct for a clock used for dimming a PowerUnit

    The combination of interrupt and interrupt represents the
    timer interrupt register and bit to be set/cleared when
    dimming is started or stopped. This prevents unecessary
    interrupts when thing are not being dimmed.
    The compare register pointed to by comparereg
    is set to the appropriate time that represents
    when to turn on/pulse the triac.
*/
struct DimmerClock
{
    /*! \brief The current level of the dimmer (0-100) */
    uint8_t level;

    /*! \brief The timer interrupt register */
    volatile uint8_t * interruptreg;

    /*! \brief The bit on the timer interrupt register */
    uint8_t interruptbit;

    /*! \brief Time timer compare register to use */
    volatile uint16_t * comparereg;

    /*! \brief The currently-connected PowerUnit, if any */
    volatile struct PowerUnit * pu;
};


/* \brief The PowerUnits used by this microcontroller */
volatile struct PowerUnit punits[PU_COUNT];

/* \brief The DimmerClocks used by this microcontroller */
volatile struct DimmerClock dimclocks[DIMMER_COUNT];


/* \brief Count of the number of zero-crossings */
/* volatile uint16_t zerocrosscount; */

/*! \brief The times from the zero-crossing timer represing the
           falling edge [0] and the rising edge [1] */ 
volatile uint16_t zerocrossstamp[2];

/*! \brief A buffer for receiving input from the serial port */
volatile uint8_t serbuffer[BUFSIZE];

/*! \brief Index to be read next in the input buffer (serbuffer) */
volatile uint8_t curRead;

/*! \brief Index to be written next in the input buffer (serbuffer) */
volatile uint8_t curWrite;


/*! \brief Read the next entry in the input buffer

    This takes care of wrapping around the end of the buffer
*/
uint8_t ReadNextBuff(void)
{
    char c;

    while(curRead == curWrite);

    c = serbuffer[curRead++];

    if(curRead >= BUFSIZE)
        curRead = 0;

    return c;
}

/*! \brief Write the next entry in the input buffer

    This takes care of wrapping around the end of the buffer
*/
void WriteNextBuff(const uint8_t c)
{
    serbuffer[curWrite++] = c;
    if(curWrite >= BUFSIZE)
        curWrite = 0;
}


/*! \brief Start dimming a power unit at the given level 

    Finds the next available dimmer. If no dimmer is available, it
    returns RES_NODIMMER. Otherwise, returns RES_SUCCESS.
    \note pu->dimmer should be checked for NULL prior
          to calling this
*/
uint8_t StartDimming(volatile struct PowerUnit * pu, uint8_t level)
{
    volatile struct DimmerClock * dim = NULL;
    uint8_t dindex;

    /* find the next available dimmer */
    for(dindex = 0; dindex < DIMMER_COUNT; dindex++)
    {
        if(dimclocks[dindex].pu == NULL)
        {
            dim = &(dimclocks[dindex]);
            break;
        }
    }

    if(dim == NULL)
        return RES_NODIMMER; /* No available dimmers */

    dim->pu = pu;

    dim->level = level;

    /* NOT using PROGMEM */
    *(dim->comparereg) = (((uint32_t)levelarray[level] * (uint32_t)ONETWENTYHERTZ) >> 16);

    /* If using PROGMEM */
    /**(dim->comparereg) = (((uint32_t)pgm_read_word(&levelarray[level]) * (uint32_t)ONETWENTYHERTZ) >> 16);*/

    /* Enable the timer interrupt */
    bit_set(*(dim->interruptreg), dim->interruptbit);

    pu->dimmer = dim;

    pu->state = PUSTATE_DIM;

    return RES_SUCCESS;
}

/*! \brief Stops dimming on a given PowerUnit

    \note This does not turn off the pin - it may be left on! 
*/
void StopDimming(volatile struct PowerUnit* pu)
{
    /* Turn off the timer interrupt */
    bit_clear(*(pu->dimmer->interruptreg), pu->dimmer->interruptbit);
    pu->dimmer->level = 0;

    /* turn off the interrupts before doing this! */
    pu->dimmer->pu = NULL;
    pu->dimmer = NULL;
}


/*! \brief Completely turns on a PowerUnit

    This will force the output on the IO pin to be set continuously to 1
*/
void TurnOn(volatile struct PowerUnit* pu)
{
    /* Is it currently being dimmed? */
    if(pu->dimmer == NULL) /* Nope, not being dimmed */
    {
            bit_set(*(pu->portreg), pu->portbit);
            pu->state = PUSTATE_ON;
    }
    else /* It is being dimmed */ 
    {
            /* Stop dimming, turn on completely */
            /**(pu->dimmer->comparereg) = 1000; */ /* may prevent flashes */
            StopDimming(pu);
            bit_set(*(pu->portreg), pu->portbit);
            pu->state = PUSTATE_ON;
    }
}


/*! \brief Completely turns off a PowerUnit

    This will force the output on the IO pin to be set continuously to 0
*/
void TurnOff(volatile struct PowerUnit* pu)
{
    /* Is it currently being dimmed? */
    if(pu->dimmer == NULL) /* Nope, not being dimmed */
    {
            bit_clear(*(pu->portreg), pu->portbit);
            pu->state = PUSTATE_OFF;
    }
    else /* It is being dimmed */
    {
            /* Stop dimming, turn off completely */
            /**(pu->dimmer->comparereg) = 65535; */ /* may prevent flashes */
            StopDimming(pu);
            bit_clear(*(pu->portreg), pu->portbit);
            pu->state = PUSTATE_OFF;
    }
}


/*! \brief Change the dimming level of a PowerUnit 

    \bug Sometimes the light will flash momentarily. This
         is possibly due to some timing issues when changing
         the level while the timer is still running (ie there
         is a timer interrupt at an inopportune time, or
         the compare value changes at a bad time)
*/
uint8_t Level(volatile struct PowerUnit* pu, uint8_t level)
{
    uint8_t ret = RES_SUCCESS;
    uint16_t newreg = 0;

    if(level >= 100)
	TurnOn(pu);
    else if(level == 0)
        TurnOff(pu);
    else
    {
        if(pu->dimmer == NULL)
            ret = StartDimming(pu, level);
        else
        {
            /* see documentation for levelarray */
            newreg = (((uint32_t)levelarray[level] * (uint32_t)ONETWENTYHERTZ) >> 16);

            /* may prevent some flashes due to errors in timing ? */
            if(level > pu->dimmer->level && ICR4 >= newreg)
            {
               
               bit_set(PORTB,7);
               bit_set(*(pu->portreg), pu->portbit);
               _delay_us(PULSE_WIDTH);
               bit_clear(*(pu->portreg), pu->portbit);
            }

            pu->dimmer->level = level;
            *(pu->dimmer->comparereg) = newreg;
        }
    }

    return ret;
}


/*! \brief Process a command stored in the buffer

    The command is given by the only parameter, and any
    further information is obtained directly from the buffer

    This function also returns the appropriate response through the
    serial port
*/
uint8_t ProcessCommand(uint8_t command)
{
    uint8_t ret = RES_SUCCESS;
    uint8_t id = 0;
    uint8_t level = 0;
    uint8_t i;
    uint8_t counter = 0;
    uint8_t info[3+INFO_SIZE];

    switch (command)
    {
    case COM_NOTHING:
        break;

    case COM_LEVEL:
        id = ReadNextBuff();
        level = ReadNextBuff();
        if(id > PU_COUNT || id == 0)
            ret = RES_INVALID_ID;
        else
            ret = Level(&punits[id-1], level);

        Serial_send3(ret, command, id);
        break;

    case COM_ON:
        id = ReadNextBuff();
        if(id > PU_COUNT || id == 0)
            ret = RES_INVALID_ID;
        else
            ret = Level(&punits[id-1], 100);

        Serial_send3(ret, command, id);
        break;

    case COM_OFF:
        id = ReadNextBuff();
        if(id > PU_COUNT || id == 0)
            ret = RES_INVALID_ID;
        else
            ret = Level(&punits[id-1], 0);

        Serial_send3(ret, command, id);
        break;

    case COM_INFO:
        info[0] = RES_SUCCESS;
        info[1] = COM_INFO;
        info[2] = 0;
        info[3] = zerocrossstamp[0]; /* low part */
        info[4] = (zerocrossstamp[0] >> 8); /* high part */
        info[5] = zerocrossstamp[1]; /* low part */
        info[6] = (zerocrossstamp[1] >> 8); /* high part */
        counter = 7;
        for(i = 0; i < DIMMER_COUNT; i++)
        {
            if(dimclocks[i].pu == NULL)
            {
                info[counter++] = 0;
                info[counter++] = 0;
                info[counter++] = 0;
                info[counter++] = 0;
            }
            else
            {
                info[counter++] = dimclocks[i].pu->id;
                info[counter++] = dimclocks[i].level;
                info[counter++] = *(dimclocks[i].comparereg);
                info[counter++] = (*(dimclocks[i].comparereg) >> 8);
            }
        }

        for(i = 0; i < PU_COUNT; i++)
        {
            info[counter++] = punits[i].id;
            info[counter++] = punits[i].state;

            if(punits[i].dimmer == NULL)
                info[counter++] = 0;
            else
                info[counter++] = punits[i].dimmer->level;
        }

        Serial_sendarr(info, INFO_SIZE+3);
        break;
    default:
        ret = RES_INVALID_COM;
        Serial_send3(ret, command, id);
        break;
    }

    return ret;
}


/*! \brief Creates a new dimmer clock object

    For a description of the parameters, see
    the details for the DimmerClock struct
*/
void NewDimmerClock(volatile struct DimmerClock * dim,
                 volatile uint8_t * interruptreg,
                 uint8_t interruptbit,
                 volatile uint16_t * comparereg)
{
    dim->interruptreg = interruptreg;
    dim->interruptbit = interruptbit;
    dim->comparereg = comparereg;
    dim->pu = NULL;
    dim->level = 0;
}

/*! \brief Creates a new PowerUnit object

    For a description of the parameters, see
    the details for the PoerUnit struct
*/
void NewPowerUnit(volatile struct PowerUnit * pu,
                  uint8_t id,
                  volatile uint8_t * portreg,
                  uint8_t portbit)
{
    pu->id = id;
    pu->state = PUSTATE_OFF;
    pu->portreg = portreg;
    pu->portbit = portbit;
    pu->dimmer = NULL;
}


/*! \brief Main loop of the microcontroller

    This loop sets up the power units and dimmer
    clocks, initializes the serial port, as well
    as sets up the input pin and interrupt for
    the zero-crossing timer. It also initializes the
    command buffer and sends an identification
    string to the PC program.

    After that, it enters a loop, waiting for
    commands to be written to the command buffer (serbuffer)
    and executes ProcessCommand() if necessary.
*/
int main(void)
{
    uint8_t c;

    NewPowerUnit(&punits[0], PU_LIGHT1, &PORTG, 0);
    NewPowerUnit(&punits[1], PU_LIGHT2, &PORTG, 1);
    NewPowerUnit(&punits[2], PU_RECEPTACLE, &PORTG, 2);

    NewDimmerClock(&dimclocks[0], &TIMSK1, OCIE1A, &OCR1A);
    NewDimmerClock(&dimclocks[1], &TIMSK1, OCIE1B, &OCR1B);
    NewDimmerClock(&dimclocks[2], &TIMSK1, OCIE1C, &OCR1C);
    NewDimmerClock(&dimclocks[3], &TIMSK3, OCIE3A, &OCR3A);
    NewDimmerClock(&dimclocks[4], &TIMSK3, OCIE3B, &OCR3B);
    NewDimmerClock(&dimclocks[5], &TIMSK3, OCIE3C, &OCR3C);

    /* Initialize */
    curRead = curWrite = 0;
    /* zerocrosscount = 0; */

    /* Initialize the serial port */
    Serial_init();

    /* For the internal LED */
    bit_set(DDRB, 7);

    /* Outputs to the triacs */
    /*  Light 1 -> Arduino pin 41 */
    /*  Light 2 -> Arduino pin 40 */
    /*  Receptacle -> Arduino pin 39 */
    bit_set(DDRG, 0);
    bit_set(DDRG, 1);
    bit_set(DDRG, 2);


    /****************************************/
    /* Zero crossing detector/input capture */
    /****************************************/
    /* Input capture ICP4/PL0 (Arduino pin 49) */
    /* Noise canceller */
    bit_set(TCCR4B, ICNC4);
    /* Input capture interrupt */
    bit_set(TIMSK4, ICIE4);
    /* Internal pull-up resistor */
    /* Already configured as input by default */
    bit_set(PORTL, 0);


    /****************************************/
    /* Zero crossing Timer setup            */
    /****************************************/
    /* Timer 4 is used as the input capture */
    /*  and is being run in normal mode, also at fcpu8 */
    /*  Both: Maximum (TOP) should be about 32.7ms */
    bit_set(TCCR4B, CS41);

    /****************************************/
    /* Dimmer Timer setup                   */
    /****************************************/
    /* Timers 1&3 are the phase shift delay */
    /* Run in CTC mode at fcpu/8 */
    /* Interrupts initially off, levels at 0 */
    /* The max value is set to the zero crossing time */
    bit_set(TCCR1B, WGM13);
    bit_set(TCCR1B, WGM12);
    ICR1 = ONETWENTYHERTZ;
    bit_set(TCCR1B, CS11);

    bit_set(TCCR3B, WGM33);
    bit_set(TCCR3B, WGM32);
    ICR3 = ONETWENTYHERTZ;
    bit_set(TCCR3B, CS31);

    /* sleep_enable(); */

    /*  Enable global interrupts */
    sei();

    /* Send the identification string */
    Serial_send6(0,0,0,'B','e','n');

    while(1)
    {
        /* sleep_cpu(); */

        /*while(Serial_needsreading())
            WriteNextBuff(Serial_receive());*/

        /*if(bit_get(UCSR0A, DOR0) || bit_get(UCSR0A, FE0))
            bit_clear(PORTB, 7);*/

        /*if(zerocrosscount >= 120)
        {
            bit_flip(PORTB,7);
            zerocrosscount =0;
        }*/

        /* Check for new commands & process them */
        if(curRead != curWrite)
        {
            c = ReadNextBuff();
            if( c == '\\')
            {
                if(ProcessCommand((uint8_t) ReadNextBuff()) != RES_SUCCESS)
                {
                    Serial_flush();
                    curRead = curWrite = 0;
                }
            }
            else
            {
                Serial_send3(RES_INVALID_START,c,0);
                Serial_flush();
                curRead = curWrite = 0;
            }
        }
    }

    return 0;
}



/*! \brief Interrupt routine for zero-cross 

    This gets run on a rising or falling edge (dependong on
    register TCCR4B, bit ICES4). It records the timestamp
    in zerocrossstamp.

    After detecting a rising edge, it adjusts the counters for the dimmer clocks
    to what would be expected so that they remain in sync.
*/
ISR(TIMER4_CAPT_vect)
{
    /* Adjust the dimmer timers' counters continuously
       Do this only on the rising edge. We
       can calculate what the other counter should be at this point
       TCNT3 = ((zerocrossstamp[0] + zerocrossstamp[1])/2 - ICR4)/2 + ICR4
          ...some algebra...
       TCNT3 = (zerocrossstamp[0] + zerocrossstamp[1] + (2*ICR4))/4;
          ...then use bit shifts rather than multiplication
    */
    if(bit_get(TCCR4B, ICES4))
    {
        TCNT1 = TCNT3 = ((zerocrossstamp[0] + zerocrossstamp[1] + (ICR4<<1))>>2);
    }

    /* Store the timestamp */

    /* I don't know why the following line doesn't work */
    /*zerocrossstamp[bit_get(TCCR4B, ICES4)] = ICR4; */
    if(bit_get(TCCR4B, ICES4))
        zerocrossstamp[1] = ICR4;
    else
        zerocrossstamp[0] = ICR4;

    /* Flip the input capture edge detector */
    bit_flip(TCCR4B, ICES4);

    /* Increment the counter */
    /*zerocrosscount++;*/
    
    /* Zero the timer counter */
    TCNT4 = 0;

}

/************************************
 * INTERRUPTS
 ************************************/

/*! \brief Timer interrupt for phase-shifting 

   Gets run when the pulse must be sent to the triac
   to turn on the circuit. The value at which this gets
   called is set with Level()
*/
ISR(TIMER1_COMPA_vect)
{
    bit_set(*(dimclocks[0].pu->portreg), dimclocks[0].pu->portbit);
    _delay_us(PULSE_WIDTH);
    bit_clear(*(dimclocks[0].pu->portreg), dimclocks[0].pu->portbit);
}

/*! \copydoc ISR(TIMER1_COMPA_vect) */
ISR(TIMER1_COMPB_vect)
{
    bit_set(*(dimclocks[1].pu->portreg), dimclocks[1].pu->portbit);
    _delay_us(PULSE_WIDTH);
    bit_clear(*(dimclocks[1].pu->portreg), dimclocks[1].pu->portbit);
}

/*! \copydoc ISR(TIMER1_COMPA_vect) */
ISR(TIMER1_COMPC_vect)
{
    bit_set(*(dimclocks[2].pu->portreg), dimclocks[2].pu->portbit);
    _delay_us(PULSE_WIDTH);
    bit_clear(*(dimclocks[2].pu->portreg), dimclocks[2].pu->portbit);
}

/*! \copydoc ISR(TIMER1_COMPA_vect) */
ISR(TIMER3_COMPA_vect)
{
    bit_set(*(dimclocks[3].pu->portreg), dimclocks[3].pu->portbit);
    _delay_us(PULSE_WIDTH);
    bit_clear(*(dimclocks[3].pu->portreg), dimclocks[3].pu->portbit);
}

/*! \copydoc ISR(TIMER1_COMPA_vect) */
ISR(TIMER3_COMPB_vect)
{
    bit_set(*(dimclocks[4].pu->portreg), dimclocks[4].pu->portbit);
    _delay_us(PULSE_WIDTH);
    bit_clear(*(dimclocks[4].pu->portreg), dimclocks[4].pu->portbit);
}

/*! \copydoc ISR(TIMER1_COMPA_vect) */
ISR(TIMER3_COMPC_vect)
{
    bit_set(*(dimclocks[5].pu->portreg), dimclocks[5].pu->portbit);
    _delay_us(PULSE_WIDTH);
    bit_clear(*(dimclocks[5].pu->portreg), dimclocks[5].pu->portbit);
}

/* \brief Interrupt routine for receiving commands through the serial port 

   If data is received from the serial port, this function is called, which
   just writes it to the command buffer (serbuffer)
*/
ISR(USART0_RX_vect)
{
    WriteNextBuff(Serial_receive());
}

