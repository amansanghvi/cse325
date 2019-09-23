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
#define TPM2_MOD 10000 // 10ms mod (100Hz).
#define TPM2_PRESCALE 0x3 // prescaler of 8 (Makes 1 tick 1 microsecond).

void init_clocks();
void init_ports();
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

    PRINTF("Hello World\n");

    init_clocks();
    init_ports();

	Tpm_ch tpm0;
	tpm_init(&tpm0, 0, 5, 10000, 0x3, 1200); // TPM0_CH5, 10ms mod (100Hz), 2ms on, prescaler of 8 (Makes 1 tick 1 microsecond).
	tpm_enable(tpm0);

	Tpm_ch pwm_tpm;
//	tpm_init(&pwm_tpm, 2, 0, TPM2_MOD, TPM2_PRESCALE, 1500); // TPM2_CH0
//	tpm_enable(pwm_tpm);
	TPM2->CONTROLS[0].CnSC |= (0x1 << 2) | (0x2 << 4);  // Edge-aligned PWM, Low-true pulses
	TPM2->MOD = 20000;
	TPM2->CONTROLS[0].CnV = 2000;
	TPM2->SC |= TPM2_PRESCALE | 0x01 << 3; // prescale /16

	Tpm_ch motor_tpm;
	tpm_init(&motor_tpm, 2, 1, TPM2_MOD, TPM2_PRESCALE, 1500); // TPM2_CH1
	tpm_enable(motor_tpm);


	volatile int a = 0;
	volatile unsigned long long i = 1;

	while(1) {
		if (i%1000000 == 0) {
			if (a) { // LED is active high, so percent is inverted.
				tpm_update_val(tpm0, 9000); // Lower it.
//				tpm_update_val(pwm_tpm, 1000); // Lower it.
				TPM2->CONTROLS[0].CnV = 1100;
				tpm_update_val(motor_tpm, 1000); // Lower it.
			} else {
				tpm_update_val(tpm0, 2000); // Increase it.
				TPM2->CONTROLS[0].CnV = 1900;
//				tpm_update_val(pwm_tpm, 0); // Increase it.
				tpm_update_val(motor_tpm, 2000); // Increase it.
			}
			a = a ? 0 : 1;
			i=1;
		}
		i++;
		/* 'Dummy' NOP to allow source level single stepping of
			tight while() loop */
		__asm volatile ("nop");
	}

    return 0 ;
}

void init_clocks() {
	SIM->SCGC6 |= 1 << 24; // Enable TPM0
	SIM->SCGC6 |= 1 << 26; // Enable TPM2
	SIM->SOPT2 |= (0x2 << 24); // Set TPM0SRC to OSCERCLK (8MHz).
	SIM->SCGC5 |= (0x1 << 10); // Enable PORTB
	SIM->SCGC5 |= (0x1 << 12); // Enable PORTD
	SIM->SCGC5 |= (0x1 << 13); // Enable PORTE
}

void init_ports() {
	PORTB->PCR[2] |= (0x3 << 8); // MUX for alt 3 (TPM2_CH0).
	PORTB->PCR[3] |= (0x3 << 8); // MUX for alt 3 (TPM2_CH1).
	PORTD->PCR[5] |= (0x4 << 8); // MUX for alt 4 (TPM0_CH5).
	PORTE->PCR[29] |= (0x3 << 8);// MUX to TPM0_CH2
}
