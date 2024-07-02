#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "ADE90xx.h"

// SPI interface definitions for ESP32s3
#define SPI_BUS SPI3_HOST

#define SPI_BUS_MOSI_GPIO 11 // SDI
#define SPI_BUS_SCK_GPIO 12 //CLK
#define SPI_BUS_MISO_GPIO 13 // SDO

#define ADE_SPI_CS_GPIO 10
#define ADE_PM0_GPIO 14
#define ADE_PM1_GPIO 15

static const char *TAG = "app_main";

spi_device_handle_t ade9000_dev;

// Function prototypes
void ADE_SPI_CS_HIGH(void);
void ADE_SPI_CS_LOW(void);
void ADE_SPI_WriteByte(uint8_t Data);
uint8_t ADE_SPI_ReadByte(void);
void ADE_PM0_HIGH(void);
void ADE_PM0_LOW(void);
void ADE_PM1_HIGH(void);
void ADE_PM1_LOW(void);
void ADE_Delay_US(uint32_t us);
void ADE_Delay_MS(uint32_t ms);

// Initialize your handler structure
ADE90xx_Handler_t ade9000_handler = {
    .ADE_SPI_CS_HIGH = ADE_SPI_CS_HIGH,
    .ADE_SPI_CS_LOW = ADE_SPI_CS_LOW,
    .ADE_SPI_WriteByte = ADE_SPI_WriteByte,
    .ADE_SPI_ReadByte = ADE_SPI_ReadByte,
    .ADE_PM0_HIGH = ADE_PM0_HIGH,
    .ADE_PM0_LOW = ADE_PM0_LOW,
    .ADE_PM1_HIGH = ADE_PM1_HIGH,
    .ADE_PM1_LOW = ADE_PM1_LOW,
    .ADE_CF1_Read = NULL,  // Replace with actual function if needed
    .ADE_CF2_Read = NULL,  // Replace with actual function if needed
    .ADE_CF3_Read = NULL,  // Replace with actual function if needed
    .ADE_CF4_Read = NULL,  // Replace with actual function if needed
    // .ADE_IRQ0_Read = NULL, // Replace with actual function if needed
    // .ADE_IRQ1_Read = NULL, // Replace with actual function if needed
    .ADE_Delay_US = ADE_Delay_US,
    .ADE_Delay_MS = ADE_Delay_MS
};



// Set GPIO for CS high
void ADE_SPI_CS_HIGH(void) {
    //printf("ADE_SPI_CS_HIGH...\n");
    gpio_set_level(ADE_SPI_CS_GPIO, 1);
    //ADE_Delay_MS(10);
    //printf("GPIO%d level: %d\n", ADE_SPI_CS_GPIO, gpio_get_level(ADE_SPI_CS_GPIO));
}

// Set GPIO for CS low
void ADE_SPI_CS_LOW(void) {
    //printf("ADE_SPI_CS_LOW...\n");
    gpio_set_level(ADE_SPI_CS_GPIO, 0);
    //ADE_Delay_MS(10);
    //printf("GPIO%d level: %d\n", ADE_SPI_CS_GPIO, gpio_get_level(ADE_SPI_CS_GPIO));
}

// Write a byte over SPI
void ADE_SPI_WriteByte(uint8_t Data) {
    ////printf("ADE_SPI_WriteByte...\n");
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &Data;
    spi_device_transmit(ade9000_dev, &t);
}

// Read a byte over SPI
uint8_t ADE_SPI_ReadByte(void) {
    ////printf("ADE_SPI_ReadByte...\n");
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.flags = SPI_TRANS_USE_RXDATA;
    spi_device_transmit(ade9000_dev, &t);
    return t.rx_data[0];
}

// Power management functions
void ADE_PM0_HIGH(void) {
    ////printf("ADE_PM0_HIGH...\n");
    gpio_set_level(ADE_PM0_GPIO, 1);
}

void ADE_PM0_LOW(void) {
    ////printf("ADE_PM0_LOW...\n");
    gpio_set_level(ADE_PM0_GPIO, 0);
}

void ADE_PM1_HIGH(void) {
    ////printf("ADE_PM1_HIGH...\n");
    gpio_set_level(ADE_PM1_GPIO, 1);
}

void ADE_PM1_LOW(void) {
    ////printf("ADE_PM1_LOW...\n");
    gpio_set_level(ADE_PM1_GPIO, 0);
}

// Delay US function
void ADE_Delay_US(uint32_t us) {
    ////printf("ADE_Delay_US...%d us\n", us);
    ets_delay_us(us);
}
// Delay MS function
void ADE_Delay_MS(uint32_t ms) {
    ////printf("ADE_Delay_MS...%d ms\n", ms);
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

// Function to Initialize ESP32 GPIOs
void init_ESP32_gpio(void) {
    ////printf("starting init_ESP32_gpio...\n");
    gpio_pad_select_gpio(ADE_SPI_CS_GPIO);
    gpio_set_direction(ADE_SPI_CS_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(ADE_SPI_CS_GPIO, 1);

    // Configure output rgb_led EN pin
    // gpio_config_t io_conf = {
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pull_up_en = 0,
    // };
    // io_conf.pin_bit_mask = ((uint64_t)1 << ADE_SPI_CS_GPIO);
    // gpio_config(&io_conf); // Configure the GPIO


    gpio_pad_select_gpio(ADE_PM0_GPIO);
    gpio_set_direction(ADE_PM0_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(ADE_PM0_GPIO, 0);

    gpio_pad_select_gpio(ADE_PM1_GPIO);
    gpio_set_direction(ADE_PM1_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(ADE_PM1_GPIO, 0);
}

// Function to initialize ESP32 SPI bus
void init_esp32_spi_bus() {
    ////printf("starting init_esp32_spi_bus...\n");
    spi_bus_config_t bus_config = {
        .miso_io_num = SPI_BUS_MISO_GPIO,
        .mosi_io_num = SPI_BUS_MOSI_GPIO,
        .sclk_io_num = SPI_BUS_SCK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_BUS, &bus_config, SPI_DMA_CH_AUTO));
}

// Function to initialize and add ADE9000 to the ESP32 SPI BUS 
void init_add_device_spi_bus(spi_device_handle_t *spi_device, uint8_t cs_io_num) {
    ////printf("starting init_add_device_spi_bus...\n");
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 12 * 1000 * 1000, // 12 MHz
        .mode = 0, // SPI mode 0: CPOL=0, CPHA=0
        .spics_io_num = -1, //cs_io_num: load from function parameter, // -1: We manage CS manually
        .queue_size = 1,

        // .flags = 0,              // no flags set
        // .command_bits = 0,       // no command bits used
        // .address_bits = 0,       // register address is first byte in MOSI
        // .dummy_bits = 0          // no dummy bits used
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI_BUS, &dev_config, spi_device));
}

void app_main(void) {
    ////printf("starting main...\n");
    init_ESP32_gpio();
    ADE_Delay_MS(100);
    init_esp32_spi_bus();
    ADE_Delay_MS(100);

    init_add_device_spi_bus(&ade9000_dev, ADE_SPI_CS_GPIO);
    ADE_Delay_MS(100);

    while(1) {
    // Initialize the ADE9000
    ADE90xx_Init(&ade9000_handler, true, true, false);

    // ADE90xx_InitLL();
    // uint32_t Data;
    // uint32_t dataRead = ADE90xx_ReadRegLL(&ade9000_handler, ADE_ADD_PART_ID, ADE_SIZE_PART_ID, ADE_SB_PART_ID, ADE_MB_PART_ID, &Data);
    // printf("Register value: 0x%08X\n", dataRead);//Log the result

    ADE_Delay_MS(1000);
    }
}





    // // Example usage of ADE90xx_ReadReg16
    // uint16_t reg_value;
    // ADE90xx_ReadReg16(&ade9000_handler, 0x480, 0xFFFF, &reg_value);
    // //printf("Read 16-bit register value: 0x%04X\n", reg_value);

    // // Example usage of ADE90xx_WriteReg16
    // ADE90xx_WriteReg16(&ade9000_handler, 0x480, 0xFFFF, 0x1234);

    // // Example usage of ADE90xx_ReadReg32
    // uint32_t reg_value32;
    // ADE90xx_ReadReg32(&ade9000_handler, 0x500, 0xFFFFFFFF, &reg_value32);
    // //printf("Read 32-bit register value: 0x%08X\n", reg_value32);

    // // Example usage of ADE90xx_WriteReg32
    // ADE90xx_WriteReg32(&ade9000_handler, 0x500, 0xFFFFFFFF, 0x12345678);