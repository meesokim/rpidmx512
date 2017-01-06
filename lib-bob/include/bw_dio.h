/**
 * @file bw_dio.h
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BW_DIO_H_
#define BW_DIO_H_

#define BW_DIO_DEFAULT_SLAVE_ADDRESS	0x84	///< http://www.bitwizard.nl/wiki/index.php/Default_addresses

typedef enum {
	BW_DIO_PIN_IO0 = (1 << 0),	///< 0b00000001
	BW_DIO_PIN_IO1 = (1 << 1),	///< 0b00000010
	BW_DIO_PIN_IO2 = (1 << 2),	///< 0b00000100
	BW_DIO_PIN_IO3 = (1 << 3),	///< 0b00001000
	BW_DIO_PIN_IO4 = (1 << 4),	///< 0b00010000
	BW_DIO_PIN_IO5 = (1 << 5),	///< 0b00100000
	BW_DIO_PIN_IO6 = (1 << 6)	///< 0b01000000
} bw_spi_dio_Pin;

typedef enum {
	BW_DIO_FSEL_INPT = 0,
	BW_DIO_FSEL_OUTP = 1
} bw_spi_dio_FunctionSelect;

#endif /* BW_DIO_H_ */
