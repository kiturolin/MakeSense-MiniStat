#ifndef _LMP91000_H_
#define _LMP91000_H_

#include "i2c.h"
#include "adc.h"

#define TEMP_OFFSET	1562.2
#define TEMP_SLOPE		(-8.16)

#define ADC_REF 3300

#define LMP91000_BUS_ADDR 0x90 // 0b1001 0000 (shited left)

#define LOCKCFG 1
#define UNLOCKCFG 0

#define REGADDR_STATUS  0x00
#define REGADDR_LOCK    0x01
#define REGADDR_TIACN   0x10
#define REGADDR_REFCN   0x11
#define REGADDR_MODECN 0x12

#define TIAGAIN_EXTR 0x00
#define TIAGAIN_2K75 0x01
#define TIAGAIN_3K50 0x02
#define TIAGAIN_007K 0x03
#define TIAGAIN_014K 0x04
#define TIAGAIN_035K 0x05
#define TIAGAIN_120K 0x06
#define TIAGAIN_350K 0x07

#define RLOAD_010 0x00
#define RLOAD_033 0x01
#define RLOAD_050 0x02
#define RLOAD_100 0x03

#define REFSRC_INTR 0x00
#define REFSRC_EXTR 0x01

#define INTZ_20PCT  0x00
#define INTZ_50PCT  0x01
#define INTZ_67PCT  0x02
#define INTZ_BYPASS 0x03

#define BIASSIGN_NEG 0x00
#define BIASSIGN_POS 0x01

#define BIAS_00PCT 0x00 // default
#define BIAS_01PCT 0x01
#define BIAS_02PCT 0x02
#define BIAS_04PCT 0x03
#define BIAS_06PCT 0x04
#define BIAS_08PCT 0x05
#define BIAS_10PCT 0x06
#define BIAS_12PCT 0x07
#define BIAS_14PCT 0x08
#define BIAS_16PCT 0x09
#define BIAS_18PCT 0x0A
#define BIAS_20PCT 0x0B
#define BIAS_22PCT 0x0C
#define BIAS_24PCT 0x0D

#define OPMODE_DEEP_SLEEP   0x00 // default
#define OPMODE_GALVANIC     0x01
#define OPMODE_STANDBY      0x02
#define OPMODE_AMPEROMETRIC 0x03
#define OPMODE_TIA_OFF      0x06
#define OPMODE_TIA_ON       0x07


typedef struct {
	uint8_t mode;
	uint8_t lock;
	uint8_t gain_level;
	uint8_t rload;
	uint8_t intz;
	uint8_t bias;
	float temp;
	float voltage;
	float current;
} LMP91000DeviceType;

#endif
