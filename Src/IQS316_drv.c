/******************************************************************************
*                                                                             *
*                            Module specification                             *
*                                                                             *
*                                Copyright by                                 *
*                                                                             *
*                              HOYEN TECH Co., Ltd                            *
*                                                                             *
*******************************************************************************
Name             :  IQS316_driver.c
Description      :  STM32M0 specific functions for IQS316 I2C Firmware library
*******************************************************************************/

#include "iqs316_driver.h"
#include "stdio.h"
#include "hwcfg.h"
#include "string.h"

#define DEBUG_IQS316
extern SPI_HandleTypeDef hspi1;
uint8_t write = 0x00, read = 0x01, FE = 0xFE, FF = 0xFF;
struct tIQS316 IQS316;

/*****************************************************************************
//
//! Initialise
//!
//! Initializes the STM32F0 I2C
//!
//! \param None
//!
//! \return None
//
*****************************************************************************/
void IQS316_Init(void)
{
	unsigned char temp;
	MCLR_LOW();
	HAL_Delay(1);

	MCLR_HIGH();
	SPI1_SS_HIGH();

	temp = CommsIQS316_RxTx(FF);
	while (temp != 0xFF)
	{
		temp = CommsIQS316_RxTx(FF);
	}
	temp = CommsIQS316_RxTx(FF);

	HAL_Delay(20);

	IQS316_Settings();
}


void IQS316_Settings(void)
{
	uint8_t ui8StartGroup, ui8CurrentGroup;
	uint8_t ui8ProdNo, ui8VersionNo;
	uint8_t ui8DataArray[20], readataArray[20];
	memset(ui8DataArray, 0xFF, 20);
	memset(readataArray, 0xFF, 20);

	//
	// Confirm comms are working correctly, and also that expected IQS316
	// IC version is used.  Do this by reading back the Product and Version
	// numbers from the IQS316
	//
	IQS316_Read(PROD_NUM, ui8DataArray, 2);
	ui8ProdNo = ui8DataArray[0];
	//IQS316_Read(VERSION_NUM, ui8DataArray,1);
	ui8VersionNo = ui8DataArray[1];
	IQS316_End_Comms_Window();

#ifdef DEBUG_IQS316
	if ((ui8ProdNo != 27) || (ui8VersionNo != 1))
	{
		// Error condition, handle this here
		// (fix comms or get correct IQS316 version)
		//
		printf("\n\r ui8ProdNo %d ui8VersionNo %d error  \n\r", ui8ProdNo, ui8VersionNo);
		while (1);
	}
	else
#endif
		//
		// Acknowledge the reset by sending an ACK_RESET to the IQS316.  This will
		// clear the SHOW_RESET bit in UI_FLAGS0 register.  From here on further, if
		// this SHOW_RESET bit ever becomes set, we know an unexpected reset has
		// occurred on the IQS316, and we should repeat the setup
		//
		ui8DataArray[0] = (ACK_RESET | LTN_DISABLE | WDT_DISABLE);
	IQS316_Write(PROX_SETTINGS_2, ui8DataArray, 1);

#ifdef DEBUG_IQS316
	IQS316_Read(UI_FLAGS0, ui8DataArray, 1);
	IQS316_End_Comms_Window();

	if ((ui8DataArray[0] & SHOW_RESET) != 0)
	{
		// The show reset bit should be cleared after writing the ACK_RESET
		// previously.  Check write procedures, and make sure comms window is
		// closed after sending ACK_RESET.
		while (1);
	}
#endif
	//
	// IQS316 Application specific SETUP
	// 1 - CHANNEL SETUPn00
	//
	ui8DataArray[0] = 0x03;         // CHAN_ACTIVE0
	ui8DataArray[1] = 0x0F;         // CHAN_ACTIVE1
	ui8DataArray[2] = 0x0F;         // CHAN_ACTIVE2
	ui8DataArray[3] = 0x0F;         // CHAN_ACTIVE3
	ui8DataArray[4] = 0x0F;         // CHAN_ACTIVE4

	IQS316_Write(CHAN_ACTIVE0, ui8DataArray, 5);
	IQS316_End_Comms_Window();
	//
	// 2 - Setup ATI and thresholds (settings which must be sent in specific
	// comms window - depending which group is active)
	//

	IQS316_Read(GROUP_NUM, ui8DataArray, 1);
	ui8StartGroup = ui8DataArray[0];

	//
	// Enable skip conversions, so that IQS316 cycles through the the groups
	// 0, 1, 2, 3, 4, 0, 1, ....   to allow configuring settings which must be
	// setup while in a specific cycle.
	//
	ui8DataArray[0] = (SKIP_CONV | LTN_DISABLE | WDT_DISABLE);
	IQS316_Write(PROX_SETTINGS_2, ui8DataArray, 1);
	ui8CurrentGroup = ui8StartGroup;

	do
	{
		switch (ui8CurrentGroup)
		{
		case 0:
		{
			// ATI C and ATI I settings
			//
			ui8DataArray[0] = 0x00;          // ATI_MULT1
			ui8DataArray[1] = 0x00;          // ATI_MULT2
			IQS316_Write(ATI_MULT1, ui8DataArray, 2);
			//
			// Set thresholds (in upper nibble of LTA)
			// NOTE: this will overwrite the LTA value also, but auto-ATI
			// will be done later, which will reseed the LTAs correctly
			//
			ui8DataArray[0] = PROX_THRES_8;  // LTA_04_HI
			ui8DataArray[1] = 0x00;          // low byte - irrelevant
			ui8DataArray[2] = PROX_THRES_8;  // LTA_15_HI
			ui8DataArray[3] = 0x00;          // low byte - irrelevant
			ui8DataArray[4] = PROX_THRES_8;  // LTA_26_HI
			ui8DataArray[5] = 0x00;          // low byte - irrelevant
			ui8DataArray[6] = PROX_THRES_8;  // LTA_37_HI
			IQS316_Write(LTA_04_HI, ui8DataArray, 7);
			break;
		}
		case 1:
		{
			ui8DataArray[0] = 0x00;          // ATI_MULT1
			ui8DataArray[1] = 0x00;          // ATI_MULT2
			IQS316_Write(ATI_MULT1, ui8DataArray, 2);

			ui8DataArray[0] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_04_HI
			ui8DataArray[1] = 0x00;          // low byte - irrelevant
			ui8DataArray[2] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_15_HI
			ui8DataArray[3] = 0x00;          // low byte - irrelevant
			ui8DataArray[4] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_26_HI
			ui8DataArray[5] = 0x00;          // low byte - irrelevant
			ui8DataArray[6] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_37_HI
			IQS316_Write(LTA_04_HI, ui8DataArray, 7);
			break;
		}
		case 2:
		{
			ui8DataArray[0] = 0x00;          // ATI_MULT1
			ui8DataArray[1] = 0x00;          // ATI_MULT2
			IQS316_Write(ATI_MULT1, ui8DataArray, 2);

			ui8DataArray[0] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_04_HI
			ui8DataArray[1] = 0x00;          // low byte - irrelevant
			ui8DataArray[2] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_15_HI
			ui8DataArray[3] = 0x00;          // low byte - irrelevant
			ui8DataArray[4] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_26_HI
			ui8DataArray[5] = 0x00;          // low byte - irrelevant
			ui8DataArray[6] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_37_HI
			IQS316_Write(LTA_04_HI, ui8DataArray, 7);
			break;
		}
		case 3:
		{
			ui8DataArray[0] = 0x00;          // ATI_MULT1
			ui8DataArray[1] = 0x00;          // ATI_MULT2
			IQS316_Write(ATI_MULT1, ui8DataArray, 2);

			ui8DataArray[0] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_04_HI
			ui8DataArray[1] = 0x00;          // low byte - irrelevant
			ui8DataArray[2] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_15_HI
			ui8DataArray[3] = 0x00;          // low byte - irrelevant
			ui8DataArray[4] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_26_HI
			ui8DataArray[5] = 0x00;          // low byte - irrelevant
			ui8DataArray[6] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_37_HI
			IQS316_Write(LTA_04_HI, ui8DataArray, 7);
			break;
		}
		case 4:
		{
			ui8DataArray[0] = 0x00;          // ATI_MULT1
			ui8DataArray[1] = 0x00;          // ATI_MULT2
			IQS316_Write(ATI_MULT1, ui8DataArray, 2);

			ui8DataArray[0] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_04_HI
			ui8DataArray[1] = 0x00;          // low byte - irrelevant
			ui8DataArray[2] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_15_HI
			ui8DataArray[3] = 0x00;          // low byte - irrelevant
			ui8DataArray[4] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_26_HI
			ui8DataArray[5] = 0x00;          // low byte - irrelevant
			ui8DataArray[6] = PROX_THRES_20 | TOUCH_THRES_3_16;  // LTA_37_HI
			IQS316_Write(LTA_04_HI, ui8DataArray, 7);
			break;
		}
		}
		IQS316_End_Comms_Window();

		IQS316_Read(GROUP_NUM, ui8DataArray, 1);
		ui8CurrentGroup = ui8DataArray[0];
	} while (ui8CurrentGroup != ui8StartGroup);
	//
	// Now Group specific settings are done, so disable the skip conversions
	//
	ui8DataArray[0] = (LTN_DISABLE | WDT_DISABLE);
	IQS316_Write(PROX_SETTINGS_2, ui8DataArray, 1);
	//
	// Set the high/low settings for prox and touch thresholds
	//
	ui8DataArray[0] = (PROX_THRES_RANGE | ND);
	IQS316_Write(UI_SETTINGS0, ui8DataArray, 1);
	//
	// Set ATI Target - For Prox Mode
	//
	ui8DataArray[0] = 0x03;
	ui8DataArray[1] = 0x20;
	IQS316_Write(AUTO_ATI_TARGET_HI, ui8DataArray, 2);
	IQS316_End_Comms_Window();
	//
	// Perform automated ATI routine (to setup ATI Compensation values)
	// NOTE: ATI_MODE already set to ProxMode, no need to configure.
	//
	ui8DataArray[0] = CXVSS | HALT0 | AUTO_ATI | CXDIV1;
	IQS316_Write(PROX_SETTINGS_1, ui8DataArray, 1);
	IQS316_End_Comms_Window();
	//
	// Read ATI Busy flag until it clears, then ProxMode ATI is done
	//
	do
	{
		IQS316_Read(UI_FLAGS0, ui8DataArray, 1);
		IQS316_End_Comms_Window();
	} while ((ui8DataArray[0] & ATI_BUSY) != 0);
	//
	// Perform ATI for Touch Mode
	// Set ATI_MODE to Touch
	//
	ui8DataArray[0] = ATI_MODE | PROX_THRES_RANGE | ND;
	IQS316_Write(UI_SETTINGS0, ui8DataArray, 1);
	IQS316_End_Comms_Window();
	//
	// Set ATI Target - For Touch Mode
	//
	ui8DataArray[0] = 0x03;
	ui8DataArray[1] = 0x20;
	IQS316_Write(AUTO_ATI_TARGET_HI, ui8DataArray, 2);
	IQS316_End_Comms_Window();
	//
	// Perform automated ATI routine (to setup ATI Compensation values)
	//
	ui8DataArray[0] = CXVSS | HALT0 | AUTO_ATI | CXDIV1;
	IQS316_Write(PROX_SETTINGS_1, ui8DataArray, 1);
	IQS316_End_Comms_Window();
	//
	// Read ATI Busy flag until it clears, then ATI is done
	//
	do
	{
		IQS316_Read(UI_FLAGS0, ui8DataArray, 1);
		IQS316_End_Comms_Window();

	} while ((ui8DataArray[0] & ATI_BUSY) != 0);

	printf("\n\r IQS316 is Ready ! \n\r");
	//
	// Now setup the advanced settings as required by the design, such as the
	// following:  Low-Power, charging mode, eventMode
	//
}

/*****************************************************************************
//
//! Obtain new data from the IQS316
//!
//! This fucntion reads the group number  to determine whether the IQS316 is in
//! PROX mode or TOUCH mode. If the group number is 0, PROX mode, the
//! prox_detected flag is cleared and the routine finishes. If the group number
//! is not 0, with Auto-Mode assumed, then the prox_detected flag is set.  TOUCH
//! and PROX status registers for each group are read and stored in the global
//! data structures.
//!
//! \param None
//!
//! \return None
//
*****************************************************************************/
void IQS316_Refresh_Data(void)
{
	uint8_t ui8CurrentGroup, ui8TempTouch, ui8TempProx, ui8TempUIFlags0;
	uint8_t TouchNumber;

	IQS316_Read(UI_FLAGS0, &ui8TempUIFlags0, 1);
	IQS316_Read(PROX_STAT, &ui8TempProx, 1);
	IQS316_Read(TOUCH_STAT, &ui8TempTouch, 1);
	IQS316_Read(GROUP_NUM, &ui8CurrentGroup, 1);
	IQS316_End_Comms_Window();

	// Update the specific groups data
	switch (ui8CurrentGroup)
	{
	case 1:
		IQS316.touch4_11 &= 0xFC;
		IQS316.touch4_11 |= (ui8TempTouch);
		break;
	case 2:
		IQS316.touch4_11 &= 0xF3;
		IQS316.touch4_11 |= (ui8TempTouch << 2);
		break;
	case 3:
		IQS316.touch4_11 &= 0xCF;
		IQS316.touch4_11 |= (ui8TempTouch << 4);
		break;
	case 4:
		IQS316.touch4_11 &= 0x3F;
		IQS316.touch4_11 |= (ui8TempTouch << 6);
		break;
	}
}

/*****************************************************************************
//
//! Process the new data from the IQS316
//!
//! Function to be called after obtaining new data from the IQS316
//! This function processes the data contained in the IQS316 struct, and is
//! only done here as an example.  The channel number that is in touch is
//! displayed on 4 I/O's (channel 4-19 displayed as value 0-15 on PD7..4)
//!
//! \param None
//!
//! \return None
//
*****************************************************************************/
void IQS316_Process_Data(void)
{
	uint16_t CheckNumber = 0x01, i;
	uint8_t TouchNumber;
	if (IQS316.touch4_11 != 0)
	{
		printf("\n\r Touch ");
		for (CheckNumber = 0x01, i = 0; i < 8; i++, CheckNumber <<= 1)
		{
			if (CheckNumber&IQS316.touch4_11)
				printf("%d  ", i + 1);
		}
	}
}

/*****************************************************************************
//
//! Perform an I2C read from the IQS316 slave device
//!
//! The function waits for RDY to be set, to show a comms window is active, and
//! then gives a START, ADR+WRITE, ADR, REPEAT-START, ADR+READ, DATA1..DATA2..
//! NOTE: no i2c STOP is given, since this is handled seperately to control the
//! IQS316 comms window.
//!
//! \param ui8Address is the address on the IQS316 from which to read
//! \param ui8Data[] is the array where the read data is packed into
//! \param ui8Length is the number of bytes to read from the IQS316
//!
//! \return None
//
*****************************************************************************/
void IQS316_Read(uint8_t ui8Address, uint8_t *ui8Data, uint8_t ui8Length)
{
	uint8_t ui8Header;

	ui8Header = CommsIQS316_RxTx(read);
	while (ui8Header != 0xFF)
	{
		Comms_Error();
		ui8Header = CommsIQS316_RxTx(read);
	}
	ui8Header = CommsIQS316_RxTx(ui8Address);
	for (int i = 0; i < ui8Length; i++)
	{
		ui8Data[i] = CommsIQS316_RxTx(FE);
	}
	ui8Header = CommsIQS316_RxTx(FF);
}

/*****************************************************************************
//
//! Perform an I2C write to the IQS316 slave device
//!
//! The function waits for RDY to be set, to show a comms window is active, and
//! then gives a START, ADR+WRITE, ADR, DATA1..DATA2..
//! NOTE: no i2c STOP is given, since this is handled seperately to control the
//! IQS316 comms window.
//!
//! \param ui8Address is the address on the IQS316 where the data is to be written
//! \param ui8Data[] is the array where the data is pre-loaded
//! \param ui8Length is the number of bytes to write to the IQS316
//!
//! \return None
//
*****************************************************************************/
void IQS316_Write(uint8_t ui8Address, uint8_t *ui8Data, uint8_t ui8Length)
{
	uint8_t ui8Header, rui8Data[10], newaddress;

	ui8Header = CommsIQS316_RxTx(write);
	while (ui8Header != 0xFF)
	{
		Comms_Error();
		ui8Header = CommsIQS316_RxTx(write);
	}
	for (int i = 0; i < ui8Length; i++)
	{
		newaddress = ui8Address + i;
		ui8Header = CommsIQS316_RxTx(newaddress);
		ui8Header = CommsIQS316_RxTx(ui8Data[i]);
	}
	ui8Header = CommsIQS316_RxTx(0xFF);
}


/*****************************************************************************
//
//! End the comms window with the IQS316
//!
//! The IQS316 comms window is ended by sending an I2C STOP condition.  This
//! function then also waits for RDY to go LOW to make sure a following comms
//! does not catch the RDY high just before it is cleared by the IQS316.
//!
//! \param None
//!
//! \return None
//
*****************************************************************************/
void IQS316_End_Comms_Window(void)
{
	uint8_t ui8Header;

	ui8Header = CommsIQS316_RxTx(write);

	while (ui8Header != 0xFF)
	{
		Comms_Error();
		ui8Header = CommsIQS316_RxTx(write);
	}
	//  {
	//    printf("\n\r CWStart=>%d \n\r",ui8Header);
	//  }
	//	else
	//		 printf("\n\r ECW=T \n\r");

	ui8Header = CommsIQS316_RxTx(FE);
	//	if (ui8Header != 0x00)
	//    printf("\n\r CWEnd=%d \n\r",ui8Header);


	//	printf("\n\r IQS316_End_Comms_Window Wait for RDY to go LOW \n\r");
	//  while(RRDY()!= GPIO_PIN_RESET){}
	//  printf("\n\r IQS316_End_Comms_Window  RDY is LOW \n\r");
}

uint8_t CommsIQS316_RxTx(uint8_t ui8SendData)
{
	uint8_t ui8ReceiveData, i;

	while (RRDY() == GPIO_PIN_RESET){}
	SPI1_SS_LOW();
	i = HAL_SPI_TransmitReceive_IT(&hspi1, &ui8SendData, &ui8ReceiveData, 1);
	SPI1_SS_HIGH();
	if (i != 0)
	{
		printf("\n\r CommsIQS316_RxTx i = %d \n\r", i);
		return i;
	}
	else
		return ui8ReceiveData;
}


void Comms_Error(void)
{
	uint8_t ui8Header;
	//	printf("\n\r Comms_Error \n\r");

	ui8Header = CommsIQS316_RxTx(0xFF);

	ui8Header = CommsIQS316_RxTx(FF);
	while (ui8Header != 0xFF)
	{
		ui8Header = CommsIQS316_RxTx(FF);
	}
	ui8Header = CommsIQS316_RxTx(FF);
	HAL_Delay(20);
}

