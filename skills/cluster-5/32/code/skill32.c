/*
Author: Nnenna Eze
Date: 12/1/2020
Team 15, Skill 32
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "driver/timer.h"
#include "soc/timer_group_struct.h"

static const char *TAG = "example";


#define PCNT_TEST_UNIT      PCNT_UNIT_0
#define PCNT_H_LIM_VAL      90000
#define PCNT_L_LIM_VAL     -90000
#define PCNT_THRESH1_VAL    6
#define PCNT_THRESH0_VAL   -6
#define PCNT_INPUT_SIG_IO   34//4  // Pulse Input GPIO
#define PCNT_INPUT_CTRL_IO  5  // Control GPIO HIGH=count up, LOW=count down


#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC   (1)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload

int16_t count = 0;
int16_t cflag = 0;
int16_t prev = 0;
float speed = 0;
float prev_err = 0;

typedef struct {
    int unit;  // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

typedef struct {
    uint64_t timer_counter_value;
    int pulse_count;
} timer_event_t;

xQueueHandle timer_queue;


/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
// static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
// {
//     int pcnt_unit = (int)arg;
//     pcnt_evt_t evt;
//     evt.unit = pcnt_unit;
//     /* Save the PCNT event type that caused an interrupt
//        to pass it to the main program */
//     pcnt_get_event_status(pcnt_unit, &evt.status);
//     xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
// }

/* Configure LED PWM Controller
 * to output sample pulses at 1 Hz with duty of about 10%
 */
// static void ledc_init(void)
// {
//     // Prepare and then apply the LEDC PWM timer configuration
//     ledc_timer_config_t ledc_timer;
//     ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE;
//     ledc_timer.timer_num        = LEDC_TIMER_1;
//     ledc_timer.duty_resolution  = LEDC_TIMER_10_BIT;
//     ledc_timer.freq_hz          = 1;  // set output frequency at 1 Hz
//     ledc_timer.clk_cfg = LEDC_AUTO_CLK;
//     ledc_timer_config(&ledc_timer);
//
//     // Prepare and then apply the LEDC PWM channel configuration
//     ledc_channel_config_t ledc_channel;
//     ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
//     ledc_channel.channel    = LEDC_CHANNEL_1;
//     ledc_channel.timer_sel  = LEDC_TIMER_1;
//     ledc_channel.intr_type  = LEDC_INTR_DISABLE;
//     ledc_channel.gpio_num   = LEDC_OUTPUT_IO;
//     ledc_channel.duty       = 100; // set duty at about 10%
//     ledc_channel.hpoint     = 0;
//     ledc_channel_config(&ledc_channel);
// }

/* Initialize PCNT functions:
 *  - configure and initialize PCNT
 *  - set up the input filter
 *  - set up the counter events to watch
 */
static void pcnt_example_init(int unit)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(PCNT_TEST_UNIT, 30);
    pcnt_filter_enable(PCNT_TEST_UNIT);

    // /* Set threshold 0 and 1 values and enable events to watch */
    // pcnt_set_event_value(unit, PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
    // pcnt_event_enable(unit, PCNT_EVT_THRES_1);
    // pcnt_set_event_value(unit, PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
    // pcnt_event_enable(unit, PCNT_EVT_THRES_0);
    // /* Enable events on zero, maximum and minimum limit values */
    // pcnt_event_enable(unit, PCNT_EVT_ZERO);
    // pcnt_event_enable(unit, PCNT_EVT_H_LIM);
    // pcnt_event_enable(unit, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_TEST_UNIT);
    pcnt_counter_clear(PCNT_TEST_UNIT);

    /* Install interrupt service and add isr callback handler */
    //pcnt_isr_service_install(0);
    //pcnt_isr_handler_add(unit, pcnt_example_intr_handler, (void *)unit);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(PCNT_TEST_UNIT);
}

void IRAM_ATTR timer_group0_isr(void *para) {

    // Prepare basic event data, aka set flag
    timer_event_t evt;
    //evt.flag = 1;
    uint64_t timer_counter_val =
        ((uint64_t) TIMERG0.hw_timer[TIMER_0].cnt_high) << 32
        | TIMERG0.hw_timer[TIMER_0].cnt_low;

        evt.timer_counter_val = timer_counter_val;
        evt.pulse_count = count - prev;

        prev = count;
    // Clear the interrupt, Timer 0 in group 0
    TIMERG0.int_clr_timers.t0 = 1;

    // After the alarm triggers, we need to re-enable it to trigger it next time
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

    // Send the event data back to the main program task
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

static void alarm_init() {
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = TEST_WITH_RELOAD;
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    // Timer's counter will initially start from value below
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

    // Configure the alarm value and the interrupt on alarm
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_SEC * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_group0_isr,
        (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

    // Start timer
    timer_start(TIMER_GROUP_0, TIMER_0);
}

static void timer_evt_task(void *arg) {
    while (1) {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        // Do something if triggered!
        if (evt.flag == 1) {
            printf("Action!\n");
        }
    }
}

void app_main(void)
{
    int pcnt_unit = PCNT_UNIT_0;
    /* Initialize LEDC to generate sample pulse signal */
    //ledc_init();

    /* Initialize PCNT event queue and PCNT functions */
    pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
    pcnt_example_init(pcnt_unit);

    int16_t count = 0;
    pcnt_evt_t evt;
    portBASE_TYPE res;
    while (1) {
        /* Wait for the event information passed from PCNT's interrupt handler.
         * Once received, decode the event type and print it on the serial monitor.
         */
        res = xQueueReceive(pcnt_evt_queue, &evt, 1000 / portTICK_PERIOD_MS);
        // if (res == pdTRUE) {
        //     pcnt_get_counter_value(pcnt_unit, &count);
        //     ESP_LOGI(TAG, "Event PCNT unit[%d]; cnt: %d", evt.unit, count);
        //     if (evt.status & PCNT_EVT_THRES_1) {
        //         ESP_LOGI(TAG, "THRES1 EVT");
        //     }
        //     if (evt.status & PCNT_EVT_THRES_0) {
        //         ESP_LOGI(TAG, "THRES0 EVT");
        //     }
        //     if (evt.status & PCNT_EVT_L_LIM) {
        //         ESP_LOGI(TAG, "L_LIM EVT");
        //     }
        //     if (evt.status & PCNT_EVT_H_LIM) {
        //         ESP_LOGI(TAG, "H_LIM EVT");
        //     }
        //     if (evt.status & PCNT_EVT_ZERO) {
        //         ESP_LOGI(TAG, "ZERO EVT");
        //     }
        // } else {
        //     pcnt_get_counter_value(pcnt_unit, &count);
        //     ESP_LOGI(TAG, "Current counter value :%d", count);
        // }
        pcnt_get_counter_value(PCNT_TEST_UNIT, &count);
       printf("Current counter value :%d\n", count);
    }
}
