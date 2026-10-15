/*
 * uart.h
 *
 *  Created on: Oct 12, 2026
 *      Author: joseluis
 */

#ifndef UART_H_
#define UART_H_


void uart_init(void);

void uart_send_char(char c);

void uart_send_string(const char *str);

#endif /* UART_H_ */
