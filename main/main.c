
#include <stdio.h>
#include <string.h>

#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "i2c-scanner";

#define I2C_MASTER_SCL_IO 4
#define I2C_MASTER_SDA_IO 5
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_PORT 0
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0

static esp_err_t i2c_master_init(void) {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };
  esp_err_t err = i2c_param_config(I2C_PORT, &conf);
  if (err != ESP_OK) {
    return err;
  }
  return i2c_driver_install(I2C_PORT, conf.mode, I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);
}

void app_main(void) {
  ESP_ERROR_CHECK(i2c_master_init());
  ESP_LOGI(TAG, "I2C Initialized Successfully");
  uint8_t address;
  printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
  for (int i = 0; i < 128; i += 16) {
    printf("%02x: ", i);
    for (int j = 0; j < 16; j++) {
      fflush(stdout);
      address = i + j;
      i2c_cmd_handle_t cmd = i2c_cmd_link_create();
      i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
      i2c_master_stop(cmd);
      esp_err_t ret =
          i2c_master_cmd_begin(I2C_PORT, cmd, 50 / portTICK_PERIOD_MS);
      i2c_cmd_link_delete(cmd);
      if (ret == ESP_OK) {
        ESP_LOGI(TAG, "%x", address);
        // printf("%02x ", address);
      } else if (ret == ESP_ERR_TIMEOUT) {
        printf("UU \n");
      } else {
        printf("-- \n");
      }
    }
    printf("\r\n");
  }
  i2c_driver_delete(I2C_PORT);
}