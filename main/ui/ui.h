#pragma once

#include "esp_err.h"

esp_err_t ui_init(void);

void ui_set_flow_rate(float flow_rate);
void ui_set_environment(float temperature, float humidity);