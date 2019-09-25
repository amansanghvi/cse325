/*
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    Assignment_2.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL46Z4.h"
#include "fsl_debug_console.h"
#include "tpm.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
#define TPM2_MOD 10000-1 // 10ms mod (100Hz).
#define TPM2_PRESCALE 0x3 // prescaler of 8 (Makes 1 tick 1 microsecond).
#define CYCLES_PER_MILLIS 0x5DC0

void init_clocks();
void init_ports();
void wait(unsigned int millis);
void circle(Tpm_ch servo, Tpm_ch motor);
void square(Tpm_ch servo, Tpm_ch motor);
/*
 * @brief   Application entry point.
 */
int main(void) {

  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
  	/* Init FSL debug console. */
    BOARD_InitDebugConsole();

    init_clocks();
    init_ports();

    GPIOE->PDDR |= (1 << 29); // Configure 'pin' for GP-Output
    GPIOE->PCOR |= (1 << 29);

	Tpm_ch servo_tpm;
	tpm_init(&servo_tpm, 2, 0, TPM2_MOD, TPM2_PRESCALE, 1000); // TPM2_CH0

	Tpm_ch motor_tpm;
	tpm_init(&motor_tpm, 2, 1, TPM2_MOD, TPM2_PRESCALE, 600); // TPM2_CH1

//	circle(servo_tpm, motor_tpm);
	square(servo_tpm, motor_tpm);

    return 0 ;
}

void square(Tpm_ch servo, Tpm_ch motor) {
	int corner = 8500/4;
	int straight = 2500;

	int serv_val = 1470;

	tpm_update_val(servo, serv_val);
	tpm_update_val(motor, 600);
	tpm_enable(servo);
	tpm_enable(motor);

	wait(straight); // Straight
	tpm_update_val(servo, 1000);
	wait(corner); // Left

	tpm_update_val(servo, serv_val); // Straight
	wait(straight);
	tpm_update_val(servo, 1000); // Left
	wait(corner);

	tpm_update_val(servo, serv_val); // Straight
	wait(straight);
	tpm_update_val(servo, 1000); // Left
	wait(corner);

	tpm_update_val(servo, serv_val); // Straight
	wait(straight);
	tpm_update_val(servo, 1000); // Left
	wait(corner);

	tpm_update_val(servo, serv_val); // Straight

	tpm_disable(motor);
	tpm_disable(servo);
}

void circle(Tpm_ch servo, Tpm_ch motor) {
	tpm_update_val(servo, 1000);
	tpm_update_val(motor, 600);
	tpm_enable(servo);
	tpm_enable(motor);

	wait(8500);

	tpm_disable(motor);
	tpm_disable(servo);
}

void wait(unsigned int millis) {
	PIT->MCR = 0x0;
	PIT->CHANNEL[0].LDVAL = millis*CYCLES_PER_MILLIS - 1;
	PIT->CHANNEL[0].TCTRL = (0x1 << 1); // enable Timer 1 interrupts
	PIT->CHANNEL[0].TCTRL |= (0x1 << 0); // start Timer 1
	while (PIT->CHANNEL[0].CVAL > 10) {};
	PIT->MCR = 0x1;
	PIT->CHANNEL[0].TCTRL = 0; // disable Timer 1.
}

void init_clocks() {
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	SIM->SCGC6 |= (0x1 << 24); // Enable TPM0
	SIM->SCGC6 |= (0x1 << 26); // Enable TPM2
	SIM->SOPT2 |= (0x2 << 24); // Set TPM0SRC to OSCERCLK (8MHz).
	SIM->SCGC5 |= (0x1 << 10); // Enable PORTB
//	SIM->SCGC5 |= (0x1 << 12); // Enable PORTD
	SIM->SCGC5 |= (0x1 << 13); // Enable PORTE
}

void init_ports() {
	PORTB->PCR[2] |= (0x3 << 8); // MUX for alt 3 (TPM2_CH0).
	PORTB->PCR[3] |= (0x3 << 8); // MUX for alt 3 (TPM2_CH1).
	PORTE->PCR[29] |= (1 << 8); // Sets MUX bits to set PORTD pin 29 as GPIO
}
