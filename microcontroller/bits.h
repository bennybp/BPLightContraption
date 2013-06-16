/*! \file
 *  \brief     Macros to make bit manipulation easier
 *  \author    Benjamin Pritchard (ben@bennyp.org)
 *  \copyright 2013 Benjamin Pritchard. Released under the MIT License
 */

#ifndef BIT_H
#define BIT_H

/*! Get bit m in register p */
#define bit_get(p,m) ((p) & (0x01 << m))

/*! Set bit m in register p */
#define bit_set(p,m) ((p) |= (0x01 << m))

/*! Clear bit m in register p */
#define bit_clear(p,m) ((p) &= ~(0x01 << m))

/*! Flip or toggle bit m in register p */
#define bit_flip(p,m) ((p) ^= (0x01 << m))

/*! Convert a bit position to a bit */
#define BIT(x) (0x01 << (x))

#endif
