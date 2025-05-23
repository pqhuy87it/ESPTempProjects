/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _VARIANT_EINK290NRF52833PRO_
#define _VARIANT_EINK290NRF52833PRO_

#include "nrf.h"
#include "nrf_peripherals.h"

/** Master clock frequency */
#if defined(NRF52_SERIES)
#define VARIANT_MCK       (64000000ul)
#else
#define VARIANT_MCK       (16000000ul)
#endif

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Number of pins defined in PinDescription array
#if GPIO_COUNT == 1
#define PINS_COUNT           (32u)
#define NUM_DIGITAL_PINS     (32u)
#elif GPIO_COUNT == 2
#define PINS_COUNT           (64u)
#define NUM_DIGITAL_PINS     (64u)
#else
#error "Unsupported GPIO_COUNT"
#endif
#define NUM_ANALOG_INPUTS    (8u)
#define NUM_ANALOG_OUTPUTS   (8u)

#define EBYTE2
#define PIN_LED1             (2)
#define PIN_BUTTON           (24)
#define BLUE_LED             (PIN_LED1)
#define LED_BUILTIN          (PIN_LED1)
#define SOUND_PIN            (30)
#define RST_PIN              (17)
#define DC_PIN               (31)
#define CS_PIN               (22)
#define BUSY_PIN             (13)

/*
 * Analog pins
 */
#define PIN_A0               (2) // P0.02
#define PIN_A1               (3) // P0.03
#define PIN_A2               (4) // P0.04
#define PIN_A3               (5) // P0.05
#define PIN_A4               (28)// P0.28
#define PIN_A5               (29)// P0.29
#define PIN_A6               (30)// P0.30
#define PIN_A7               (31)// P0.31

static const uint8_t A0  = PIN_A0 ;
static const uint8_t A1  = PIN_A1 ;
static const uint8_t A2  = PIN_A2 ;
static const uint8_t A3  = PIN_A3 ;
static const uint8_t A4  = PIN_A4 ;
static const uint8_t A5  = PIN_A5 ;

#if defined(NRF52_SERIES)
#define ADC_RESOLUTION    14
#else
#define ADC_RESOLUTION    10
#endif

/*
 * Serial interfaces
 */
// Serial
#define PIN_SERIAL_RX       (10)
#define PIN_SERIAL_TX       (9)

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 1

#define PIN_SPI_MISO         (21) // not conected
#define PIN_SPI_MOSI         (15)
#define PIN_SPI_SCK          (20)
#define PIN_SPI_SS           (CS_PIN)

static const uint8_t SS   = PIN_SPI_SS ;
static const uint8_t MOSI = PIN_SPI_MOSI ;
static const uint8_t MISO = PIN_SPI_MISO ;
static const uint8_t SCK  = PIN_SPI_SCK ;

/*
 * Wire Interfaces
 */
#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (28u)
#define PIN_WIRE_SCL         (3u)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

#ifdef __cplusplus
}
#endif

#endif
