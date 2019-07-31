/* This file defines the parameters and the interface for the GW5200 + AR0233 image
   sensor driver.
 */

#ifndef _INCLUDED_SENSOR_H_
#define _INCLUDED_SENSOR_H_

#include <cyu3types.h>

/* I2C Slave address for the GW5200. */
#define SENSOR_ADDR_WR 	   0xDA         /* Slave address used to write to sensor registers. */
#define SENSOR_ADDR_RD 	   0xDB         /* Slave address used to read from sensor registers. */

#define I2C_SLAVEADDR_MASK 0xFE         /* Mask to get actual I2C slave address value without direction bit. */
#define I2C_SLAVEADDR_READ_BIT	0x01

#define I2C_MEMORY_ADDR_WR 0xA0         /* I2C slave address used to write to an EEPROM. */
#define I2C_MEMORY_ADDR_RD 0xA1         /* I2C slave address used to read from an EEPROM. */

#define GW5_RESET_GPIO   20			    /* GW5 reset pin is low active */
#define SENSOR_TRIGGER_GPIO 19			/* the sensor trigger GPIO bypassed GW5 connected to AR0233 through SerDes*/

/* Function    : SensorWrite2B
   Description : Write two bytes of data to image sensor over I2C interface.
   Parameters  :
                 slaveAddr - I2C slave address for the sensor.
                 highAddr  - High byte of memory address being written to.
                 lowAddr   - Low byte of memory address being written to.
                 highData  - High byte of data to be written.
                 lowData   - Low byte of data to be written.
 */
extern CyU3PReturnStatus_t
SensorWrite2B (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t highData,
        uint8_t lowData);

/* Function    : SensorWrite
   Description : Write arbitrary amount of data to image sensor over I2C interface.
   Parameters  :
                 slaveAddr - I2C slave address for the sensor.
                 highAddr  - High byte of memory address being written to.
                 lowAddr   - Low byte of memory address being written to.
                 count     - Size of write data in bytes. Limited to a maximum of 64 bytes.
                 buf       - Pointer to buffer containing data.
 */
extern CyU3PReturnStatus_t
SensorWrite (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t count,
        uint8_t *buf);

/* Function    : SensorRead2B
   Description : Read 2 bytes of data from image sensor over I2C interface.
   Parameters  :
                 slaveAddr - I2C slave address for the sensor.
                 highAddr  - High byte of memory address being written to.
                 lowAddr   - Low byte of memory address being written to.
                 buf       - Buffer to be filled with data. MSB goes in byte 0.
 */
extern CyU3PReturnStatus_t
SensorRead2B (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t *buf);

/* Function    : SensorRead
   Description : Read arbitrary amount of data from image sensor over I2C interface.
   Parameters  :
                 slaveAddr - I2C slave address for the sensor.
                 highAddr  - High byte of memory address being written to.
                 lowAddr   - Low byte of memory address being written to.
                 count     = Size of data to be read in bytes. Limited to a max of 64.
                 buf       - Buffer to be filled with data.
 */
extern CyU3PReturnStatus_t
SensorRead (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t count,
        uint8_t *buf);

/* Function    : SensorInit
   Description : Initialization of AR0233 + GW5200.
   Parameters  : None
 */
extern void
SensorInit (
        void);

/* Function    : SensorReset
   Description : Reset the ISP and image sensor using FX3 GPIO.
   Parameters  : None
 */
extern void
SensorReset (
        void);

/* Function     : SensorInit_1080p_60fps
   Description  : Configure the GW5200+AR0233 sensor for 1080p 60fps video stream.
   Parameters   : None
 */
extern void
SensorInit_1080p_60fps (
        void);

/* Function    : SensorI2cBusTest
   Description : Test whether the GW5200 is connected on the I2C bus.
   Parameters  : None
 */
extern uint8_t
SensorI2cBusTest (
        void);

#endif /* _INCLUDED_SENSOR_H_ */

/*[]*/

