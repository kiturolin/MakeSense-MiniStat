#include "LMP91000.h"

typedef enum { NOT_READY = 0, READY } LMP91000_Stauts;

float TIA_GAIN[] = {2750, 3500, 7000, 14000, 35000, 120000, 350000};
float TIA_BIAS[] = {0, 0.01, 0.02, 0.04, 0.06, 0.08, 0.1, 0.12, 0.14, 0.16, 0.18, 0.2, 0.22, 0.24};
float TIA_ZERO[] = {0.2, 0.5, 0.67};

LMP91000DeviceType lmp91000;

void
LMP91000_Enable ()
{
  HAL_GPIO_WritePin (LMP91000_EN_GPIO_Port, LMP91000_EN_Pin, GPIO_PIN_RESET);
}

void
LMP91000_Disable ()
{
  HAL_GPIO_WritePin (LMP91000_EN_GPIO_Port, LMP91000_EN_Pin, GPIO_PIN_SET);
}

void
LMP91000_WriteData (uint8_t addr, uint8_t data)
{
  LMP91000_Enable ();

  if (HAL_I2C_Mem_Write (&hi2c1, LMP91000_BUS_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &data, I2C_MEMADD_SIZE_8BIT,
                         HAL_MAX_DELAY) != HAL_OK) {
    uint32_t ErrCode = HAL_I2C_GetError (&hi2c1);
  }

  LMP91000_Disable ();
}

uint8_t
LMP91000_ReadData (uint8_t addr)
{
  LMP91000_Enable ();

  uint8_t result;
  if (HAL_I2C_Mem_Read (&hi2c1, LMP91000_BUS_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &result, I2C_MEMADD_SIZE_8BIT,
                        HAL_MAX_DELAY) != HAL_OK) {
    uint32_t ErrCode = HAL_I2C_GetError (&hi2c1);
  }

  LMP91000_Disable ();
  return result;
}

void
LMP91000_ToggleLock (uint8_t lockable)
{
  LMP91000_WriteData (REGADDR_LOCK, lockable);
}

void
LMP91000_SetTIAGain (uint8_t gain_level)
{
  // TIACN寄存器中上一次的值
  uint8_t last_content = LMP91000_ReadData (REGADDR_TIACN);
  uint8_t rload = last_content & 0x03; // 只保留最后两位
  uint8_t data = (gain_level << 2) + rload;
  LMP91000_WriteData (REGADDR_TIACN, data);
}

void
LMP91000_SetRLoad (uint8_t rload)
{
  // TIACN寄存器中上一次的值
  uint8_t last_content = LMP91000_ReadData (REGADDR_TIACN);
  uint8_t gain = last_content & 0xfc; // 只保留最高六位
  uint8_t data = gain + rload;
  LMP91000_WriteData (REGADDR_TIACN, data);
}

void
LMP91000_SetRefSource (uint8_t source)
{
  // REFCN寄存器中上一次的值
  uint8_t last_content = LMP91000_ReadData (REGADDR_REFCN);
  uint8_t last = last_content & 0x7f; // 只保留最低六位
  uint8_t data = (source << 7) + last;
  LMP91000_WriteData (REGADDR_REFCN, data);
}

void
LMP91000_SetIntZero (uint8_t zero_level)
{
  // REFCN寄存器中上一次的值
  uint8_t last_content = LMP91000_ReadData (REGADDR_REFCN);
  uint8_t last = last_content & 0x9f; // 只保留0~4, 7位
  uint8_t data = (zero_level << 5) + last;
  LMP91000_WriteData (REGADDR_REFCN, data);
}

void
LMP91000_SetBiasSign (uint8_t sign)
{
  // REFCN寄存器中上一次的值
  uint8_t last_content = LMP91000_ReadData (REGADDR_REFCN);
  uint8_t last = last_content & 0xef;
  uint8_t data = (sign << 4) + last;
  LMP91000_WriteData (REGADDR_REFCN, data);
}

void
LMP91000_SetBias (uint8_t bias)
{ // REFCN寄存器中上一次的值
  uint8_t last_content = LMP91000_ReadData (REGADDR_REFCN);
  uint8_t last = last_content & 0xf0;
  uint8_t data = bias + last;
  LMP91000_WriteData (REGADDR_REFCN, data);
}

void
LMP91000_ToggleShortFET (uint8_t if_short)
{
  uint8_t last_content = LMP91000_ReadData (REGADDR_MODECN);
  uint8_t last = last_content & 0x7f;
  uint8_t data = (if_short << 7) + last;
  LMP91000_WriteData (REGADDR_MODECN, data);
}

void
LMP91000_ChangeMode (uint8_t mode)
{
  uint8_t last_content = LMP91000_ReadData (REGADDR_MODECN);
  uint8_t last = last_content & 0xf8;
  uint8_t data = mode + last;
  LMP91000_WriteData (REGADDR_MODECN, data);
}

float
LMP91000_GetTemp ()
{
  uint8_t last_mode = LMP91000_ReadData (REGADDR_MODECN);
  LMP91000_ChangeMode (OPMODE_TIA_ON);
  uint32_t adc_val = HAL_ADC_GetValue (&hadc1);
  float voltage = (adc_val / 4095) * ADC_REF; // in mV(millivolts)
  float temp = TEMP_SLOPE * voltage + TEMP_OFFSET;
  LMP91000_WriteData (REGADDR_MODECN, last_mode);
  lmp91000.temp = temp;
  return temp;
}

float
LMP91000_GetVout ()
{
  uint32_t adc_val = HAL_ADC_GetValue (&hadc1);
  float voltage = (adc_val / 4095) * ADC_REF; // in mV(millivolts)
  lmp91000.voltage = voltage;
  return voltage;
}

float
LMP91000_GetCurrent ()
{

  if (lmp91000.mode != OPMODE_AMPEROMETRIC && lmp91000.mode != OPMODE_TIA_ON) {
    // 如果不在测量模式，返回错误或无效值
    return -1.0f;
  }

  float v_out = LMP91000_GetVout ();

  // TIA_GAIN 数组的索引是从 0 开始的，而宏定义的值从 TIAGAIN_2K75(0x01) 开始
  // 所以需要做一个转换。注意 TIAGAIN_EXTR(0x00) 是外部电阻，这里暂不处理。
  float tia_gain_value;
  if (lmp91000.gain_level > 0 && lmp91000.gain_level <= 7) {
    tia_gain_value = TIA_GAIN[lmp91000.gain_level - 1]; // 索引 = level - 1
  } else {

    return -1.0f;
  }

  float v_ref = ADC_REF; // 单位: mV
  float v_zero;
  if (lmp91000.intz == INTZ_BYPASS) {
    v_zero = 0;
  } else {

    v_zero = v_ref * TIA_ZERO[lmp91000.intz];
  }

  float current = (v_out - v_zero) / tia_gain_value;

  lmp91000.current = current;
  return current;
}

void
LMP91000_StatusUpdate ()
{
  LMP91000_ToggleLock (UNLOCKCFG);
  LMP91000_ChangeMode (lmp91000.mode);

  LMP91000_SetTIAGain (lmp91000.gain_level);
  LMP91000_SetRLoad (lmp91000.rload);
  LMP91000_SetBias (lmp91000.bias);
  LMP91000_SetIntZero (lmp91000.intz);
  LMP91000_ToggleLock (lmp91000.lock);
}

void
LMP91000_Init ()
{
  HAL_ADC_Start (&hadc1);
  HAL_ADCEx_Calibration_Start (&hadc1, ADC_SINGLE_ENDED);
  HAL_ADC_PollForConversion (&hadc1, 50);

  lmp91000.mode = OPMODE_AMPEROMETRIC;
  lmp91000.lock = UNLOCKCFG;
  lmp91000.gain_level = TIAGAIN_035K;
  lmp91000.rload = RLOAD_033;
  lmp91000.intz = INTZ_20PCT;
  lmp91000.bias = BIAS_00PCT;
  lmp91000.temp = 25.0f;

  LMP91000_StatusUpdate ();
}
