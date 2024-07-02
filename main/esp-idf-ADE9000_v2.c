#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "ADE90xx.h"

// SPI interface definitions for ESP32s3
#define SPI_BUS SPI3_HOST
#define ADE_SPI_CS_GPIO 10
#define ADE_SPI_MOSI_GPIO 11 // SDI
#define ADE_SPI_SCK_GPIO 12
#define ADE_SPI_MISO_GPIO 13 // SDO

#define ADE_PM0_GPIO 14
#define ADE_PM1_GPIO 15

static const char *TAG = "app_main";

ADE90xx_Handler_t ade9000_dev = {
    .ADE_SPI_CS_HIGH = ADE_SPI_CS_HIGH,
    .ADE_SPI_CS_LOW = ADE_SPI_CS_LOW,
    .ADE_SPI_WriteByte = ADE_SPI_WriteByte,
    .ADE_SPI_ReadByte = ADE_SPI_ReadByte,
    .ADE_PM0_HIGH = ADE_PM0_HIGH,
    .ADE_PM0_LOW = ADE_PM0_LOW,
    .ADE_PM1_HIGH = ADE_PM1_HIGH,
    .ADE_PM1_LOW = ADE_PM1_LOW,
    .ADE_CF1_Read = ADE_CF1_Read,
    .ADE_CF2_Read = ADE_CF2_Read,
    .ADE_CF3_Read = ADE_CF3_Read,
    .ADE_CF4_Read = ADE_CF4_Read,
    .ADE_IRQ0_Read = ADE_IRQ0_Read,
    .ADE_IRQ1_Read = ADE_IRQ1_Read,
    .ADE_Delay_US = ADE_Delay_US,
    .ADE_Delay_MS = ADE_Delay_MS
};

// Set GPIO for CS high
void ADE_SPI_CS_HIGH(void) {
    gpio_set_level(ADE_SPI_CS_GPIO, 1);
}

// Set GPIO for CS low
void ADE_SPI_CS_LOW(void) {
    gpio_set_level(ADE_SPI_CS_GPIO, 0);
}

// Write a byte over SPI
void ADE_SPI_WriteByte(uint8_t Data) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &Data;
    spi_device_transmit(ade9000_dev, &t);
}

// Read a byte over SPI
uint8_t ADE_SPI_ReadByte(void) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    uint8_t rxData;
    t.rx_buffer = &rxData;
    spi_device_transmit(ade9000_dev, &t);
    return rxData;
}

// Power management functions
void ADE_PM0_HIGH(void) {
    gpio_set_level(ADE_PM0_GPIO, 1);
}

void ADE_PM0_LOW(void) {
    gpio_set_level(ADE_PM0_GPIO, 0);
}

void ADE_PM1_HIGH(void) {
    gpio_set_level(ADE_PM1_GPIO, 1);
}

void ADE_PM1_LOW(void) {
    gpio_set_level(ADE_PM1_GPIO, 0);
}

// Delay functions
void ADE_Delay_US(uint32_t us) {
    ets_delay_us(us);
}

void ADE_Delay_MS(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

// Initialize GPIOs
void init_gpio(void) {
    gpio_pad_select_gpio(ADE_SPI_CS_GPIO);
    gpio_set_direction(ADE_SPI_CS_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ADE_SPI_CS_GPIO, 1);

    gpio_pad_select_gpio(ADE_PM0_GPIO);
    gpio_set_direction(ADE_PM0_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ADE_PM0_GPIO, 0);

    gpio_pad_select_gpio(ADE_PM1_GPIO);
    gpio_set_direction(ADE_PM1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ADE_PM1_GPIO, 0);
}

// Function to initialize SPI bus
void init_spi_bus() {
    spi_bus_config_t bus_config = {
        .miso_io_num = ADE_SPI_MISO_GPIO,
        .mosi_io_num = ADE_SPI_MOSI_GPIO,
        .sclk_io_num = ADE_SPI_SCK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_BUS, &bus_config, SPI_DMA_CH_AUTO));
}

// Function to initialize ADE90xx
void init_ade9000(spi_device_handle_t *spi) {
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz
        .mode = 0, // SPI mode 0
        .spics_io_num = -1, // We manage CS manually
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI_BUS, &dev_config, spi));
}

void app_main(void) {
    init_gpio();
    init_spi_bus();

    init_ade9000(&ade9000_dev);

    // Initialize the ADE9000
    ADE90xx_Init(&ADE_Handler, true, true, false);
}
