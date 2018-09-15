/**
 * LTC6804.c
 * Contains protocols/functions for communication with LTC6804
 * Author : vivekhpandya
 */
#define LTC6804_H
#include "LTC6804.h"
#include "iomap.h"

void measure(void)
{
	// ADC conversion
	//HAL_SPI_Transmit(&hspi1,&D_out,sizeof(D_out),0);
}

/**
 * @Description : Calculate PEC15 Table values and
 * 				  fill array for further calculation
 * @Parameter 1 : void
 * @RetValue    : void
 */
void init_PEC15_Table(void)
{
    uint16_t remainder;			//Check if signed or un-signed
    for (uint16_t i = 0; i < 256; i++)
    {
        remainder = i << 7;
        for (uint16_t bit = 8; bit > 0; --bit)
        {
            if (remainder & 0x4000)
            {
                remainder = ((remainder << 1));
                remainder = (remainder ^ Poly);
            }
            else
            {
                remainder = ((remainder << 1));
            }
        }
        pec15Table[i] = remainder&0xFFFF;
    }
}

/**
 * @Description : Calculate CRC(PEC) for given input
 * @Parameter 1 : Message for which CRC(PEC) is to be calculated
 * @Parameter 2 : Length of message passed (bytes)
 * @RetValue    : 16-bit calculated CRC
 */
uint16_t pec15(uint8_t *data , uint16_t len)
{
	uint16_t remainder,address;
    remainder = PEC_SEED;//PEC seed
    for (uint16_t i = 0; i < len; i++)
    {
        address = ((remainder >> 7) ^ data[i]) & 0xff;//calculate PEC table address
        remainder = (remainder << 8 ) ^ pec15Table[address];
//        printf("%u",remainder);
    }
    return (remainder<<1); //The CRC15 has a 0 in the LSB so the final value must be multiplied by 2 i.e. bit shifting
}

/**
 * @Description : Executing(sending) Broadcast Read Command for reading particular register OR register group
 * @Parameter 1 : 11-bit command for required execution (16-bit data format is used)
 * @Parameter 2 : Data pointer to store data to be received in response of command executed(sent)
 * @Parameter 3 : Length of data to be received in response of command executed(sent) (Excluding PEC)
 * @RetValue    : Err_Code
 */
uint8_t Read_Command(uint16_t Cmd_code, uint8_t *Rec_Data_Ptr, uint8_t Rec_Data_Length)
{
	uint8_t data_string[4];
	uint16_t pec15_generated,pec15_verification;
	uint8_t *pec15_pointer;

	CSB_PORT->ODR &= ~CSB_PIN;

	data_string[0] = (uint8_t)((Cmd_code>>8) & 0x0007);
	data_string[1] = (uint8_t)Cmd_code;
	pec15_generated = pec15(data_string,2);		//Generate PEC for command
	data_string[2] = (uint8_t)(pec15_generated>>8);
	data_string[3] = (uint8_t)(pec15_generated);

	HAL_SPI_Transmit(&hspi1,data_string,4,50);

	HAL_SPI_Receive(&hspi1,Rec_Data_Ptr,Rec_Data_Length+2,50);

	CSB_PORT->ODR |= CSB_PIN;

	pec15_pointer = Rec_Data_Ptr + Rec_Data_Length;
	pec15_verification = ((uint16_t)(*pec15_pointer)<<8);
	pec15_pointer++;
	pec15_verification	|= (uint16_t)*pec15_pointer;
	if(pec15_verification == pec15(Rec_Data_Ptr,Rec_Data_Length))
	{
		return ERR_SUCCESS;
	}
	else
	{
		return ERR_PECMISMATCH1;
	}
}

/**
 * @Description : Executing(sending) Broadcast Read Command for reading particular register OR register group (For Daisy-Chain)
 * @Parameter 1 : 11-bit command for required execution (16-bit data format is used)
 * @Parameter 2 : Data pointer to store data to be received in response of command executed(sent)
 * @Parameter 3 : Length of data to be received in response of command executed(sent) (Excluding PEC)
 * @Parameter 4 : No of LTC ICs in daisy chain to be communicated
 * @RetValue    : Err_Code
 */
uint8_t Read_Command_Chain(uint16_t Cmd_code, uint8_t *Rec_Data_Ptr, uint8_t Rec_Data_Length, uint8_t No_of_ICs)
{
	uint8_t data_string[4],err_code=0,iq;
	uint16_t pec15_generated,pec15_verification;
	uint8_t *pec15_pointer;

	CSB_PORT->ODR &= ~CSB_PIN;

	data_string[0] = (uint8_t)((Cmd_code>>8) & 0x0007);
	data_string[1] = (uint8_t)Cmd_code;
	pec15_generated = pec15(data_string,2);		//Generate PEC for command
	data_string[2] = (uint8_t)(pec15_generated>>8);
	data_string[3] = (uint8_t)(pec15_generated);

	HAL_SPI_Transmit(&hspi1,data_string,4,50);

	HAL_SPI_Receive(&hspi1,Rec_Data_Ptr,(Rec_Data_Length+2)*No_of_ICs,50);

	CSB_PORT->ODR |= CSB_PIN;

	for(iq=0;iq<No_of_ICs;iq++)
	{
		pec15_pointer = Rec_Data_Ptr + (Rec_Data_Length+2)*iq + Rec_Data_Length;
		pec15_verification = ((uint16_t)(*pec15_pointer)<<8);
		pec15_pointer++;
		pec15_verification	|= (uint16_t)*pec15_pointer;
		if(pec15_verification != pec15(Rec_Data_Ptr + (Rec_Data_Length+2)*iq,Rec_Data_Length))
		{
			err_code |= err_pecmismatch[iq];
		}
	}

	return err_code;
}

/**
 * @Description : Executing(sending) Broadcast Write Command for writing particular register OR register group
 * @Parameter 1 : 11-bit command for required execution (16-bit data format is used)
 * @Parameter 2 : Data pointer of an array having new register value/values to be sent with command executed(sent)
 * @Parameter 3 : Length of data to be sent with of command executed(sent) starting from data pointer
 * @RetValue    : void
 */
void Write_Command(uint16_t Cmd_code, uint8_t *Trans_Data_Ptr, uint8_t Trans_Data_Length)
{
	uint8_t data_string[25],ip;
	uint16_t pec15_generated;//pec15_verification;
//	uint8_t *pec15_pointer;

	CSB_PORT->ODR &= ~CSB_PIN;

	data_string[0] = (uint8_t)((Cmd_code>>8) & 0x0007);
	data_string[1] = (uint8_t)Cmd_code;
	pec15_generated = pec15(data_string,2);		//Generate PEC for command
	data_string[2] = (uint8_t)(pec15_generated>>8);
	data_string[3] = (uint8_t)(pec15_generated);

	for(ip=4;ip<Trans_Data_Length+4;ip++)
	{
		data_string[ip] = (uint8_t)*Trans_Data_Ptr;
		Trans_Data_Ptr++;
	}
	pec15_generated = pec15(&data_string[4],(ip-4));		//Generate PEC for data
	data_string[ip++] = (uint8_t)(pec15_generated>>8);
	data_string[ip]   = (uint8_t)(pec15_generated);

	HAL_SPI_Transmit(&hspi1,data_string,ip+1,50);

	CSB_PORT->ODR |= CSB_PIN;
}

/**
 * @Description : Executing(sending) Broadcast Write Command for writing particular register OR register group (For Daisy-Chain)
 * @Parameter 1 : 11-bit command for required execution (16-bit data format is used)
 * @Parameter 2 : Data pointer of an array having new register value/values to be sent with command executed(sent)
 * @Parameter 3 : Length of data to be sent with of command executed(sent) starting from data pointer for any particular IC
 * @Parameter 4 : No of LTC ICs in daisy chain to be communicated
 * @RetValue    : void
 */
void Write_Command_Chain(uint16_t Cmd_code, uint8_t *Trans_Data_Ptr, uint8_t Trans_Data_Length, uint8_t No_of_ICs)
{
	uint8_t data_string[25],ip,iq,next_string_address;
	uint8_t *Actual_Trans_Data_Ptr;
	uint16_t pec15_generated;//pec15_verification;
//	uint8_t *pec15_pointer;

	CSB_PORT->ODR &= ~CSB_PIN;

	data_string[0] = (uint8_t)((Cmd_code>>8) & 0x0007);
	data_string[1] = (uint8_t)Cmd_code;
	pec15_generated = pec15(data_string,2);		//Generate PEC for command
	data_string[2] = (uint8_t)(pec15_generated>>8);
	data_string[3] = (uint8_t)(pec15_generated);
	next_string_address = 4;

	for(iq=0;iq<No_of_ICs;iq++)
	{
		Actual_Trans_Data_Ptr = Trans_Data_Ptr + (Trans_Data_Length*(No_of_ICs-1-iq));				//As actually farthest chip command is at the top of Array
		for(ip=next_string_address;ip<Trans_Data_Length+next_string_address;ip++)
		{
			data_string[ip] = (uint8_t)*Actual_Trans_Data_Ptr;
			Actual_Trans_Data_Ptr++;
		}
		pec15_generated = pec15(&data_string[next_string_address],(ip-next_string_address));		//Generate PEC for data
		data_string[ip++] = (uint8_t)(pec15_generated>>8);
		data_string[ip]   = (uint8_t)(pec15_generated);
		next_string_address = ip+1;
	}

	HAL_SPI_Transmit(&hspi1,data_string,ip+1,50);
//	HAL_SPI_Transmit(&hspi1,&data_string[4],Trans_Data_Length-4,50);

	CSB_PORT->ODR |= CSB_PIN;
}

/**
 * @Description : Executing(sending) Broadcast Command and polling result
 * @Parameter 1 : 11-bit command for required execution (16-bit data format is used)
 * @RetValue    : Err_Code
 */
uint8_t Run_Command(uint16_t Cmd_code)
{
	uint8_t data_string[4];
	uint16_t pec15_generated;

	CSB_PORT->ODR &= ~CSB_PIN;

	data_string[0] = (uint8_t)((Cmd_code>>8) & 0x0007);
	data_string[1] = (uint8_t)Cmd_code;
	pec15_generated = pec15(data_string,2);		//Generate PEC for command
	data_string[2] = (uint8_t)(pec15_generated>>8);
	data_string[3] = (uint8_t)(pec15_generated);

	HAL_SPI_Transmit(&hspi1,data_string,4,50);

	CSB_PORT->ODR |= CSB_PIN;

	return ERR_SUCCESS;
}

/**
 * @Description : Executing(sending) Dummy byte for wake up
 * @Parameter 1 : void
 * @RetValue    : void
 */
void DummyByte(void)
{
//	uint8_t dummy = 0x7A;
	CSB_PORT->ODR &= ~CSB_PIN;

//	HAL_SPI_Transmit(&hspi1,&dummy,1,10);
	delay_100us(100);delay_100us(100);delay_100us(100);

	CSB_PORT->ODR |= CSB_PIN;
}

///**
// * @Description : Calculate CRC(PEC) for given input
// * @Parameter 1 : Message for which CRC(PEC) is to be calculated
// * @Parameter 2 : Length of message passed in
// * @RetValue    : 16-bit calculated CRC
// */
//uint16_t CRC_Calculate(uint16_t DIN, uint8_t DIN_length)
//{
//	uint16_t xPEC,effective_Poly,it1,it2,loop_i;
//	uint16_t bit_selector = 0x0001;
//	xPEC = PEC_SEED;
//	for(loop_i=0;loop_i<DIN_length;loop_i++)
//	{
//		if((DIN & bit_selector) == 0)
//		{
//			if((xPEC & PEC_MSB) == 0)
//			{
//				effective_Poly = 0;
//			}
//			else
//			{
//				effective_Poly = Poly;
//			}
//			effective_Poly &= ~0x0001;
//		}
//		else
//		{
//			if((xPEC & PEC_MSB) == 0)
//			{
//				effective_Poly = Poly;
//			}
//			else
//			{
//				effective_Poly = 0;
//			}
//			effective_Poly |= 0x0001;
//		}
//
//		it1 = (xPEC<<1) & nPoly;
//		it2 = (((xPEC<<1) ^ effective_Poly) & Poly);
//		xPEC = it1 | it2;
//
//		bit_selector <<= 1;
//	}
//	return (xPEC<<1);
//}

/**
 * @Description : Generate 1 micr0-second delay for 72Mhz clock in STM32F103
 * @Parameter 1 : void
 * @RetValue    : void
 */
void delay_1us(void)
{
	uint8_t i;
	for(i=0;i<=2;i++);
}

void delay_5us(void)
{
	delay_1us();delay_1us();delay_1us();delay_1us();delay_1us();
}

void delay_10us(uint8_t repeat)
{
	uint8_t i;
	for(i=0;i<repeat;i++)
	{
		delay_5us();delay_5us();
	}
}

void delay_100us(uint8_t repeat)
{
	uint8_t i;
	for(i=0;i<repeat;i++)
	{
		delay_10us(10);
	}
}
