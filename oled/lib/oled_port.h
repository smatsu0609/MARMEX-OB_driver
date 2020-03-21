#ifndef _OLED_PORT_H_INCLUDED
#define _OLED_PORT_H_INCLUDED

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
//#include <wiringPi.h>

#define OLED_RESN_PIN			(24)
#define OLED_VCC_ON_PIN			(25)

//Porting
#define OLED_VCC_OFF()				oled_write_gpio(OLED_VCC_ON_PIN, 0)
#define OLED_VCC_ON()				oled_write_gpio(OLED_VCC_ON_PIN, 1)
#define OLED_RESET_ASSERT()			oled_write_gpio(OLED_RESN_PIN, 0)
#define OLED_RESET_RELEASE()		oled_write_gpio(OLED_RESN_PIN, 1)
#define OLED_SPI_OPEN()				oled_open_spi()
#define OLED_SPI_CLOSE()			oled_close_spi()
#define OLED_SPI_SEND(buf, len)		oled_spi_send(buf, len);
#define OLED_GPIO_INIT()			oled_init_gpio()
#define OLED_GPIO_DEINIT()			oled_deinit_gpio()
#define OLED_DELAY(msec)			usleep(msec*1000)


void oled_init_gpio(void);
void oled_deinit_gpio(void);
void oled_init_spi(void);
void oled_deinit_spi(void);
void oled_open_spi(void);
void oled_close_spi(void);
void oled_spi_send( uint8_t *buf, uint32_t len);
void oled_write_gpio( int pin, int level );

#endif
