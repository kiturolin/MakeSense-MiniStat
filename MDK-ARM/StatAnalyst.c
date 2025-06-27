#include "MiniStatAnalyst.h"

// num1 and num2			two points
// dt					difference in time between the two points of interest
// float					intentionally using float since it uses less space than double
//
// Calculates the derviate between two points
float
calcDerivative (int16_t num1, int16_t num2, int16_t dt)
{
  return (num2 - num1) / (float)dt;
}

void
checkVBounds (int16_t *v1, int16_t *v2, const int8_t redox)
{
  // for the reduction scan, v1 must be larger than v2 since reduction goes
  // from more positive number to more negative number
  if (redox == REDUCTION) {
    if (*v2 > *v1) {
      int16_t temp = *v1; // create temp variable to facilitate swapping
      *v1 = *v2;
      *v2 = temp;
    }
  }
  // for the oxidation scan, v1 must be smaller than v2 since oxiation goes
  // from more negative number to more positive number
  else if (redox == OXIDATION) {
    if (*v1 > *v2) {
      int16_t temp = *v1; // create temp variable to facilitate swapping
      *v1 = *v2;
      *v2 = temp;
    }
  }
}

uint16_t
calcBaseline_v1 (int16_t v1,
                 int16_t v2,
                 int8_t redox,
                 const float current[],
                 const int16_t voltage[],
                 float *slope,
                 float *intercept,
                 uint16_t samples)
{
	// 手动修复一个bug: 将 || 替换为 &&, 若为 || 则if表达式永远为真
  if (redox != REDUCTION && redox != OXIDATION) return NO_REDOX_TYPE;

  if (v2 == v1) return NO_BASELINE_SELECTED;
  else checkVBounds (&v1, &v2, redox);

  int16_t v_incr = fabs (voltage[0] - voltage[1]);

  // initializes working variables
  uint16_t i = 0;
  float xy_sum = 0;
  float x_sum = 0;
  float y_sum = 0;
  float x_squared_sum = 0;

  while (!(v1 >= voltage[i] - v_incr && v1 <= voltage[i] + v_incr &&
           (voltage[i + 1] - voltage[i]) / fabs (voltage[i + 1] - voltage[i]) == redox)) {
    i++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  uint16_t count = 0;
  while (!(v2 >= voltage[i] - v_incr && v2 <= voltage[i] + v_incr) && (i + 1) < samples) {
    xy_sum += voltage[i] * current[i];
    x_sum += voltage[i];
    y_sum += current[i];
    x_squared_sum += voltage[i] * voltage[i];

    i++;
    count++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  *slope = (xy_sum - (x_sum * y_sum / (float)count)) / (x_squared_sum - (x_sum * x_sum / (float)count));
  *intercept = y_sum / (float)count - (*slope) * x_sum / (float)count;

  return NO_ERROR;
}

uint16_t
calcBaseline_v2 (
    uint8_t exp, const float current[], const int16_t voltage[], float *slope, float *intercept, uint16_t samples)
{
  // Initializes working variables
  int8_t redox = 0;
  int16_t v1 = 0;
  int16_t v2 = 0;
  int16_t v_incr = fabs (voltage[0] - voltage[1]);

  if (exp == MB_APTAMER_BASE) {
    redox = REDUCTION;
    v1 = -50;
    v2 = -200;
  } else if (exp == FeCN_OX_BASE) {
    redox = OXIDATION;
    v1 = -185;
    v2 = -30;
  } else if (exp == FeCN_RED_BASE) {
    redox = REDUCTION;
    v1 = 400;
    v2 = 300;
  } else return NO_BASELINE_SELECTED;

  // initializes working variables
  uint16_t i = 0;
  float xy_sum = 0;
  float x_sum = 0;
  float y_sum = 0;
  float x_squared_sum = 0;

  while (!(v1 >= voltage[i] - v_incr && v1 <= voltage[i] + v_incr &&
           (voltage[i + 1] - voltage[i]) / fabs (voltage[i + 1] - voltage[i]) == redox)) {
    i++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  uint16_t count = 0;
  while (!(v2 >= voltage[i] - v_incr && v2 <= voltage[i] + v_incr) && (i + 1) < samples) {
    xy_sum += voltage[i] * current[i];
    x_sum += voltage[i];
    y_sum += current[i];
    x_squared_sum += voltage[i] * voltage[i];

    i++;
    count++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  *slope = (xy_sum - (x_sum * y_sum / (float)count)) / (x_squared_sum - (x_sum * x_sum / (float)count));
  *intercept = y_sum / (float)count - (*slope) * x_sum / (float)count;

  // if you get this far, then everything worked correctly
  return NO_ERROR;
}

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
                   uint16_t samples)
{
  if (redox != REDUCTION || redox != OXIDATION) return NO_REDOX_TYPE;

  if (v2 == v1) return NO_PEAK_REGION_SELECTED;
  checkVBounds (&v1, &v2, redox);

  int16_t v_incr = fabs (voltage[0] - voltage[1]);

  uint16_t i = 0;
  while (!(v1 >= voltage[i] - v_incr && v1 <= voltage[i] + v_incr &&
           (voltage[i + 1] - voltage[i]) / fabs (voltage[i + 1] - voltage[i]) == redox)) {
    i++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  // Set the current index as the peak, then increment the index.
  *peak = current[i] - (slope * voltage[i] + intercept);
  *v_peak = voltage[i];
  i++;

  while (!(v2 >= voltage[i] - v_incr && v2 <= voltage[i] + v_incr) && (i + 1) < samples) {
    // If the current index is contains a value greater than the absolute
    // value of the peak, then update the value of the peak.
    if (fabs (*peak) < fabs (current[i] - (slope * voltage[i] + intercept))) {
      *peak = current[i] - (slope * voltage[i] + intercept);
      *v_peak = voltage[i];
    }

    i++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  return NO_ERROR;
}

//
uint16_t
getPeakCurrent_v2 (uint8_t exp,
                   const float current[],
                   const int16_t voltage[],
                   const float slope,
                   const float intercept,
                   float *peak,
                   int16_t *v_peak,
                   uint16_t samples)
{
  // initializes working variables
  int8_t redox = 0;
  int16_t v1 = 0;
  int16_t v2 = 0;
  int16_t v_incr = fabs (voltage[0] - voltage[1]);

  if (exp == MB_APTAMER_PEAK) {
    redox = REDUCTION;
    v1 = -315;
    v2 = -375;
  } else if (exp == FeCN_OX_PEAK) {
    redox = OXIDATION;
    v1 = 140;
    v2 = 300;
  } else if (exp == FeCN_RED_PEAK) {
    redox = REDUCTION;
    v1 = 170;
    v2 = 30;
  } else return NO_BASELINE_SELECTED;

  uint16_t i = 0;
  while (!(v1 >= voltage[i] - v_incr && v1 <= voltage[i] + v_incr &&
           (voltage[i + 1] - voltage[i]) / fabs (voltage[i + 1] - voltage[i]) == redox)) {
    i++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  // Set the current index as the peak, then increment the index.
  *peak = current[i] - (slope * voltage[i] + intercept);
  *v_peak = voltage[i];
  i++;

  while (!(v2 >= voltage[i] - v_incr && v2 <= voltage[i] + v_incr) && (i + 1) < samples) {
    if (fabs (*peak) < fabs (current[i] - (slope * voltage[i] + intercept))) {
      *peak = current[i] - (slope * voltage[i] + intercept);
      *v_peak = voltage[i];
    }

    i++;
    if (i > samples) return NO_BASELINE_FOUND;
  }

  return NO_ERROR;
}

// Γ=(N_A Q)/nFA
float
calcProbeDensity (float charge, uint8_t num_electrons, float area)
{
  return (AVOGADRO * charge) / (num_electrons * FARADAY * area);
}

// Q = i*V/v
float
calcCharge (float current, int16_t voltage, uint16_t scan_rate)
{
  return current * voltage / scan_rate;
}

float
calcArea (float charge)
{
  return charge / CHARGE_TRANSFER_CONSTANT;
}

float
calcPhase (float freq, float dt, uint8_t scale)
{
  return 360 * freq * dt / pow (10, scale);
}

float
calcImg (float z, float theta)
{
  return z * sin (theta * PI / 180);
}

float
calcReal (float z, float theta)
{
  return z * cos (theta * PI / 180);
}

float
getMax (const float data[], uint16_t samples)
{
  // assumes first value is the max, then checks if any
  // subsequent value in the array is bigger than the first
  float max = data[0];

  for (uint16_t i = 1; i < samples; i++) {
    if (data[i] > max) max = data[i];
  }

  return max;
}

float
getMin (const float data[], uint16_t samples)
{
  // assumes first value is the minimum, then checks if any subsequent value
  // in the array is smaller than the first
  float min = data[0];

  for (uint16_t i = 1; i < samples; i++) {
    if (data[i] < min) min = data[i];
  }

  return min;
}

float
getPeaktoPeak_v1 (const float data[], uint16_t samples)
{
  return getMax (data, samples) - getMin (data, samples);
}

float
getPeaktoPeak_v2 (const float data[], uint16_t samples, float *max, float *min)
{
  // assumes first value is the min/max, then checks if any subsequent value
  // in the array is smaller/bigger than the first
  *max = data[0];
  *min = data[0];

  for (uint16_t i = 1; i < samples; i++) {
    if (data[i] > *max) *max = data[i];
    if (data[i] < *min) *min = data[i];
  }

  return *max - *min;
}

float
getAverage (const float data[], uint16_t samples)
{
  float sum = 0;

  for (uint16_t i = 0; i < samples; i++) { sum += data[i]; }

  return sum / samples;
}

double
getZeroCrossing (const float data[], const unsigned long time[], const float avg, const uint16_t samples)
{
  for (uint16_t i = 0; i < samples - 1; i++) {
    if (data[i + 1] - avg == 0) return time[i + 1];
    else if (data[i] - avg == 0) return time[i];
    else if ((data[i + 1] - avg) / (data[i] - avg) < 0) {

      //			SerialUSB.println();
      //			SerialUSB.println();
      //			SerialUSB.print(i);
      //			SerialUSB.println();
      //			SerialUSB.println();

      double slope = (data[i + 1] - data[i]) / (time[i + 1] - time[i]);
      double intercept = (data[i] - avg) - slope * time[i];
      return (0 - intercept) / slope;
    }
  }

  return 0;
}
