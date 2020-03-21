#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oled_port.h"

static int spi_fd = -1;
static unsigned char spi_mode;
static unsigned char spi_bitsPerWord;
static unsigned int spi_speed;
static const char *spi_device = "/dev/spidev0.0";


void oled_init_gpio(void)
{
	int fd = 0;
	char portNum[8];
	char path[64];
	
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		perror("GPIO export open error.");
		exit(1);
	}

	sprintf( portNum, "%d", OLED_RESN_PIN );
	write( fd, portNum, strlen(portNum)+1);
	sprintf( portNum, "%d", OLED_VCC_ON_PIN );
	write( fd, portNum, strlen(portNum)+1 );

	close(fd);

	sprintf( path, "/sys/class/gpio/gpio%d/direction", OLED_RESN_PIN );
	fd = open( path, O_WRONLY );
	if (fd < 0) {
		perror("GPIO direction open error.");
		exit(1);
	}
	write(fd,"out",4);
	close(fd);
	
	sprintf( path, "/sys/class/gpio/gpio%d/direction", OLED_VCC_ON_PIN );
	fd = open( path, O_WRONLY );
	if (fd < 0) {
		perror("GPIO direction open error.");
		exit(1);
	}
	write(fd,"out",4);
	close(fd);
	
}

void oled_deinit_gpio(void)
{
	int fd = 0;
	char portNum[8];
	fd = open("/sys/class/gpio/unexport", O_WRONLY);

	if (fd < 0) {
		perror("GPIO unexport open error.");
		exit(1);
	}
	sprintf( portNum, "%d", OLED_RESN_PIN );
	write( fd, portNum, strlen(portNum)+1);
	sprintf( portNum, "%d", OLED_VCC_ON_PIN );
	write( fd, portNum, strlen(portNum)+1);
	
	close(fd);
}

void oled_write_gpio( int port, int level )
{
	int fd;
	char path[64];
	
	sprintf( path, "/sys/class/gpio/gpio%d/value", port );
	fd = open( path, O_WRONLY );
	if ( fd < 0 ) {
		perror("GPIO value open error.\n");
		exit(1);
	}

	if ( level ) {
		write(fd,"1",2);
	} else {
		write(fd,"0",2);
	}
	
	close(fd);
}

void oled_open_spi(void)
{
	int status_value = -1;
	
	if ( spi_fd != -1 ) {
		return;
	}

    //----- SET SPI MODE -----
    //SPI_MODE_0 (0,0) 	CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    //SPI_MODE_1 (0,1) 	CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_2 (1,0) 	CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_3 (1,1) 	CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
    spi_mode = SPI_MODE_0;
    
    //----- SET BITS PER WORD -----
    spi_bitsPerWord = 8;
    
    //----- SET SPI BUS SPEED -----
    spi_speed = 20000000;		//20MHz (50nS per bit) 

   	spi_fd = open(spi_device, O_RDWR);
    if (spi_fd < 0) {
        perror("Error - Could not open SPI device");
        exit(1);
	}

    status_value = ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode);
    if(status_value < 0)
    {
        perror("Could not set SPIMode (WR)...ioctl fail");
        exit(1);
    }

    status_value = ioctl(spi_fd, SPI_IOC_RD_MODE, &spi_mode);
    if(status_value < 0)
    {
      perror("Could not set SPIMode (RD)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord (WR)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord(RD)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (WR)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (RD)...ioctl fail");
      exit(1);
    }
}

void oled_close_spi(void)
{
	int status_value = 0;

	if ( spi_fd != -1 ) {
	    status_value = close(spi_fd);
	    spi_fd = -1;
	}

    if(status_value < 0)
    {
    	perror("Error - Could not close SPI device");
    	exit(1);
    }
}

void oled_spi_send( uint8_t *buf, uint32_t len)
{
	int retVal = -1;
	struct spi_ioc_transfer spi;

	memset(&spi, 0, sizeof(spi));
	
	spi.tx_buf        = (unsigned long)buf;
	spi.rx_buf        = 0;
	spi.len           = len;
	spi.delay_usecs   = 0;
	spi.speed_hz      = spi_speed;
	spi.bits_per_word = spi_bitsPerWord;
	spi.cs_change = 0;
	spi.pad = 0;		// fill .pad with 0

	retVal = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) ;
	if(retVal < 0)
	{
		perror("Error - Problem transmitting spi data..ioctl");
		exit(1);
	}

}

