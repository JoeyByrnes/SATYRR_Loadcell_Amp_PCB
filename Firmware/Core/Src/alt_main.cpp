#include <dwt_delay.hpp>
#include "alt_main.h"
#include "stm32g4xx_hal.h"
#include <Protocentral_ADS1220.hpp>
#include <string.h>
#include <stdio.h>
#include "main.h"

#define PGA 32                // Programmable Gain, confirm that the same as set_pga_gain
#define VREF 2.048            // Internal reference of 2.048V
#define VFSR VREF/PGA
#define FSR (((long int)1<<23)-1)

#define CAN_DEVICE_ID (uint8_t)20
#define CAN_MASTER_ID (uint8_t)0
#define CAN_DATA_SIZE 8    // Number of data bytes in a CAN packet

uint8_t* raw1;
uint8_t* raw2;
uint8_t* raw3;
uint8_t* raw4;
uint8_t* raw5;
uint8_t* raw6;


bool first_reading = true;

Protocentral_ADS1220 ads1;
Protocentral_ADS1220 ads2;
Protocentral_ADS1220 ads3;

extern UART_HandleTypeDef huart2;

extern SPI_HandleTypeDef hspi1;

extern DMA_HandleTypeDef hdma_usart2_tx;

extern FDCAN_HandleTypeDef hfdcan1;

int cnt1,cnt2,cnt3,cnt4,cnt5,cnt6;
uint32_t reading_cnt = 0;

int alt_main()
{
	/* Initialization */

	ads1.begin(GPIOA, GPIO_PIN_2, GPIOA, GPIO_PIN_9);
	ads2.begin(GPIOA, GPIO_PIN_3, GPIOB, GPIO_PIN_0);
	ads3.begin(GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_8);

	ads1.set_OperationMode(MODE_TURBO);
	ads2.set_OperationMode(MODE_TURBO);
	ads3.set_OperationMode(MODE_TURBO);

	ads1.set_data_rate(DR_1000SPS);
	ads2.set_data_rate(DR_1000SPS);
	ads3.set_data_rate(DR_1000SPS);

	ads1.set_pga_gain(PGA_GAIN_128);
	ads2.set_pga_gain(PGA_GAIN_128);
	ads3.set_pga_gain(PGA_GAIN_128);

	ads1.set_conv_mode_single_shot();
	ads2.set_conv_mode_single_shot();
	ads3.set_conv_mode_single_shot();

	cnt1 = 0;
	cnt2 = 0;
	cnt3 = 0;
	cnt4 = 0;
	cnt5 = 0;
	cnt6 = 0;

	while (1)
	{


	}
}


int serial_cnt = 0;

void main_1KHz_interrupt()
{

	ads1.select_mux_channels(MUX_AIN0_AIN1);
	ads1.Start_SingleShot_Conv();

	ads2.select_mux_channels(MUX_AIN0_AIN1);
	ads2.Start_SingleShot_Conv();

	ads3.select_mux_channels(MUX_AIN0_AIN1);
	ads3.Start_SingleShot_Conv();

	usleep(550);
	raw2 = ads1.Read_Data_Immediate();
	raw4 = ads2.Read_Data_Immediate();
	raw6 = ads3.Read_Data_Immediate();

	ads1.select_mux_channels(MUX_AIN3_AIN2);
	ads1.Start_SingleShot_Conv();

	ads2.select_mux_channels(MUX_AIN3_AIN2);
	ads2.Start_SingleShot_Conv();

	ads3.select_mux_channels(MUX_AIN3_AIN2);
	ads3.Start_SingleShot_Conv();

	usleep(550);

	raw1 = ads1.Read_Data_Immediate();
	raw3 = ads2.Read_Data_Immediate();
	raw5 = ads3.Read_Data_Immediate();

	// Prepare the first CAN packet with the first 4 uint16_t values
	FDCAN_TxHeaderTypeDef txHeader;

	txHeader.Identifier = CAN_MASTER_ID;
	txHeader.IdType = FDCAN_STANDARD_ID;
	txHeader.TxFrameType = FDCAN_DATA_FRAME;
	txHeader.DataLength = FDCAN_DLC_BYTES_8;
	txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	txHeader.BitRateSwitch = FDCAN_BRS_OFF;
	txHeader.FDFormat = FDCAN_CLASSIC_CAN;
	txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	txHeader.MessageMarker = 0;

	uint8_t packet0[8]; // Packet 0 with 8 bytes
	uint8_t packet1[8]; // Packet 1 with 8 bytes
	uint8_t packet2[8]; // Packet 2 with 8 bytes

	packet0[0] = CAN_DEVICE_ID;
	packet0[1] = 0;
	packet0[2] = raw1[0];
	packet0[3] = raw1[1];
	packet0[4] = raw1[2];
	packet0[5] = raw2[0];
	packet0[6] = raw2[1];
	packet0[7] = raw2[2];

	packet1[0] = CAN_DEVICE_ID;
	packet1[1] = 1;
	packet1[2] = raw3[0];
	packet1[3] = raw3[1];
	packet1[4] = raw3[2];
	packet1[5] = raw4[0];
	packet1[6] = raw4[1];
	packet1[7] = raw4[2];

	packet2[0] = CAN_DEVICE_ID;
	packet2[1] = 2;
	packet2[2] = raw5[0];
	packet2[3] = raw5[1];
	packet2[4] = raw5[2];
	packet2[5] = raw6[0];
	packet2[6] = raw6[1];
	packet2[7] = raw6[2];


	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, packet0);
	usleep(130);
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, packet1);
	usleep(130);
	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, packet2);
	usleep(130);

}
