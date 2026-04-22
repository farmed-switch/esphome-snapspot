#include "ES8388AudioSink.h"

struct es8388_cmd_s {
  uint8_t reg;
  uint8_t value;
};

ES8388AudioSink::ES8388AudioSink() {

  i2c_config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = 33,
      .scl_io_num = 32,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
  };

  i2c_config.master.clk_speed = 100000;

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100,
      .bits_per_sample = (i2s_bits_per_sample_t)16,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
      .intr_alloc_flags = 0,
      .dma_buf_count = 8,
      .dma_buf_len = 512,
      .use_apll = true,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 256 * 44100};

  i2s_pin_config_t pin_config = {
      .bck_io_num = 27,
      .ws_io_num = 25,
      .data_out_num = 26,
      .data_in_num = -1
  };

  int err;

  err = i2s_driver_install((i2s_port_t)0, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    ESP_LOGE("OI", "i2s driver installation error: %d", err);
  }

  err = i2s_set_pin((i2s_port_t)0, &pin_config);
  if (err != ESP_OK) {
    ESP_LOGE("OI", "i2s set pin error: %d", err);
  }

  err = i2c_param_config(I2C_NUM_0, &i2c_config);
  if (err != ESP_OK) {
    ESP_LOGE("OI", "i2c param config error: %d", err);
  }

  err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
  if (err != ESP_OK) {
    ESP_LOGE("OI", "i2c driver installation error: %d", err);
  }

  i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();

  err = i2c_master_start(i2c_cmd);
  if (err != ESP_OK) {
    ESP_LOGE("OI", "i2c master start error: %d", err);
  }

  writeReg(ES8388_DACCONTROL3, 0x04);
  writeReg(ES8388_CONTROL2, 0x50);
  writeReg(ES8388_CHIPPOWER, 0x00);
  writeReg(ES8388_MASTERMODE, 0x00);

  writeReg(ES8388_DACPOWER, 0x3e);
  writeReg(ES8388_CONTROL1, 0x12);

  writeReg(ES8388_DACCONTROL1, 0x18);
  writeReg(ES8388_DACCONTROL2, 0x02);

  writeReg(ES8388_DACCONTROL16, 0x1B);
  writeReg(ES8388_DACCONTROL17, 0x90);
  writeReg(ES8388_DACCONTROL20, 0x90);

  writeReg(ES8388_DACCONTROL21, 0x80);
  writeReg(ES8388_DACCONTROL23, 0x00);

  writeReg(ES8388_DACCONTROL5, 0x00);
  writeReg(ES8388_DACCONTROL4, 0x00);

  writeReg(ES8388_ADCPOWER, 0xff);
  writeReg(ES8388_ADCCONTROL1, 0x88);

  writeReg(ES8388_ADCCONTROL2, 0xf0);
  writeReg(ES8388_ADCCONTROL3, 0x80);
  writeReg(ES8388_ADCCONTROL4, 0x0e);
  writeReg(ES8388_ADCCONTROL5, 0x02);

  writeReg(ES8388_ADCCONTROL8, 0x20);
  writeReg(ES8388_ADCCONTROL9, 0x20);

  writeReg(ES8388_DACCONTROL24, 0x1e);
  writeReg(ES8388_DACCONTROL25, 0x1e);

  writeReg(ES8388_DACCONTROL26, 0x1e);
  writeReg(ES8388_DACCONTROL27, 0x1e);

  writeReg(ES8388_DACPOWER, 0x3c);
  writeReg(ES8388_DACCONTROL3, 0x00);
  writeReg(ES8388_ADCPOWER, 0x00);

  startI2sFeed();
}

void ES8388AudioSink::writeReg(uint8_t reg_add, uint8_t data) {

  int res = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  res |= i2c_master_start(cmd);
  res |= i2c_master_write_byte(cmd, ES8388_ADDR, ACK_CHECK_EN);
  res |= i2c_master_write_byte(cmd, reg_add, ACK_CHECK_EN);
  res |= i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
  res |= i2c_master_stop(cmd);
  res |= i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  if (res != ESP_OK) {
    ESP_LOGE("RR", "Unable to write to ES8388: %d", res);
  } else {
    ESP_LOGE("RR", "register successfull written.");
  }
}

ES8388AudioSink::~ES8388AudioSink() {}
