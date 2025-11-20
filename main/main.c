/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "hid_codes.h"
#include "hid_func.h"
#include "gpio_func.h"

/* for nvs_storage*/
#define LOCAL_NAMESPACE "storage"


#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0)
typedef  nvs_handle nvs_handle_t;
#endif

static const char *tag = "NimBLEKBD_main";
extern volatile bool s_is_connected;

nvs_handle_t Nvs_storage_handle = 0;

/* from ble_func.c */
extern void ble_init();

void mouse_movement_task(void *pvParameters) {
    ESP_LOGI(tag, "mouse movement task started");
    while (1) {
        if (s_is_connected) {
            hid_mouse_change_key(HID_MOUSE_NONE, 20, 0, true);
            vTaskDelay(pdMS_TO_TICKS(5000));
            hid_mouse_change_key(HID_MOUSE_NONE, -20, 0, true);
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        else {
            vTaskDelay(pdMS_TO_TICKS(2000));

        }
    }
}

void
app_main(void)
{
    /* Initialize NVS — it is used to store PHY calibration data and Nimble bonding data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK( nvs_open(LOCAL_NAMESPACE, NVS_READWRITE, &Nvs_storage_handle) );

    BaseType_t result = xTaskCreate(mouse_movement_task, "mouse_task", 4096, NULL, 1, NULL);
    if (result != pdPASS) {
        ESP_LOGE(tag, "Failed to create movement task (err=%ld)", result);
    }

    ble_init();
    ESP_LOGI(tag, "BLE init ok, waiting for buttons ...");
}
