/* SKILL 7 CODE
  Nnenna Eze
  09/19/2020
   Team 15
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
//#define BLINK_GPIO CONFIG_BLINK_GPIO
#define LED_GREEN 26
#define LED_BLUE 25
#define LED_RED 4
#define LED_YELLOW 13

unsigned int dec_to_bin(unsigned int k) {
    return (k == 0 || k == 1 ? k : ((k % 2) + 10 * dec_to_bin(k / 2)));
}

void app_main(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(LED_GREEN);
    gpio_pad_select_gpio(LED_BLUE);
    gpio_pad_select_gpio(LED_RED);
    gpio_pad_select_gpio(LED_YELLOW);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BLUE, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_YELLOW, GPIO_MODE_OUTPUT);

    printf("Testing LEDs...\n");
    int num;
    int bin;
    int binnum[4];
    while(1) {
        /* Blink off (output low) */
        num = 0;
        while(num < 16){
          bin = dec_to_bin(num);
          binnum[0] = ((num < 8)? 0 : 1);
          binnum[1] = (bin >> 2) & 1U;
          binnum[2] = (bin >> 1) & 1U;
          binnum[3] = (bin >> 0) & 1U;
          printf("%d\n",num);
          printf("%d\n",bin);
          gpio_set_level(LED_GREEN, binnum[0]);
          //vTaskDelay(1000 / portTICK_PERIOD_MS);
          gpio_set_level(LED_BLUE,binnum[1]);
          //vTaskDelay(1000 / portTICK_PERIOD_MS);
          gpio_set_level(LED_RED,binnum[2]);
          //vTaskDelay(1000 / portTICK_PERIOD_MS);
          gpio_set_level(LED_YELLOW,binnum[3]);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          num++;
        }

    }
}
