/*Authors: Nnenna Eze, Lesbeth Roque (Team Skill - 28)
Date: 11/10/2020
Team 15
BSD Socket API Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "addr_from_stdin.h"

// #if defined(CONFIG_EXAMPLE_IPV4)
// #define HOST_IP_ADDR "192.168.1.132"//CONFIG_EXAMPLE_IPV4_ADDR
// #elif defined(CONFIG_EXAMPLE_IPV6)
// #define HOST_IP_ADDR "192.168.1.132"//CONFIG_EXAMPLE_IPV6_ADDR
// #else
// #define HOST_IP_ADDR "192.168.1.132"
// #endif

#define PORT CONFIG_EXAMPLE_PORT
#define BLUEPIN 14
#define GREENPIN 32
#define REDPIN 15
#define ONBOARD 13
#define ID 3
#define DEVNUM 3

#define TIMER_DIVIDER 16
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_INTERVAL_2_SEC (2)
#define TEST_WITH_RELOAD 1

SemaphoreHandle_t mux = NULL;
static xQueueHandle gpio_evt_queue = NULL;
static xQueueHandle timer_queue;

typedef struct{
  int flag;
}timer_event_t;

static const char *TAG = "example";
static const char *payload = "leader";

char ips[DEVNUM][16] = {
  "192.168.1.132",
  "192.168.1.113",
  "192.168.1.124"
};

int count = 0;
int myID = (int)ID;
int status = 1; //1 for leader, 0 for nonleader
int minval = (int)ID;
int election = 0; //1 for election happening/candidiate; 0 for leaving election/no longer candidate/no election
//int devicenum? keep track of number of devices, so you know when new one joins
//char lp; //to send leader "heartbeat", "L" for leader, or "P" for participant

void IRAM_ATTR timer_group0_isr(void *para){
  timer_event_t evt;
  evt.flag = 1;

  TIMERG0.int_clr_timers.t0 = 1;

  TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

  xQueueSendFromISR(timer_queue, &evt, NULL);
}

// static void config_ip_init(){
//
// }

static void led_init(){
  gpio_pad_select_gpio(BLUEPIN);
  gpio_pad_select_gpio(GREENPIN);
  gpio_pad_select_gpio(REDPIN);
  gpio_pad_select_gpio(ONBOARD);

  gpio_set_direction(BLUEPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(ONBOARD, GPIO_MODE_OUTPUT);

  uart_driver_install(UART_NUM_1, 1024 * 2, 0, 0, NULL, 0);
}

static void alarm_init(){
  timer_config_t config;
  config.divider = TIMER_DIVIDER;
  config.counter_dir = TIMER_COUNT_UP;
  config.counter_en = TIMER_PAUSE;
  config.alarm_en = TIMER_ALARM_EN;
  config.intr_type = TIMER_INTR_LEVEL;
  config.auto_reload = TEST_WITH_RELOAD;
  timer_init(TIMER_GROUP_0, TIMER_0, &config);

  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_2_SEC * TIMER_SCALE);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_group0_isr, (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

  if(status == 1){
      timer_start(TIMER_GROUP_0, TIMER_0);
  }
}

static void led_task(){
  while(1){
    if(election == 1){
      //printf("Election LED on...\n");
      gpio_set_level(REDPIN, 1);
      gpio_set_level(GREENPIN, 0);
      gpio_set_level(BLUEPIN, 0);
    }else if (status == 1 && election == 0){
      gpio_set_level(REDPIN, 0);
      gpio_set_level(GREENPIN, 1);
      gpio_set_level(BLUEPIN, 0);
    }else if (status == 0 && election == 0){
      gpio_set_level(REDPIN, 0);
      gpio_set_level(GREENPIN, 0);
      gpio_set_level(BLUEPIN, 1);
    }
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}


static void udp_server_task(void *pvParameters)
{
  //char message[16];
  char rx_buffer_in[5];
  char addr_str[128];
  int addr_family = (int)pvParameters;
  int ip_protocol = 0;
  struct sockaddr_in6 dest_addr;

    while (1) {

      if (addr_family == AF_INET) {
                 struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
                 dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
                 dest_addr_ip4->sin_family = AF_INET;
                 dest_addr_ip4->sin_port = htons(PORT);
                 ip_protocol = IPPROTO_IP;
             } else if (addr_family == AF_INET6) {
                 bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
                 dest_addr.sin6_family = AF_INET6;
                 dest_addr.sin6_port = htons(PORT);
                 ip_protocol = IPPROTO_IPV6;
             }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        while (1) {

            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer_in, sizeof(rx_buffer_in) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (source_addr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }
                rx_buffer_in[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                //ESP_LOGI(TAG, "%s", rx_buffer_in);

                if(minval!=(rx_buffer_in[1]-'0') || (rx_buffer_in[3]-'0') == 1){
                  election = 1;
                }else if ((minval == (rx_buffer_in[1]-'0')) && myID == minval){
                  //leader, so send out leader meesage through enabling timer function
                  status = 1;
                  printf("I am current leader\n");
                }else if((minval == (rx_buffer_in[1]-'0')) && myID != minval){
                  status = 0;
                  printf("I have Non-leader status\n");
                }

                if(election == 1){
                  printf("Election occurring...\n");
                  if(minval == (rx_buffer_in[1]-'0')){
                    election = 0;
                    printf("Election over\n");
                  }else if(myID > (rx_buffer_in[0]-'0')){
                    minval = rx_buffer_in[1]-'0';
                    status = 0;
                    printf("Status changed\n");
                  }
                }

                int err = sendto(sock,payload, strlen(payload), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));

                //data to send back
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }else{
                  printf("Server sent leader status \n");
                }
            }
            //vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


static void udp_client_task(void *pvParameters)
{
  char rx_buffer_out[5];
  //char host_ip[] = HOST_IP_ADDR;
  int addr_family = 0;
  int ip_protocol = 0;
  int socks[DEVNUM];
  struct sockaddr_in addr[DEVNUM];
  for(int i = 0; i <= DEVNUM; i++){
    //strlen(ips[i])-1 --> would replace 12
    //if(ips[i][12]!=(myID+'0')){
      struct sockaddr_in dest_addr;
      dest_addr.sin_addr.s_addr = inet_addr(ips[i]);
      dest_addr.sin_family = AF_INET;
      dest_addr.sin_port = htons(PORT);
      addr_family = AF_INET;
      ip_protocol = IPPROTO_IP;

      int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
      if (sock < 0) {
          ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
          break;
      }
      ESP_LOGI(TAG, "Socket created, sending to %s:%d", ips[i], PORT);
      socks[i] = sock;
      addrs[i] = dest_addr;
    //}
  }

  //  while (1) {

// #if defined(CONFIG_EXAMPLE_IPV4)
//         struct sockaddr_in dest_addr;
//         dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
//         dest_addr.sin_family = AF_INET;
//         dest_addr.sin_port = htons(PORT);
//         addr_family = AF_INET;
//         ip_protocol = IPPROTO_IP;
// #elif defined(CONFIG_EXAMPLE_IPV6)
//         struct sockaddr_in6 dest_addr = { 0 };
//         inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
//         dest_addr.sin6_family = AF_INET6;
//         dest_addr.sin6_port = htons(PORT);
//         dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
//         addr_family = AF_INET6;
//         ip_protocol = IPPROTO_IPV6;
// #elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
//         struct sockaddr_in6 dest_addr = { 0 };
//         ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
// #endif

        while (1) {
          rx_buffer_out[0] = myID + '0';//id
          rx_buffer_out[1] = minval + '0';//minVal
          rx_buffer_out[2] = status +'0';//status
          rx_buffer_out[3] = election + '0';
          printf("Sending: ID %d, minval %d and status %d\n",ID, minval, status);
          for(int j=0;j<=DEVNUM-1;j++){

            int err = sendto(socks[j], rx_buffer_out, strlen(rx_buffer_out), 0, (struct sockaddr *)&addrs[j], sizeof(addrs[j]));
            if (err < 0) {
                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                 break;
             }
          }

             printf("%s\n",rx_buffer_out);
             ESP_LOGI(TAG, "Status sent");

            struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            // int len = recvfrom(sock, rx_buffer_in, sizeof(rx_buffer_in) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            //
            // if (len < 0) {
            //     ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            //     break;
            // }
            // // Data received
            // else {
            //     rx_buffer_in[len] = 0; // Null-terminate whatever we received and treat like a string
            //     ESP_LOGI(TAG, "Client received %d bytes from %s:", len, host_ip);
            //
            //
            // }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    //}
    vTaskDelete(NULL);
}
//figure out how to end timer if leader disconnected, signal to other devices
static void timer_evt_task(void *arg){
  while(1){
    timer_event_t evt;

    xQueueReceive(timer_queue, &evt, portMAX_DELAY);

    if(evt.flag == 1){
      printf("Timer message sent: ");
      if(status == 1){
        payload = "leader";
      }else if (status == 0){
        payload = "connected";
      }
      printf("%s\n",payload);
    }
  }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());


    timer_queue = xQueueCreate(2, sizeof(timer_event_t));

    xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);

    //config_ip_init();
    alarm_init();
    led_init();

    xTaskCreate(led_task, "led_task", 1024,NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
    xTaskCreate(udp_server_task, "udp_server", 4096, (void*)AF_INET, 5, NULL);

  }
