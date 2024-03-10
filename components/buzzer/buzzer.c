#include "buzzer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

const char *TAG = "buzzer";

void init_buzzer()
{
    gpio_reset_pin(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_GPIO, 0);
}

void alarm_buzzer()
{
    ESP_LOGI(TAG, "Playing buzzer alarm");
    gpio_set_level(BUZZER_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(BUZZER_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(BUZZER_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(BUZZER_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(BUZZER_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(BUZZER_GPIO, 0);
    ESP_LOGI(TAG, "Done playing buzzer alarm");
}