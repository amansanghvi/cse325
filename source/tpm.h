#ifndef TPM_H
#define TPM_H

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL46Z4.h"
#include "fsl_debug_console.h"

typedef struct _Tpm {
	TPM_Type *tpm;
	unsigned char channel;
	int mod;
} Tpm_ch;

void tpm_init(Tpm_ch *tpm_ch, unsigned char tpm, unsigned char channel, int mod,  unsigned char prescaler, int high_len);
void tpm_enable(Tpm_ch tpm_ch);
void tpm_update_val(Tpm_ch tpm_ch, int high_len);
void tpm_disable(Tpm_ch tpm_ch);

#endif
