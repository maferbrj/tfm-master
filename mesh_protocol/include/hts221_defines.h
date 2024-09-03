#ifndef HTS221_DEFINES_H
#define HTS221_DEFINES_H

#include "iris_boardv1.h"

/** Address */
#define HTS221_ADDR          0xBEU
#define HTS221_ADDR_7BITS    (HTS221_ADDR >> 1)

/** Registers */
#define HTS221_REG_WHO       0x0FU
#define HTS221_REG_CONF      0x10U
#define HTS221_REG_CTRL1     0x20U
#define HTS221_REG_CTRL2     0x21U
#define HTS221_REG_CTRL3     0x22U
#define HTS221_REG_STATUS    0x27U
#define HTS221_REG_HUM_H     0x29U
#define HTS221_REG_HUM_L     0x28U
#define HTS221_REG_TEMP_H    0x2BU
#define HTS221_REG_TEMP_L    0x2AU
/** Calibrtion */
#define HTS221_REG_H0_RH     0x30U
#define HTS221_REG_H1_RH     0x31U
#define HTS221_REG_T0_DEG    0x32U
#define HTS221_REG_T1_DEG    0x33U
#define HTS221_REG_T_MSB     0x35U
#define HTS221_REG_H0_OUT_H  0x37U
#define HTS221_REG_H0_OUT_L  0x36U
#define HTS221_REG_H1_OUT_H  0x3BU
#define HTS221_REG_H1_OUT_L  0x3AU
#define HTS221_REG_T0_OUT_H  0x3DU
#define HTS221_REG_T0_OUT_L  0x3CU
#define HTS221_REG_T1_OUT_H  0x3FU
#define HTS221_REG_T1_OUT_L  0x3EU

/** Bits */
// AV_CONF
#define HTS221_BIT_AVGT2     (1 << 5)
#define HTS221_BIT_AVGT1     (1 << 4)
#define HTS221_BIT_AVGT0     (1 << 3)
#define HTS221_POS_AVGT      3
#define HTS221_BIT_AVGH2     (1 << 2)
#define HTS221_BIT_AVGH1     (1 << 1)
#define HTS221_BIT_AVGH0     (1 << 0)
#define HTS221_POS_AVGH      0
// CTRL_REG1
#define HTS221_BIT_PD        (1 << 7)
#define HTS221_BIT_BDU       (1 << 2)
#define HTS221_BIT_ODR1      (1 << 1)
#define HTS221_BIT_ODR0      (1 << 0)
#define HTS221_POS_ODR       0
// CTRL_REG2
#define HTS221_BIT_BOOT      (1 << 7)
#define HTS221_BIT_HEATER    (1 << 1)
#define HTS221_BIT_ONE_SHOT  (1 << 0)
// CTRL_REG3
#define HTS221_BIT_DRDY_H_L  (1 << 7)
#define HTS221_BIT_PP_OD     (1 << 6)
#define HTS221_BIT_DRDY      (1 << 2)
// STATUS_REG
#define HTS221_BIT_H_DA      (1 << 1)
#define HTS221_BIT_T_DA      (1 << 0)
// Calibration
#define HTS221_POS_T0_MSB    0
#define HTS221_POS_T1_MSB    2
#define HTS221_MASK_T0_MSB   0x03U
#define HTS221_MASK_T1_MSB   0x0CU

typedef enum {
	HTS221_FREC_ONE_SHOT = 0b00U,
	HTS221_FREC_1HZ      = 0b01U,
	HTS221_FREC_7HZ      = 0b10U,
	HTS221_FREC_12HZ5    = 0b11U
} hts221_conf_frec_t;

typedef enum {
	HTS221_AVTEMP_2   = 0b000U,
	HTS221_AVTEMP_4   = 0b001U,
	HTS221_AVTEMP_8   = 0b010U,
	HTS221_AVTEMP_16  = 0b011U,
	HTS221_AVTEMP_32  = 0b100U,
	HTS221_AVTEMP_64  = 0b101U,
	HTS221_AVTEMP_128 = 0b110U,
	HTS221_AVTEMP_256 = 0b111U
} hts221_conf_avtemp_t;

typedef enum {
	HTS221_AVHUMD_4   = 0b000U,
	HTS221_AVHUMD_8   = 0b001U,
	HTS221_AVHUMD_16  = 0b010U,
	HTS221_AVHUMD_32  = 0b011U,
	HTS221_AVHUMD_64  = 0b100U,
	HTS221_AVHUMD_128 = 0b101U,
	HTS221_AVHUMD_256 = 0b110U,
	HTS221_AVHUMD_512 = 0b111U
} hts221_conf_avhumd_t;

#define HTS221_WHO 0xBCU

#endif // HTS221__DEFINES_H
