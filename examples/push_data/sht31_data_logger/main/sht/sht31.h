
#ifndef SHT31_H
#define SHT31_H

#include "esp_log.h"
#include "driver/i2c.h"


#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_CLK_FREQ 100000

#define I2C_MASTER_TX_BUF_DISABLE 0 
#define I2C_MASTER_RX_BUF_DISABLE 0
#define WRITE_BIT I2C_MASTER_WRITE            
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1

/**
 * @brief initialise i2c drivers for sht31
 * 
 * @return esp_err_t 
 */
esp_err_t sht31_init(void);

/**
 * @brief calculate crc for sht31 sensor
 * 
 * @param data 
 * @return uint8_t 
 */

uint8_t sht31_crc(uint8_t *data);

/**
 * @brief calculates data samples for temperature and humidty takes referance to temp and humidty 
 * 
 * @param temp 
 * @param humi 
 * @return esp_err_t 
 */

esp_err_t sht31_read_temp_humi(float *temp, float *humi);

#endif
