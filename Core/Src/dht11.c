/**
 *  @file dht11.c
 *	@brief DHT11 Functions
 *  @date Nov 2, 2021
 *  @author Bruce Liao
 *	@version 1.0
 */


#include "dht11.h"
#include "math.h"
/**
 * @brief configure dht11 struct
 * @param dht struct to configure
 * @param htim TIMER for calculate delays
 * @param port the data port(like GPIOA)
 * @param pin the data pin(like GPIO_PIN_0)
 */
void dht11_init(dht11_device *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef* port, uint16_t pin){
	dht->htim = htim;
	dht->port = port;
	dht->pin = pin;
}

/**
 * @brief set dht11 data pin mode
 * @param dht struct
 * @param Mode GPIO Mode 0 is INPUT,1 is OUTPUT
 */
void set_pin_mode(dht11_device *dht, uint8_t Mode)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(Mode)		//OUTPUT
	{
	  GPIO_InitStruct.Pin = dht->pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  HAL_GPIO_Init(dht->port, &GPIO_InitStruct);
	}else				//INPUT
	{
	  GPIO_InitStruct.Pin = dht->pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  HAL_GPIO_Init(dht->port, &GPIO_InitStruct);
	}
}

uint8_t dht11_timeout(dht11_device *dht)
{
	if((uint16_t)dht->htim->Instance->CNT > 200){
		__enable_irq();
		return 0;
	}
	return 1;
}

/**
 * @brief get data from DHT11
 * @param dht struct
 * @return if something wrong in read will return 0
 */
uint8_t get_dht11_data(dht11_device *dht)
{
	uint16_t t1 = 0, t2 = 0, Bit = 0;
	uint8_t humVal = 0, tempVal = 0;
	float temp_float = 0;
	uint8_t buffer[40];

	//Start Signal
	set_pin_mode(dht, 1);	
	dht->port->ODR&=~(1<<(dht->pin));
	HAL_Delay(18);
	dht->port->ODR|=(1<<(dht->pin));
	
	__disable_irq();	//disable all interupts to do only read dht otherwise miss timer
	HAL_TIM_Base_Start(dht->htim); //start timer
	
	//set pin mode to input & check the DATA pin is high
	set_pin_mode(dht, 0);
	dht->htim->Instance->CNT=0;	
	while(HAL_GPIO_ReadPin(dht->port, dht->pin) == 1){
		if(!dht11_timeout(dht))return 0;
	}
	
	//80us of low
	dht->htim->Instance->CNT=0;	
	while(HAL_GPIO_ReadPin(dht->port, dht->pin) == 0){
		if(!dht11_timeout(dht))return 0;
	}
	t1 = (uint16_t)dht->htim->Instance->CNT;
	
	//80us of high
	dht->htim->Instance->CNT=0;	
	while(HAL_GPIO_ReadPin(dht->port, dht->pin) == 1){
		if(!dht11_timeout(dht))return 0;
	}
	t2 = dht->htim->Instance->CNT;

	//if something wrong,the answer is not 80us.
	if(t1 < 75 && t1 > 85 && t2 < 75 && t2 > 85)
	{
		__enable_irq();
		return 0;
	}

	//Get Data
	for(int j = 0; j < 40; j++)
	{
		dht->htim->Instance->CNT=0;	
		while(HAL_GPIO_ReadPin(dht->port, dht->pin) == 0){
			if(!dht11_timeout(dht))return 0;
		}
		
		dht->htim->Instance->CNT=0;	
		while(HAL_GPIO_ReadPin(dht->port, dht->pin) == 1){
			if(!dht11_timeout(dht))return 0;
		}
		t1 = dht->htim->Instance->CNT;

		//if pass time 25us set as LOW
		if(t1 > 20 && t1 < 30)
		{
			Bit = 0;
		}
		//if pass time 70us set as HIGH
		else if(t1 > 60 && t1 < 80) 
		{
			 Bit = 1;
		}

		buffer[j] = Bit;
	}

	HAL_TIM_Base_Stop(dht->htim); 	//stop timer
	__enable_irq(); 								//enable all interrupts
	
	//get hum value from data buffer
	for(int i = 0; i < 8; i++)
	{
		humVal += buffer[i];
		humVal = humVal << 1;
	}

	//get temp value from data buffer
	for(int i = 16; i < 24; i++)
	{
		tempVal += buffer[i];
		tempVal = tempVal << 1;
	}
	for(int i = 24; i < 30; i++)
	{
		temp_float +=(float)(buffer[i]*pow(2,-8+(i-24)));
	}

	humVal = humVal >> 1;
	tempVal = tempVal >> 1;

	dht->temperature =(float)(temp_float);
	dht->temperature+=tempVal;
	dht->humidty=humVal;
	
	return 1;
}
