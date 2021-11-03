/**
 *  @file dht11.h
 *	@brief DHT11 Functions
 *  @date Nov 2, 2021
 *  @author Bruce Liao
 *	@version 1.0
 */


#ifndef _DHT11_H_
#define _DHT11_H_

#include "stm32l0xx.h"
#include "main.h"


struct dht11{
	TIM_HandleTypeDef *htim; 
	GPIO_TypeDef* port;
	uint16_t pin; 
	float temperature;
	uint8_t humidty; 
};
typedef struct dht11 dht11_device;


void dht11_init(dht11_device *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef* port, uint16_t pin);
void set_pin_mode(dht11_device *dht, uint8_t Mode);
uint8_t dht11_timeout(dht11_device *dht);
uint8_t get_dht11_data(dht11_device *dht);

#endif
