/* Author: Nnenna Eze
Date: 12/01/2020
Team 15, Skill 33
*/


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"
#include <sys/time.h>
#include "driver/gpio.h"

#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_vfs_dev.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RMT_TX_CHANNEL 1 /* RMT channel for transmitter */
#define RMT_TX_GPIO_NUM PIN_TRIGGER /* GPIO number for transmitter signal */
#define RMT_RX_CHANNEL 0 /* RMT channel for receiver */
#define RMT_RX_GPIO_NUM PIN_ECHO /* GPIO number for receiver */
#define RMT_CLK_DIV 100 /* RMT counter clock divider */
#define RMT_TX_CARRIER_EN 0 /* Disable carrier */
#define rmt_item32_tIMEOUT_US 9500 /*!< RMT receiver timeout value(us) */

#define RMT_TICK_10_US (80000000/RMT_CLK_DIV/100000) /* RMT counter value for 10 us.(Source clock is APB clock) */
#define ITEM_DURATION(d) ((d & 0x7fff)*10/RMT_TICK_10_US)

#define PIN_TRIGGER 25 // Recommended trigger and echo value
#define PIN_ECHO 26

// Define for PID Stuff
#define LED1 13 // GPIO 13, Blue LED
#define LED2 5 // GPIO SCK(5), Green LED
#define LED3 12 // GPIO 12, Yellow LED

// Global Variables for PID
float previous_error = 0;
float integral = 0;
double output = 0;
float error = 0;
int setpoint = 50; // 50 cm setpoint
double dt = 100; // 100ms period
double derivative = 0;
double Kp = 1;
double Ki = 0;
double Kd = 0;
float measured_value;

static void HCSR04_tx_init(){
  rmt_config_t rmt_tx;
  rmt_tx.channel = RMT_TX_CHANNEL;
  rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
  rmt_tx.mem_block_num = 1;
  rmt_tx.clk_div = RMT_CLK_DIV;
  rmt_tx.tx_config.loop_en = false;
  rmt_tx.tx_config.carrier_duty_percent = 50;
  rmt_tx.tx_config.carrier_freq_hz = 3000;
  rmt_tx.tx_config.carrier_level = 1;
  rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
  rmt_tx.tx_config.idle_level = 0;
  rmt_tx.tx_config.idle_output_en = true;
  rmt_tx.rmt_mode = 0;
  rmt_config(&rmt_tx);
  rmt_driver_install(rmt_tx.channel, 0, 0);
}

static void HCSR04_rx_init() {
  rmt_config_t rmt_rx;
  rmt_rx.channel = RMT_RX_CHANNEL;
  rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
  rmt_rx.clk_div = RMT_CLK_DIV;
  rmt_rx.mem_block_num = 1;
  rmt_rx.rmt_mode = RMT_MODE_RX;
  rmt_rx.rx_config.filter_en = true;
  rmt_rx.rx_config.filter_ticks_thresh = 100;
  rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
  rmt_config(&rmt_rx);
  rmt_driver_install(rmt_rx.channel, 1000, 0);
}

void app_main() {
  HCSR04_tx_init();
  HCSR04_rx_init();

  rmt_item32_t item;
  item.level0 = 1;
  item.duration0 = RMT_TICK_10_US;
  item.level1 = 0;
  item.duration1 = RMT_TICK_10_US; // for one pulse this doesn't matter

  size_t rx_size = 0;
  RingbufHandle_t rb = NULL;
  rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);
  rmt_rx_start(RMT_RX_CHANNEL, 1);

  double distance = 0;

  for(;;){
    rmt_write_items(RMT_TX_CHANNEL, &item, 1, true);
    rmt_wait_tx_done(RMT_TX_CHANNEL, portMAX_DELAY);

    rmt_item32_t* item = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, 1000);
    distance = 340.29 * ITEM_DURATION(item->duration0) / (1000 * 1000 * 2); // distance in meters
    printf("Distance is %f cm\n", distance * 100); // distance in centimeters

    vRingbufferReturnItem(rb, (void*) item);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    measured_value = distance * 100;  // keeping it in cm
    printf("measured_value = %f\n", measured_value);

    /* Set the GPIO as a push/pull output */
    gpio_pad_select_gpio(LED1);
    gpio_pad_select_gpio(LED2);
    gpio_pad_select_gpio(LED3);

    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED3, GPIO_MODE_OUTPUT);

    //while(1) {
      error = setpoint - measured_value;
      printf("Error = %f\n", error);
      integral = integral + error * dt;
      derivative = (error - previous_error) / dt;
      output = Kp * error + Ki * integral + Kd * derivative;
      previous_error = error;
      vTaskDelay(dt);

      if (error < 0){ // turn on yellow led
        gpio_set_level(LED1, 0);
        gpio_set_level(LED2, 0);
        gpio_set_level(LED3, 1);
        //vTaskDelay(100);
      }
      else if (error == 0) { // turn on green led
        gpio_set_level(LED1, 0);
        gpio_set_level(LED2, 1);
        gpio_set_level(LED3, 0);

      }
      else { // turn on blue led
        gpio_set_level(LED1, 1);
        gpio_set_level(LED2, 0);
        gpio_set_level(LED3, 0);
      }
  }
}
