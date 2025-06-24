#ifndef PINOUT_H
#define	PINOUT_H

/*
 * Pinout Map:
 * | Name | Arduino Pin # |
 * | PD0  |      0        |
 * | PD1  |      1        |
 * | PD2  |      2        |
 * | PD3  |      3        |
 * | PD4  |      4        |
 * | PD5  |      5        |
 * | PD6  |      6        |
 * | PD7  |      7        |
 * | PB0  |      8        |
 * | PB1  |      9        |
 * | PB2  |      10       |
 * | PB3  |      11       |
 * | PB4  |      12       |
 * | PB5  |      13       |
 * | PC0  |      14\A0    |
 * | PC1  |      15\A1    |
 * | PC2  |      16\A2    |
 * | PC3  |      17\A3    |
 * | PC4  |      18\A4    |
 * | PC5  |      19\A5    |
 * | PE2  |      20\A6    |
 * | PE3  |      21\A7    |
 * | PE0  |      22       |
 * | PE1  |      23       |
 */
#define PLAY_BUTTON_PIN 14
#define PLAY_LED_PIN 4
#define POWER_LED_PIN 8
#define POWER_METER_PIN A6

// BLE Pins
#define BLE_EN_PIN 15

// LED Pins
#define LED_A_PIN 19
#define LED_B_PIN 18

// Motor Pins
#define MOTOR_A_DIR 10
#define MOTOR_A_PWM 9
#define MOTOR_A_ENCODER 21
#define MOTOR_B_DIR 2
#define MOTOR_B_PWM 3
#define MOTOR_B_ENCODER 5
#define MOTOR_C_DIR 13
#define MOTOR_C_PWM 6
#define MOTOR_C_ENCODER 23

// Sensor Pins
#define SENSOR_A_PIN A3
#define SENSOR_B_PIN A2

// Sound Module Pins
#define SOUND_TX_PIN 22
#define SOUND_STATE_PIN 7

#endif	/* PINOUT_H */
