#ifndef MiniStatAnalyst_h
#define MiniStatAnalyst_h

#include "main.h"
#include "math.h"

const float AVOGADRO = 6.02214076e23;
const float FARADAY = 96485.33289;
const float PI = 3.14159265359;
const float CHARGE_TRANSFER_CONSTANT = 400; // 400uC/cm2

const int8_t REDUCTION = -1;
const int8_t OXIDATION = 1;

// ERROR CODES
const uint16_t NO_ERROR = 0x00;

const uint16_t NO_REDOX_TYPE = 0x01;
const uint16_t NO_BASELINE_SELECTED = 0x02;
const uint16_t NO_BASELINE_FOUND = 0x03;
const uint16_t NO_PEAK_REGION_SELECTED = 0x04;

// Experiment Codes
const uint8_t MB_APTAMER_BASE = 0;
const uint8_t FeCN_OX_BASE = 1;
const uint8_t FeCN_RED_BASE = 2;

const uint8_t MB_APTAMER_PEAK = 3;
const uint8_t FeCN_OX_PEAK = 4;
const uint8_t FeCN_RED_PEAK = 5;

void
checkVBounds (int16_t *v1, int16_t *v2, const int8_t redox);

float
calcDerivative (int16_t num1, int16_t num2, int16_t dt);

uint16_t
calcBaseline_v1 (int16_t v1,
                 int16_t v2,
                 int8_t redox,
                 const float current[],
                 const int16_t voltage[],
                 float *slope,
                 float *intercept,
                 uint16_t samples);

uint16_t
calcBaseline_v2 (
    uint8_t exp, const float current[], const int16_t voltage[], float *slope, float *intercept, uint16_t samples);

uint16_t
getPeakCurrent_v1 (int16_t v1,
                   int16_t v2,
                   int8_t redox,
                   const float current[],
                   const int16_t voltage[],
                   const float slope,
                   const float intercept,
                   float *peak,
                   int16_t *v_peak,
                   uint16_t samples);

uint16_t
getPeakCurrent_v2 (uint8_t exp,
                   const float current[],
                   const int16_t voltage[],
                   const float slope,
                   const float intercept,
                   float *peak,
                   int16_t *v_peak,
                   uint16_t samples);

float
calcProbeDensity (float charge, uint8_t num_electrons, float area);
float
calcCharge (float current, int16_t voltage, uint16_t scan_rate);
float
calcArea (float charge);

float
calcPhase (float freq, float dt, uint8_t scale);
float
calcImg (float z, float theta);
float
calcReal (float z, float theta);

float
getMax (const float data[], uint16_t samples);
float
getMin (const float data[], uint16_t samples);
float
getPeaktoPeak_v1 (const float data[], uint16_t samples);
float
getPeaktoPeak_v2 (const float data[], uint16_t samples, float *max, float *min);

float
getAverage (const float data[], uint16_t samples);
double
getZeroCrossing (const float data[], const unsigned long time[], const float avg, const uint16_t samples);

#endif
