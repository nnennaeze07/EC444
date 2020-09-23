/*Nnenna Eze (Team 15)
Date: 09/19/2020 */

#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define BLINK_GPIO 26

void app_main(void)
{
  char str[20];
  char strvar[20];
  int num = 0;
  int s = 1;
  bool tog = 0;

  gpio_pad_select_gpio(BLINK_GPIO);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);


//s; t
  while (1) {
    gets(str);
    //s->1 is toggle
    //s->2 is echo
    //s->3 is dec to hex
    if(str=='s' && s == 1){
      printf("toggle mode\n");
      printf("Read: \n");
      scanf("%c",&strvar[0]);
      if(strvar[0]=='t'){
        gpio_set_level(BLINK_GPIO, ~tog);
        tog = ~tog;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //toggle led, remember to flush after scanning
      }else if(strvar[0]=='s'){
        s++;
      }

    }else if(str=='s' && s == 2){
      printf("echo mode\n");
      scanf("%s",&strvar);
      if (strvar[0]=='s'){
        s++;
      }
      printf("echo:");
      printf("%s\n",strvar);
      fflush(stdin);
    }else if(str=='s' && s == 3){
      printf("Enter an integer: \n");
      scanf("%s\n",&strvar);
      if (strvar[0] == 's'){ //num cant read char, input char and turn into int?
        s = 1;
        printf("toggle mode\n");
      }else{
        num = atoi(strvar);
        printf("Hex: %X\n",num);
      }
      fflush(stdin);
    }
  }
}
