#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "string.h"

#define BLINK_GPIO 2
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define UART UART_NUM_2

static int led_state = 0;
static const int RX_BUF_SIZE = 1024;
static const int TX_BUF_SIZE = 1024;

static int numOfSend = 0;

void init_GPIO(void){
    gpio_set_direction(BLINK_GPIO,GPIO_MODE_OUTPUT);

}
void init_UART(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}


void blink_task(void* pvParameter){
    while (1) {
        // Blink on (set the GPIO level high)
        printf("LED: %s!\n", led_state ? "ON" : "OFF");
        gpio_set_level(BLINK_GPIO, led_state);
        led_state = !led_state;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void uart_sending_task(void* pvParameter){
    char* Tx_data = (char*) malloc(TX_BUF_SIZE + 1); 
    while (1){
        // printf("uart on process");
        // uart_write_bytes(UART, data, len);
        sprintf(Tx_data,"Hello I am TXN num %d\n", numOfSend++);
        printf("Data sent: %s",Tx_data);
        uart_write_bytes(UART, Tx_data, strlen(Tx_data));
        vTaskDelay(2000/portTICK_PERIOD_MS );
    }   
    free(Tx_data);
}

void  uart_receive_task(void* pvParameter){
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE + 1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            printf("Data recieved: %s\n",data);
        }
    }
    free(data);
}
void app_main(void)
{   
    init_GPIO();
    init_UART();
    xTaskCreate(&blink_task, "blink_task", 2048, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(&uart_sending_task,"uart_sending_task", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(&uart_receive_task,"uart_receive_task", 2048, NULL, configMAX_PRIORITIES -1, NULL);
}
