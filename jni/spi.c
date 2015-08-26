/**********************************************************
  SPI general purpose interface code for file system

  Written by Ankur Yadav (ankurayadav@gmail.com)

  This code is made available under the BSD license.
**********************************************************/

/**
 * @file spi.c
 * @author Ankur Yadav (ankurayadav@gmail.com)
 * @brief SPI general purpose interface code for file system
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "include/linux/spi/spidev.h"
#include "bbbandroidHAL.h"

#define MAX_PATH 50	/**< Maximum buffer for creating path using snprintf() */

#define MODE0 0		/**< Macro defination for MODE0 */
#define MODE1 1		/**< Macro defination for MODE1 */
#define MODE2 2		/**< Macro defination for MODE2 */
#define MODE3 3		/**< Macro defination for MODE3 */

/**
 * This function takes input SPI file descriptor, tx buffer array to transmit data,
 * rx buffer array to receive data and length of data to be transferred.
 * It then transfers and receives data using ioctl function call.
 * @param spiFD a constant integer argument.
 * @param tx a constant uint8_t array argument.
 * @param rx a constant uint8_t array argument.
 * @param len a constant integer argument.
 * @return 0 if successful and -1 if it fails.
 */

int spiTransfer(const int spiFD, const uint8_t tx[], const uint8_t rx[], const int len)
{
	struct spi_ioc_transfer	transfer;

	transfer.tx_buf = (unsigned long) tx;
	transfer.rx_buf = (unsigned long) rx;
	transfer.len = len;
	transfer.speed_hz = 10000;
	transfer.bits_per_word = 8;

	if(ioctl(spiFD, SPI_IOC_MESSAGE(1), &transfer)<0)
	{
		return -1;
	}

	return 0;
}

/**
 * It takes spi file descriptor and
 * register address as input and returns the value present at that address.
 * @param spiFD a constant integer argument.
 * @param regAdd a constant uint8_t argument.
 * @return value present at that address.
 */

int spiReadByte(const int spiFD, const uint8_t regAdd)
{
	unsigned char tx[2], rx[2];
	memset(tx, 0, sizeof tx);
	memset(rx, 0, sizeof rx);

	tx[0] = (unsigned char) (0x80 | regAdd);
	
	spiTransfer(spiFD, tx, rx, 2);

	return rx[1];
}

/**
 * It takes file descriptor, length of data to be read and starting address of register from which data is to be read and
 * returns a pointer to the array of data bytes read.
 * It is used to get 'length' number of bytes in array from 'startAdd' starting address.
 * @param spiFD a constant integer argument.
 * @param len a constant integer argument.
 * @param startAdd a constant uint8_t argument.
 * @return a pointer to the array of data bytes read.
 */

unsigned char* spiReadBytes(const int spiFD, const int len, const uint8_t startAdd)
{
	unsigned char* data = (unsigned char *)malloc(sizeof(unsigned char)*len);
	unsigned char tx[len+1], rx[len+1];
	memset(tx, 0, sizeof tx);
	tx[0] =  (unsigned char) (0x80 | 0x40 | startAdd);
	
	spiTransfer(spiFD, tx, rx, len+1);
	
	memcpy(data, rx+1, len);
	
	return data;
}

/**
 * This function takes file descriptor, address of register to write
 * and byte to be written as input and writes byte provided in the argument at the address provided.
 * @param spiFD a constant integer argument.
 * @param regAdd a constant uint8_t argument.
 * @param data a constant uint8_t argument.
 * @return 0
 */

int spiWriteRegByte(const int spiFD, const uint8_t regAdd, const uint8_t data)
{
	unsigned char tx[2], rx[2];
	memset(rx, 0, sizeof rx);

	tx[0] = (unsigned char) regAdd;
	tx[1] = data;

	spiTransfer(spiFD, tx, rx, 2);
	return 0;
}

/**
 * It takes spi file descriptor , array of data to be written and length of data to be written as input.
 * It writes length number of data bytes from the data buffer provided.
 * @param spiFD a constant integer argument.
 * @param data a constant uint8_t argument argument.
 * @param len a constant integer argument.
 * @return 0
 */

int spiWriteBytes(const int spiFD, const uint8_t data[], const int len)
{
	unsigned char null = 0x00;
	
	spiTransfer(spiFD, data, &null, len);
	
	return 0;
}

/**
 * It takes file descriptor and mode as input and sets the mode provided in the argument
 * to the device represented by file descriptor.
 * @param spiFD a constant integer argument.
 * @param mode a constant uint8_t argument.
 * @return 0 if successful and -1 if it fails.
 */

int spiSetMode(const int spiFD, const uint8_t mode)
{
	if (ioctl(spiFD, SPI_IOC_WR_MODE, &mode)==-1)
	{
		return -1;
	}

	if (ioctl(spiFD, SPI_IOC_RD_MODE, &mode)==-1)
	{
		return -1;
	}

	return 0;
}

/**
 * It takes file descriptor and speed as input and sets the speed provided in the argument
 * to the device represented by file descriptor.
 * @param spiFD a constant integer argument.
 * @param speed a constant uint32_t argument.
 * @return 0 if successful and -1 if it fails.
 */

int spiSetSpeed(const int spiFD, const uint32_t speed)
{
	if (ioctl(spiFD, SPI_IOC_WR_MAX_SPEED_HZ, &speed)==-1)
	{
		return -1;
	}

	if (ioctl(spiFD, SPI_IOC_RD_MAX_SPEED_HZ, &speed)==-1)
	{
		return -1;
	}

	return 0;
}

/**
 * It takes file descriptor and 'bpw' bits per word as input and sets the bpw provided in the argument
 * to the device represented by file descriptor.
 * @param spiFD a constant integer argument.
 * @param bpw a constant uint8_t argument.
 * @return 0 if successful and -1 if it fails.
 */

int spiSetBitsPerWord(const int spiFD, const uint8_t bpw)
{
	if (ioctl(spiFD, SPI_IOC_WR_BITS_PER_WORD, &bpw)==-1)
	{
		return -1;
	}

	if (ioctl(spiFD, SPI_IOC_RD_BITS_PER_WORD, &bpw)==-1)
	{
		return -1;
	}
	return 0;
}

/**
 * This function takes input bus number, device number, speed of device, mode of device and bits per word for the device
 * and internally opens the file and sets speed mode and bits per word for the device
 * using spiSetMode(), spiSetSpeed() and spiSetBitsPerWord() and then returns the file descriptor.
 * @param bus a constant unsigned integer argument.
 * @param device a constant uint8_t argument.
 * @param speed a constant uint32_t argument.
 * @param mode a constant uuint8_t argument.
 * @param bpw a constant uuint8_t argument.
 * @return If successful then SPI file descriptor is returned and if it fails then -1 is returned.
 */

int spiOpen(const uint8_t bus, const uint8_t device, const uint32_t speed, const uint8_t mode, const uint8_t bpw)    //bpw = pits per word
{
	char fsBuf[MAX_PATH] ;
	int spiFD ;

	snprintf(fsBuf, sizeof(fsBuf), "/dev/spidev%d.%d", bus, device);

	spiFD = open(fsBuf, O_RDWR);
  	
  	if (spiFD < 0) {
    	return -1;
  	}

  	if (spiSetMode(spiFD, mode)==-1) return -1;
	if (spiSetSpeed(spiFD, speed)==-1) return -1;
	if (spiSetBitsPerWord(spiFD, bpw)==-1) return -1;

  	return spiFD ;
}

/**
 * This function is used to close SPI file descriptor.
 * @param spiFD a constant integer argument.
 */

void spiClose(const int spiFD)
{
	close(spiFD);
}