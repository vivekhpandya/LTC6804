/**
 * LTC6804.h
 * Contains functions for communication with LTC6804
 * Author : vivekhpandya
 */
#include "stm32f1xx_hal.h"

extern SPI_HandleTypeDef hspi1;

/* ====== Already Declared ====== */
#ifdef LTC6804_H
//Has already been defined

/* ====== Functions importing ====== */
extern void measure(void);
extern void wakeup(void);
extern void init_PEC15_Table(void);
extern uint16_t pec15(uint8_t *data , uint16_t len);
extern uint8_t Read_Command(uint16_t Cmd_code, uint8_t *Rec_Data_Ptr, uint8_t Rec_Data_Length);
extern uint8_t Read_Command_Chain(uint16_t Cmd_code, uint8_t *Rec_Data_Ptr, uint8_t Rec_Data_Length, uint8_t No_of_ICs);
extern void Write_Command(uint16_t Cmd_code, uint8_t *Trans_Data_Ptr, uint8_t Trans_Data_Length);
extern void Write_Command_Chain(uint16_t Cmd_code, uint8_t *Trans_Data_Ptr, uint8_t Trans_Data_Length, uint8_t No_of_ICs);
extern uint8_t Run_Command(uint16_t Cmd_code);
extern void DummyByte(void);
//extern uint16_t CRC_Calculate(uint16_t,uint8_t);
extern void delay_1us(void);
extern void delay_5us(void);
extern void delay_10us(uint8_t);
extern void delay_100us(uint8_t);

/* ====== Variable importing ====== */
extern uint16_t pec15Table[256];

/* ====== Error Arrays importing ====== */
extern uint8_t err_pecmismatch[8];

#endif

/* ====== Fresh Declarations ====== */
#ifndef LTC6804_H
#define LTC6804_H			//Not defined yet

/* ====== Functions declaration ====== */
void measure(void);
void wakeup(void);
void init_PEC15_Table(void);
uint16_t pec15(uint8_t *data , uint16_t len);
uint8_t Read_Command(uint16_t Cmd_code, uint8_t *Rec_Data_Ptr, uint8_t Rec_Data_Length);
uint8_t Read_Command_Chain(uint16_t Cmd_code, uint8_t *Rec_Data_Ptr, uint8_t Rec_Data_Length, uint8_t No_of_ICs);
void Write_Command(uint16_t Cmd_code, uint8_t *Trans_Data_Ptr, uint8_t Trans_Data_Length);
void Write_Command_Chain(uint16_t Cmd_code, uint8_t *Trans_Data_Ptr, uint8_t Trans_Data_Length, uint8_t No_of_ICs);
uint8_t Run_Command(uint16_t Cmd_code);
void DummyByte(void);
//uint16_t CRC_Calculate(uint16_t,uint8_t);
void delay_1us(void);
void delay_5us(void);
void delay_10us(uint8_t);
void delay_100us(uint8_t);

/* ====== Variable Declaration ====== */
uint16_t pec15Table[256];

/* ====== Error Arrays ====== */
uint8_t err_pecmismatch[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

#endif

/* ====== Error Constants ====== */
#define ERR_SUCCESS			0x00
#define ERR_PECMISMATCH1	0x01
#define ERR_PECMISMATCH2	0x02
#define ERR_PECMISMATCH3	0x04
#define ERR_PECMISMATCH4	0x08
#define ERR_PECMISMATCH5	0x10
#define ERR_PECMISMATCH6	0x20
#define ERR_PECMISMATCH7	0x40
#define ERR_PECMISMATCH8	0x80

/* ====== Other Constants ====== */
#define PEC_SEED	(uint16_t)0x0010		//PEC seed value in binary is 0-000 0000 0001 0000
#define Poly		(uint16_t)0x4599		//Polynomial is x15 + x14 + x10 + x8 + x7 + x4 + x3 + 1 i.e. in binary 0-100 0101 1001 1001
#define nPoly		~Poly
#define PEC_MSB		0x4000
#define STREAM_LEN	16

/* ====== Command related ====== */
#define LTC6804_COMMAND_LEN 4
#define LTC6804_DATA_LEN 8
#define LTC6804_CALC_BUFFER_LEN(max_modules) (LTC6804_COMMAND_LEN+LTC6804_DATA_LEN*max_modules)


///* =================== TIMING MACROS ==================
//
//#define T_REFUP 5
//#define T_SLEEP 1800
//#define T_IDLE 4
//#define T_WAKE 1

/* =================== COMMAND CODES ================== */

#define WRCFG	0x001
#define RDCFG	0x002

#define RDCVA	0x004
#define RDCVB	0x006
#define RDCVC	0x008
#define RDCVD	0x00A
#define RDAUXA	0x00C
#define RDAUXB	0x00E
#define RDSTATA	0x010
#define RDSTATB	0x012

#define CLRCELL 0x711
#define CLRAUX	0x712
#define CLRSTAT 0x713
#define PLADC	0x714
#define DIAGN	0x715
#define WRCOMM	0x721
#define RDCOMM	0x722
#define STCOMM	0x723

#define MD(x)	((x>0 && x<4)?(x<<7):(2<<7))
													/*	MD ADCOPT(CFGR0[0]) = 0 | ADCOPT(CFGR0[0]) = 1
													x=1	01 27kHz Mode (Fast)	|	 14kHz Mode
													x=2	10 7kHz Mode  (Normal)	| 	 3kHz Mode
													x=3	11 26Hz Mode  (Filtered)| 	 2kHz Mode         */
#define DCP(y)	((y<2)?(y<<4):(1<<4))
													/*	DCP
													y=0	0 Discharge Not Permitted
													y=1	1 Discharge Permitted						   */
#define CH(z)	((z<7)?(z):(0))
													/*							Total Conversion Time in the 6 ADC Modes
														CH 							27kHz 14kHz 7kHz  3kHz  2kHz  26Hz
													z=0	000 	All Cells 			1.1ms 1.3ms 2.3ms 3.0ms 4.4ms 201ms
													z=1	001 Cell 1 	and Cell 7					201us
													z=2	010 Cell 2 	and Cell 8					230us
													z=3	011 Cell 3 	and Cell 9					405us
													z=4	100 Cell 4 	and Cell 10					501us
													z=5	101 Cell 5 	and Cell 11					754us
													z=6	110 Cell 6 	and Cell 12					34ms					*/
#define ADCV	(0x260 | MD(2) | DCP(1) | CH(0))	//Control ADC conversion settings

/* =================== COMMAND BITS ================== */

#define ADC_MODE_7KHZ 0x10

#define ADC_CHN_ALL 0x0
#define ADC_CHN_1_7 0x1
#define ADC_CHN_2_8 0x2
#define ADC_CHN_3_9 0x3
#define ADC_CHN_4_10 0x4
#define ADC_CHN_5_11 0x5
#define ADC_CHN_6_12 0x6

#define PUP_DOWN 0x0
#define PUP_UP 0x1

#define ST_ONE 0x1
#define ST_TWO 0x2
#define ST_ONE_27KHZ_RES 0x9565
#define ST_ONE_14KHZ_RES 0x9553
#define ST_ONE_7KHZ_RES 0x9555
#define ST_ONE_2KHZ_RES 0x9555
#define ST_ONE_26HZ_RES 0x9555
#define ST_TWO_27KHZ_RES 0x6A9A
#define ST_TWO_14KHZ_RES 0x6AAC
#define ST_TWO_7KHZ_RES 0x6AAA
#define ST_TWO_2KHZ_RES 0x6AAA
#define ST_TWO_26HZ_RES 0x6AAA

#define NUM_CELL_GROUPS 4

///***************************************
//		Public Types
//****************************************/
//
//typedef enum {
//	LTC6804_ADC_MODE_FAST = 1, LTC6804_ADC_MODE_NORMAL = 2, LTC6804_ADC_MODE_SLOW = 3
//} LTC6804_ADC_MODE_T;
//
//typedef enum {
//	LTC6804_ADC_NONE, LTC6804_ADC_GCV, LTC6804_ADC_OWT, LTC6804_ADC_CVST, LTC6804_ADC_GPIO
//} LTC6804_ADC_STATUS_T;
//
//static const uint8_t LTC6804_ADCV_MODE_WAIT_TIMES[] = {
//	0, 2, 4, 202
//};
//
//static const uint8_t LTC6804_ADAX_MODE_WAIT_TIMES[] = {
//	0, 2, 4, 202
//};
//
//static const uint16_t LTC6804_SELF_TEST_RES[] = {
//	0, 0x9565, 0x9555, 0x9555
//};
//
//typedef struct {
//	LPC_SSP_T *pSSP;
//	uint32_t baud;
//	uint8_t cs_gpio;
//	uint8_t cs_pin;
//
//	uint8_t num_modules;
//	uint8_t *module_cell_count;
//
//	uint32_t min_cell_mV;
//	uint32_t max_cell_mV;
//
//	LTC6804_ADC_MODE_T adc_mode;
//} LTC6804_CONFIG_T;
//
//typedef struct {
//	Chip_SSP_DATA_SETUP_T *xf;
//	uint8_t *tx_buf;
//	uint8_t *rx_buf;
//	uint32_t last_message;
//	uint32_t wake_length;
//	uint8_t *cfg;
//
//	bool waiting;
//	uint32_t wait_time;
//	uint32_t flight_time;
//	uint32_t last_sleep_wake;
//
//	bool balancing;
//	uint16_t *bal_list;
//
//	LTC6804_ADC_STATUS_T adc_status;
//} LTC6804_STATE_T;
//
//typedef enum {
//	LTC6804_WAITING, LTC6804_PASS, LTC6804_FAIL, LTC6804_PEC_ERROR, LTC6804_WAITING_REFUP
//} LTC6804_STATUS_T;
//
//typedef struct {
//	uint32_t *cell_voltages_mV; // array size = #modules * cells/module
//	uint32_t pack_cell_max_mV;
//	uint32_t pack_cell_min_mV;
//} LTC6804_ADC_RES_T;
//
//typedef struct {
//	uint8_t failed_module;
//	uint8_t failed_wire;
//} LTC6804_OWT_RES_T;


//#endif
