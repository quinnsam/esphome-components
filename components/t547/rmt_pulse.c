#include "rmt_pulse.h"

#include "driver/rmt_tx.h"
#include "esp_log.h"

static const char *TAG = "rmt_pulse";

static rmt_channel_handle_t rmt_chan = NULL;
static rmt_encoder_handle_t rmt_encoder = NULL;
static rmt_transmit_config_t rmt_tx_config = { .loop_count = 0 };

void rmt_pulse_init(gpio_num_t pin)
{
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = pin,
        .clk_src = RMT_CLK_SRC_DEFAULT, // 80MHz APB clock
        .resolution_hz = 10000000, // 10MHz, 0.1us resolution
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &rmt_chan));

    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &rmt_encoder));
    
    ESP_ERROR_CHECK(rmt_enable(rmt_chan));
}

void pulse_ckv_ticks(uint16_t high_time_ticks,
                               uint16_t low_time_ticks, bool wait)
{
    rmt_symbol_word_t raw_symbols[1];
    raw_symbols[0].level0 = 1;
    raw_symbols[0].duration0 = high_time_ticks;
    raw_symbols[0].level1 = 0;
    raw_symbols[0].duration1 = low_time_ticks;

    if (rmt_transmit(rmt_chan, rmt_encoder, raw_symbols, sizeof(raw_symbols), &rmt_tx_config) != ESP_OK) {
        ESP_LOGE(TAG, "rmt_transmit failed");
        return;
    }

    // Always wait for the transaction to finish.
    if (rmt_tx_wait_all_done(rmt_chan, -1) != ESP_OK) {
        ESP_LOGE(TAG, "rmt_tx_wait_all_done failed");
    }
}

void pulse_ckv_us(uint16_t high_time_us, uint16_t low_time_us,
                            bool wait)
{
    pulse_ckv_ticks(10 * high_time_us, 10 * low_time_us, wait);
}

bool rmt_busy()
{
    return false;
}