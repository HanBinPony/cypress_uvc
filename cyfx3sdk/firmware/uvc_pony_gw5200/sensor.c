/*
 ## Cypress FX3 Camera Kit source file (sensor.c)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2012,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3dma.h>
#include <cyu3error.h>
#include <cyu3uart.h>
#include <cyu3i2c.h>
#include <cyu3types.h>
#include <cyu3gpio.h>
#include <cyu3utils.h>
#include "sensor.h"

/* This function inserts a delay between successful I2C transfers to prevent
   false errors due to the slave being busy.
 */
static void
SensorI2CAccessDelay (
        CyU3PReturnStatus_t status)
{
    /* Add a 10us delay if the I2C operation that preceded this call was successful. */
    if (status == CY_U3P_SUCCESS)
        CyU3PBusyWait (10);
}

/* Write to an I2C slave with two bytes of data. */
CyU3PReturnStatus_t
SensorWrite2B (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t highData,
        uint8_t lowData)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PI2cPreamble_t  preamble;
    uint8_t buf[2];

    /* Set the parameters for the I2C API access and then call the write API. */
    preamble.buffer[0] = slaveAddr & I2C_SLAVEADDR_MASK;
    preamble.buffer[1] = highAddr;
    preamble.buffer[2] = lowAddr;
    preamble.length    = 3;             /*  Three byte preamble. */
    preamble.ctrlMask  = 0x0000;        /*  No additional start and stop bits. */

    buf[0] = highData;
    buf[1] = lowData;

    apiRetStatus = CyU3PI2cTransmitBytes (&preamble, buf, 2, 0);
    SensorI2CAccessDelay (apiRetStatus);

    return apiRetStatus;
}

CyU3PReturnStatus_t
SensorWrite (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t count,
        uint8_t *buf)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PI2cPreamble_t preamble;

    if (count > 64)
    {
        CyU3PDebugPrint (4, "ERROR: SensorWrite count > 64\r\n");
        return 1;
    }

    /* Set up the I2C control parameters and invoke the write API. */
    preamble.buffer[0] = slaveAddr & I2C_SLAVEADDR_MASK;
    preamble.buffer[1] = highAddr;
    preamble.buffer[2] = lowAddr;
    preamble.length    = 3;
    preamble.ctrlMask  = 0x0000;

    apiRetStatus = CyU3PI2cTransmitBytes (&preamble, buf, count, 0);
    SensorI2CAccessDelay (apiRetStatus);

    return apiRetStatus;
}

CyU3PReturnStatus_t
SensorRead2B (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t *buf)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PI2cPreamble_t preamble;

    preamble.buffer[0] = slaveAddr & I2C_SLAVEADDR_MASK;        /*  Mask out the transfer type bit. */
    preamble.buffer[1] = highAddr;
    preamble.buffer[2] = lowAddr;
    preamble.buffer[3] = slaveAddr | I2C_SLAVEADDR_READ_BIT;
    preamble.length    = 4;
    preamble.ctrlMask  = 0x0004;                                /*  Send start bit after third byte of preamble. */

    apiRetStatus = CyU3PI2cReceiveBytes (&preamble, buf, 2, 0);
    SensorI2CAccessDelay (apiRetStatus);

    return apiRetStatus;
}

CyU3PReturnStatus_t
SensorRead (
        uint8_t slaveAddr,
        uint8_t highAddr,
        uint8_t lowAddr,
        uint8_t count,
        uint8_t *buf)
{
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PI2cPreamble_t preamble;

    if ( count > 64 )
    {
        CyU3PDebugPrint (4, "ERROR: SensorWrite count > 64\r\n");
        return 1;
    }

    preamble.buffer[0] = slaveAddr & I2C_SLAVEADDR_MASK;        /*  Mask out the transfer type bit. */
    preamble.buffer[1] = highAddr;
    preamble.buffer[2] = lowAddr;
    preamble.buffer[3] = slaveAddr | I2C_SLAVEADDR_READ_BIT;
    preamble.length    = 4;
    preamble.ctrlMask  = 0x0004;                                /*  Send start bit after third byte of preamble. */

    apiRetStatus = CyU3PI2cReceiveBytes (&preamble, buf, count, 0);
    SensorI2CAccessDelay (apiRetStatus);

    return apiRetStatus;
}

/*
 * Verify that the sensor can be accessed over the I2C bus from FX3.
 */
uint8_t
SensorI2cBusTest (
        void)
{
#if 0
    W5ComReturn_e ret;

	//uint8_t data;
    uint8_t msg_id;

    // HANBIN: cypress FX3 does not support preamble > 8 byte, therefore w5comapi cannot work with FX3 I2C
	ret = W5comCallApi(API_CODE_GET_GW5_STATE, 0, NULL, 0x8000, &msg_id);
	if (ret != w5_com_ret_ok) {
		CyU3PDebugPrint (4, "failed read gw5 state!\r\n");
	} else {
		CyU3PDebugPrint (4, "successfully read state 0x%x!\r\n", msg_id);
	}
	CyU3PThreadSleep (200);


	if (SensorRead (0x90, 0x00, 0x00, 1, &data) == CY_U3P_SUCCESS) {
		CyU3PDebugPrint (4, "successfully read max9296 0x%x 0x%x!\r\n", 0x00, data);
	} else {
		CyU3PDebugPrint (4, "failed read max9296!\r\n");
	}
	CyU3PThreadSleep (200);

    if (ret != w5_com_ret_ok) {
    	return CY_U3P_ERROR_INVALID_DEV;
    }
#endif
    return CY_U3P_SUCCESS;
}


/*
 * Reset the GW5 through GPIO
 */
void
SensorReset (
        void)
{
    CyU3PReturnStatus_t apiRetStatus;

    /* Drive the GPIO low to reset the sensor. */
    apiRetStatus = CyU3PGpioSetValue (GW5_RESET_GPIO, CyFalse);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "GPIO Set Value Error, Error Code = %d\r\n", apiRetStatus);
        return;
    }

    /* Wait for 20ms to allow proper reset. */
    CyU3PThreadSleep (20);

    /* Drive the GPIO high to bring the sensor out of reset. */
    apiRetStatus = CyU3PGpioSetValue (GW5_RESET_GPIO, CyTrue);
    if (apiRetStatus != CY_U3P_SUCCESS)
    {
        CyU3PDebugPrint (4, "GPIO Set Value Error, Error Code = %d\r\n", apiRetStatus);
        return;
    }

    /* Delay 20ms to allow the sensor powers up. */
    CyU3PThreadSleep (20);

    return;
}

/* sensor initialization sequence. */
void
SensorInit (
        void)
{
    if (SensorI2cBusTest () != CY_U3P_SUCCESS)        /* Verify that the sensor is connected. */
    {
        CyU3PDebugPrint (4, "Error: Reading Sensor ID failed!\r\n");
        return;
    }
}
