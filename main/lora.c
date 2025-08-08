#include "lora.h"

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_gpio.h"
#include "freertos/idf_additions.h"
#include "soc/gpio_struct.h"

// Registers
#define REG_FIFO                 0x00
#define REG_OP_MODE              0x01
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08
#define REG_PA_CONFIG            0x09
#define REG_LNA                  0x0C
#define REG_FIFO_ADDR_PTR        0x0D
#define REG_FIFO_TX_BASE_ADDR    0x0E
#define REG_FIFO_RX_BASE_ADDR    0x0F
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_IRQ_FLAGS            0x12
#define REG_RX_NB_BYTES          0x13
#define REG_PKT_SNR_VALUE        0x19
#define REG_PKT_RSSI_VALUE       0x1A
#define REG_MODEM_CONFIG_1       0x1D
#define REG_MODEM_CONFIG_2       0x1E
#define REG_PREAMBLE_MSB         0x20
#define REG_PREAMBLE_LSB         0x21
#define REG_PAYLOAD_LENGTH       0x22
#define REG_MODEM_CONFIG_3       0x26
#define REG_RSSI_WIDEBAND        0x2C
#define REG_DETECTION_OPTIMIZE   0x31
#define REG_DETECTION_THRESHOLD  0x37
#define REG_SYNC_WORD            0x39
#define REG_DIO_MAPPING_1        0x40
#define REG_VERSION              0x42 // 0x12

// Modes
#define MODE_LONG_RANGE_MODE 0x80
#define MODE_SLEEP           0x00
#define MODE_STDBY           0x01
#define MODE_TX              0x03
#define MODE_RX_CONTINUOUS   0x05
#define MODE_RX_SINGLE       0x05

// IRQ
#define IRQ_RX_DONE_MASK 0x40

#define VERSION_TIMEOUT_RESET 100

#define READ_REG  0x7F
#define WRITE_REG 0x80

void lora_write_reg(const lora_device* device, int reg, int val)
{
    uint8_t out[2] = { reg | WRITE_REG, val };
    uint8_t in[2];
    spi_transaction_t t = {
        .flags     = SPI_TRANS_USE_TXDATA,
        .length    = 8 * sizeof(out),
        .tx_buffer = out,
        .rx_buffer = in,
    };

    ESP_ERROR_CHECK(spi_device_transmit(device->__spi, &t));
}

int lora_read_reg(const lora_device* device, int reg)
{
    uint8_t out[2] = { READ_REG & reg, 0x00 };
    uint8_t in[2];

    spi_transaction_t t = {
        .flags     = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length    = 8 * sizeof(out),
        .tx_buffer = out,
        .rx_buffer = in,
    };

    ESP_ERROR_CHECK(spi_device_transmit(device->__spi, &t));

    return in[1];
}

int lora_init(lora_device* device)
{
    gpio_reset_pin(device->rst);
    gpio_set_direction(device->rst, GPIO_MODE_OUTPUT);

    gpio_reset_pin(device->cs);
    gpio_set_direction(device->cs, GPIO_MODE_OUTPUT);
    gpio_set_level(device->cs, 1);

    spi_bus_config_t bus = {
        .miso_io_num     = device->miso,
        .mosi_io_num     = device->mosi,
        .sclk_io_num     = device->sck,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 0,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev = {
        .clock_speed_hz = 8E6,
        .mode           = 0,
        .spics_io_num   = device->cs,
        .queue_size     = 7,
        .flags          = 0,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev, &device->__spi));

    lora_reset(device);

    lora_dump_registers(device);

    uint8_t version;
    uint8_t i = 0;
    while (i++ < VERSION_TIMEOUT_RESET) {
        version = lora_read_reg(device, REG_VERSION);

        ESP_LOGD("LORA", "version=0x%02x", version);

        if (version == 0x12)
            break;

        vTaskDelay(2);
    }
    ESP_LOGD("LORA", "i=%d, TIMEOUT_RESET=%d", i, VERSION_TIMEOUT_RESET);
    if (i >= VERSION_TIMEOUT_RESET + 1)
        return 0;

    lora_sleep(device);
    lora_write_reg(device, REG_FIFO_RX_BASE_ADDR, 0);
    lora_write_reg(device, REG_FIFO_TX_BASE_ADDR, 0);
    lora_write_reg(device, REG_LNA, lora_read_reg(device, REG_LNA) | 0x03);
    lora_write_reg(device, REG_MODEM_CONFIG_3, 0x04);

    lora_idle(device);
    lora_set_frequency(device, device->freq);

    return 1;
}

void lora_idle(const lora_device* device)
{
    lora_write_reg(device, REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void lora_sleep(const lora_device* device)
{
    lora_write_reg(device, REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

void lora_receive(const lora_device* device)
{
    lora_write_reg(device, REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

void lora_reset(const lora_device* device)
{
    gpio_set_level(device->rst, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(device->rst, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void lora_set_frequency(lora_device* device, uint64_t frequency)
{
    device->freq = frequency;
    uint64_t frf = ((uint64_t)frequency << 19) / 32000000;
    lora_write_reg(device, REG_FRF_MSB, (uint8_t)(frf >> 16));
    lora_write_reg(device, REG_FRF_MID, (uint8_t)(frf >> 8));
    lora_write_reg(device, REG_FRF_LSB, (uint8_t)(frf >> 0));
}

void lora_set_sync_word(const lora_device* device, int sw)
{
    lora_write_reg(device, REG_SYNC_WORD, sw);
}

int lora_receive_packet(const lora_device* device, uint8_t* buf, int size)
{
    int len = 0;

    int irq = lora_read_reg(device, REG_IRQ_FLAGS);
    lora_write_reg(device, REG_IRQ_FLAGS, irq);
    if ((irq & IRQ_RX_DONE_MASK) == 0)
        return 0;

    len = lora_read_reg(device, REG_RX_NB_BYTES);

    lora_idle(device);
    lora_write_reg(device, REG_FIFO_ADDR_PTR, lora_read_reg(device, REG_FIFO_RX_CURRENT_ADDR));
    if (len > size)
        len = size;
    for (int i = 0; i < len; i++) {
        *buf++ = lora_read_reg(device, REG_FIFO);
    }

    return len;
}

int lora_received(const lora_device* device)
{
    if (lora_read_reg(device, REG_IRQ_FLAGS) & IRQ_RX_DONE_MASK)
        return 1;
    return 0;
}

int lora_packet_rssi(const lora_device* device)
{
    return (lora_read_reg(device, REG_PKT_RSSI_VALUE) - (device->freq < 868E6 ? 164 : 157));
}

void lora_dump_registers(const lora_device* device)
{
    int i;
    printf("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
    for (i = 0; i < 0x40; i++) {
        printf("%02X ", lora_read_reg(device, i));
        if ((i & 0x0F) == 0x0F)
            printf("\n");
    }
    printf("\n");
}
