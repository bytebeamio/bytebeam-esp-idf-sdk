#include "sht31.h"
#include "esp_log.h"

i2c_port_t i2c_port = I2C_NUM_0;

esp_err_t sht31_init(void)
{

    i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_SDA,         // select SDA GPIO
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_io_num = I2C_SCL,         // select SCL GPIO
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_CLK_FREQ,  // clock frequency, we have specified 100KHz
    };
     esp_err_t err = i2c_param_config(i2c_port, &conf);
     if (err != ESP_OK) {
        return err;
    }

    return i2c_driver_install(i2c_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

uint8_t sht31_crc(uint8_t *data) {

    uint8_t crc = 0xff;
    int i, j;
    for(i = 0; i < 2; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if(crc & 0x80) {
                crc <<= 1;
                crc ^= 0x131;
            }
            else
                crc <<= 1;
        }
    }
    return crc;
}

esp_err_t sht31_read_temp_humi(float *temp, float *humi)
{
    unsigned char data_wr[] = {0x24, 0x00};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x44 << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, sizeof(data_wr), ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);

    vTaskDelay(2);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x44 << 1) | READ_BIT, ACK_CHECK_EN);
    size_t size = 6;
    uint8_t *data_rd = malloc(size);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        return ret;
    }
    i2c_cmd_link_delete(cmd);


    if(data_rd[2] != sht31_crc(data_rd) || 
       data_rd[5] != sht31_crc(data_rd + 3)) 
        return ESP_ERR_INVALID_CRC;
    *temp = -45 + (175 *(float)(data_rd[0] * 256 + data_rd[1])/ 65535.0);
    *humi = 100 * (float)(data_rd[3] * 256 + data_rd[4]) / 65535.0;

    return ESP_OK;
}

