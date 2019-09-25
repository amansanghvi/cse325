#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL46Z4.h"
#include "fsl_debug_console.h"
#include "tpm.h"

#define SAFETY_FACTOR 7 // 1/7 of the maximum speed for the motor

TPM_Type *get_tpm(unsigned char tpm);

void tpm_init(Tpm_ch *tpm_ch, unsigned char tpm, unsigned char channel, int mod,  unsigned char prescaler, int high_len) {
	tpm_ch->tpm = get_tpm(tpm);
	tpm_ch->channel = channel;
	tpm_ch->mod = mod;

	tpm_ch->tpm->CONTROLS[tpm_ch->channel].CnSC |= (0x2 << 2) | (0x2 << 4);  // Edge-aligned PWM, High-true pulses
	tpm_ch->tpm->MOD = tpm_ch->mod;
	tpm_ch->tpm->CONTROLS[tpm_ch->channel].CnV = high_len;
	tpm_ch->tpm->SC |= prescaler;
}

void tpm_enable(Tpm_ch tpm_ch) {
	tpm_ch.tpm->SC |= 0x01 << 3; // START
}

void tpm_disable(Tpm_ch tpm_ch) {
	tpm_ch.tpm->SC &= ~(0x01 << 3); // STOP
}

void tpm_update_val(Tpm_ch tpm_ch, int high_len) {
	// If it is the motor
	if (tpm_ch.tpm == TPM2 && tpm_ch.channel == 1) {
		// never go higher than 15% power.
		high_len = (high_len > tpm_ch.mod/SAFETY_FACTOR) ? tpm_ch.mod/SAFETY_FACTOR : high_len;
		high_len = (high_len < 0) ? 0 : high_len;
	}
	tpm_ch.tpm->CONTROLS[tpm_ch.channel].CnV = high_len;
}

TPM_Type *get_tpm(unsigned char tpm) {
	switch(tpm) {
	case 0:
		return TPM0;
	case 1:
		return TPM1;
	case 2:
		return TPM2;
	default:
		return NULL;
	}
}
