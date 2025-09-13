#include "lora.h"

#include <math.h>

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "esp_rom_gpio.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "soc/gpio_struct.h"
#include "string.h"

#define CMD_SET_SLEEP                0x84
#define CMD_SET_STANDBY              0x80
#define CMD_SET_FS                   0xC1
#define CMD_SET_TX                   0x83
#define CMD_SET_RX                   0x82
#define CMD_STOP_TIMER_ON_PREAMBLE   0x9F
#define CMD_SET_RX_DUTY_CYCLE        0x94
#define CMD_SET_CAD                  0xC5
#define CMD_SET_TX_CONTINUOUS_WAVE   0xD1
#define CMD_SET_TX_INFINITE_PREAMBLE 0xD2
#define CMD_SET_REGULATOR_MODE       0x96
#define CMD_CALIBRATE                0x89
#define CMD_CALIBRATE_IMAGE          0x98
#define CMD_SET_PA_CONFIG            0x95
#define CMD_SET_RX_TX_FALLBACK_MODE  0x93

#define CMD_WRITE_REGISTER 0x0D
#define CMD_READ_REGISTER  0x1D
#define CMD_WRITE_BUFFER   0x0E
#define CMD_READ_BUFFER    0x1E

#define CMD_DIO_IRQ_PARAMS             0x08
#define CMD_GET_IRQ_STATUS             0x12
#define CMD_CLEAR_IRQ_STATUS           0x02
#define CMD_SET_DIO2_AS_RF_SWITCH_CTRL 0x9D
#define CMD_SET_DIO3_AS_TCXO_CTRL      0x97

#define CMD_SET_RF_FREQUENCY      0x86
#define CMD_SET_PACKET_TYPE       0x8A
#define CMD_GET_PACKET_TYPE       0x11
#define CMD_SET_TX_PARAMS         0x8E
#define CMD_SET_MODULATION_PARAMS 0x8B

#define CMD_SET_PACKET_PARAMS         0x8C
#define CMD_SET_CAD_PARAMS            0x88
#define CMD_SET_BUFFER_BASE_ADDRESS   0x8F
#define CMD_SET_LORA_SYMB_NUM_TIMEOUT 0xA0

#define CMD_GET_STATUS           0xC0
#define CMD_GET_RSSI_INST        0x15
#define CMD_GET_RX_BUFFER_STATUS 0x13
#define CMD_GET_PACKET_STATUS    0x14
#define CMD_GET_DEVICE_ERRORS    0x17
#define CMD_CLEAR_DEVICE_ERRORS  0x07
#define CMD_GET_STATS            0x10
#define CMD_RESET_STATS          0x00

// #define REG_WHITENING_INITIAL_VALUE_MSB 0x06B8
// #define REG_WHITENING_INITIAL_VALUE_LSB 0x06B9
#define REG_LORA_SYNC_WORD_MSB 0x0740
#define REG_LORA_SYNC_WORD_LSB 0x0741
#define REG_RNG_0              0x0819
#define REG_RNG_1              0x081A
#define REG_RNG_2              0x081B
#define REG_RNG_3              0x081C
#define REG_RX_GAIN            0x08AC
#define REG_OCP_CONF           0x08E7
#define REG_XTA_TRIM           0x0911
#define REG_XTB_TRIM           0x0912

#define SYNC_WORD_PRIVATE 0x1424
#define SYNC_WORD_PUBLIC  0x3444

#define IRQ_ALL               0b1111111111
#define IRQ_TX_DONE           1 << 0
#define IRQ_RX_DONE           1 << 1
#define IRQ_PREAMBLE_DETECTED 1 << 2
#define IRQ_SYNC_WORD_VALID   1 << 3
#define IRQ_HEADER_VALId      1 << 4
#define IRQ_HEADER_ERR        1 << 5
#define IRQ_CRC_ERR           1 << 6
#define IRQ_CAD_DONE          1 << 7
#define IRQ_CAD_DETECTED      1 << 8
#define IRQ_TIMEOUT           1 << 9

#define HEADER_TYPE_EXP 0x0
#define HEADER_TYPE_IMP 0x1
#define CRC_OFF         0x0
#define CRC_ON          0x1
#define IQ_STANDARD     0x0
#define IQ_INVERT       0x1

#define XTAL_FREQ 32000000.0
#define FREQ_DIV  (double)(pow(2.0, 25.0))
#define FREQ_MUL  (double)(XTAL_FREQ / FREQ_DIV)

#define PACKET_TYPE_GFSK 0x00
#define PACKET_TYPE_LORA 0x01

#define MOD_SF5  0x05
#define MOD_SF6  0x06
#define MOD_SF7  0x07
#define MOD_SF8  0x08
#define MOD_SF9  0x09
#define MOD_SF10 0x0A
#define MOD_SF11 0x0B
#define MOD_SF12 0x0C

#define MOD_BW_7   0x00
#define MOD_BW_10  0x08
#define MOD_BW_15  0x01
#define MOD_BW_20  0x09
#define MOD_BW_31  0x02
#define MOD_BW_41  0x0A
#define MOD_BW_62  0x03
#define MOD_BW_125 0x04
#define MOD_BW_250 0x05
#define MOD_BW_500 0x06

#define MOD_CR_4_5 0x01
#define MOD_CR_4_6 0x02
#define MOD_CR_4_7 0x03
#define MOD_CR_4_8 0x04

#define TAG "LoRa"

void read_spi(const lora_handle* handle, uint8_t* tx, uint8_t* rx, uint8_t buf_size)
{
    ESP_LOGV(TAG, "READ_SPI");
    spi_transaction_t t = {
        .length    = 8 * buf_size,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    spi_device_transmit(handle->spi, &t);
}

void write_spi(const lora_handle* handle, uint8_t* tx, uint8_t buf_size)
{
    ESP_LOGV(TAG, "WRITE_SPI");
    spi_transaction_t t = {
        .length    = 8 * buf_size,
        .tx_buffer = tx,
        .rx_buffer = NULL,
    };

    spi_device_transmit(handle->spi, &t);
}

void read_register(const lora_handle* handle, uint16_t addr, uint8_t* buf, uint8_t buf_size)
{
    ESP_LOGV(TAG, "READ_REG: 0x%04X", addr);
    uint8_t data[16];
    memset(data, 0, sizeof(data));
    data[0] = CMD_READ_REGISTER;
    data[1] = (addr >> 8) & 0xFF;
    data[2] = addr & 0xFF;
    read_spi(handle, data, data, buf_size + 4);

    memcpy(buf, data + 4, buf_size);
}

uint8_t read_buffer(const lora_handle* handle, uint8_t* rx_buf, uint8_t len)
{
    ESP_LOGV(TAG, "READ_BUFFER");
    uint8_t payload_length;
    uint8_t payload_offset;
    lora_get_rx_buffer_status(handle, &payload_length, &payload_offset);

    ESP_LOGI(TAG, "Payload Length: %d, Payload Offset: %d\n", payload_length, payload_offset);

    if (payload_length > len) {
        ESP_LOGW(TAG, "ReadBuffer too small. Payload:%d buf:%d", payload_length, len);
        return 0;
    }

    uint8_t* buf = malloc(payload_length + 3);
    if (!buf) {
        ESP_LOGE(TAG, "ReadBuffer malloc fail");
        return 0;
    }

    buf[0] = CMD_READ_BUFFER;
    buf[1] = payload_offset;
    buf[2] = 0x0;
    memset(&buf[3], 0x0, payload_length);
    read_spi(handle, buf, buf, payload_length + 3);
    memcpy(rx_buf, &buf[3], payload_length);
    free(buf);

    return payload_length;
}

void write_register(const lora_handle* handle, uint16_t addr, uint8_t* buf, uint8_t buf_size)
{
    ESP_LOGV(TAG, "WRITE_REG: 0x%04X", addr);
    uint8_t data[16];

    memset(data, 0, sizeof(data));
    data[0] = CMD_WRITE_REGISTER;
    data[1] = (addr >> 8) & 0xFF;
    data[2] = addr & 0xFF;
    memcpy(data + 3, buf, buf_size);
    write_spi(handle, data, buf_size + 3);
}

int lora_init(const lora_config config, lora_handle* handle)
{
    ESP_LOGI("LORA", "MISO_GPIO=%d", config.miso);
    ESP_LOGI("LORA", "MOSI_GPIO=%d", config.mosi);
    ESP_LOGI("LORA", "SCK_GPIO=%d", config.sck);
    ESP_LOGI("LORA", "CS_GPIO=%d", config.cs);
    ESP_LOGI("LORA", "RST_GPIO=%d", config.rst);
    ESP_LOGI("LORA", "BUSY_GPIO=%d", config.busy);
    ESP_LOGV(TAG, "INIT");

    gpio_reset_pin(config.rst);
    gpio_set_direction(config.rst, GPIO_MODE_OUTPUT);

    gpio_reset_pin(config.cs);
    gpio_set_direction(config.cs, GPIO_MODE_OUTPUT);
    gpio_set_level(config.cs, 1);

    gpio_reset_pin(config.busy);
    gpio_set_direction(config.busy, GPIO_MODE_INPUT);

    spi_bus_config_t bus = {
        .miso_io_num   = config.miso,
        .mosi_io_num   = config.mosi,
        .sclk_io_num   = config.sck,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_DISABLED));

    spi_device_interface_config_t dev = {
        .clock_speed_hz = 1E5,
        .mode           = 0,
        .spics_io_num   = config.cs,
        .queue_size     = 2,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev, &handle->spi));

    handle->rst = config.rst;

    lora_reset(handle);

    uint8_t buf[2] = { 0x0, 0x0 };
    read_register(handle, REG_LORA_SYNC_WORD_MSB, buf, 2);
    uint16_t sync_word = (buf[0] << 8) | buf[1];

    if (sync_word != SYNC_WORD_PUBLIC && sync_word != SYNC_WORD_PRIVATE) {
        ESP_LOGE(TAG, "Possibly no SPI Connection. Sync: 0x%04X. Expected 0x%04X or 0x%04X",
            sync_word, SYNC_WORD_PUBLIC, SYNC_WORD_PRIVATE);
        return 1;
    }
    ESP_LOGI(TAG, "Sync Word: 0x%04X", sync_word);

    lora_standby(handle);

    lora_set_packet_type(handle, PACKET_TYPE_LORA);
    lora_set_frequency(handle, config.freq);
    lora_set_buffer_base(handle, 0x0, 0x0);
    lora_set_mod_params(handle, MOD_SF11, MOD_BW_125, MOD_CR_4_5);
    lora_set_packet_params(handle, 8, HEADER_TYPE_EXP, 0xFF, CRC_ON, IQ_STANDARD);
    lora_set_dio_irq_params(handle, IRQ_ALL);

    lora_receive(handle);

    return 1;
}

void lora_reset(const lora_handle* handle)
{
    ESP_LOGV(TAG, "RESET");
    gpio_set_level(handle->rst, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(handle->rst, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void lora_set_packet_type(const lora_handle* handle, uint8_t packet_type)
{
    ESP_LOGV(TAG, "SET_PACKET_TYPE: %d", packet_type);
    uint8_t data[2] = { CMD_SET_PACKET_TYPE, packet_type };
    write_spi(handle, data, 2);
}

uint8_t lora_get_packet_type(const lora_handle* handle)
{
    ESP_LOGV(TAG, "GET_PACKET_TYPE");
    uint8_t data[3] = { CMD_GET_PACKET_TYPE, 0x0, 0x0 };
    read_spi(handle, data, data, 3);
    return data[2];
}

void lora_set_frequency(lora_handle* handle, uint32_t freq)
{
    ESP_LOGV(TAG, "SET_FREQ: %ld", freq);
    handle->freq = freq;

    uint8_t buf[5] = { CMD_SET_RF_FREQUENCY, 0x0, 0x0, 0x0, 0x0 };

    uint32_t rf = (uint32_t)(freq / FREQ_MUL);
    buf[1]      = (rf >> 24) & 0xFF;
    buf[2]      = (rf >> 16) & 0xFF;
    buf[3]      = (rf >> 8) & 0xFF;
    buf[4]      = (rf >> 0) & 0xFF;

    write_spi(handle, buf, 5);
}

void lora_set_buffer_base(const lora_handle* handle, uint8_t tx_base, uint8_t rx_base)
{
    ESP_LOGV(TAG, "SET_BUFFER_BASE: TX: %d, RX: %d", tx_base, rx_base);
    uint8_t buf[3] = { CMD_SET_BUFFER_BASE_ADDRESS, tx_base, rx_base };

    write_spi(handle, buf, 3);
}

void lora_get_rx_buffer_status(
    const lora_handle* handle, uint8_t* payload_length, uint8_t* rx_start_buffer)
{
    ESP_LOGV(TAG, "GET_BUFFER_STATUS");
    uint8_t buf[4] = { CMD_GET_RX_BUFFER_STATUS, 0x0, 0x0, 0x0 };
    read_spi(handle, buf, buf, 4);
    *payload_length  = buf[2];
    *rx_start_buffer = buf[3];
}

void lora_standby(const lora_handle* handle)
{
    ESP_LOGV(TAG, "STANDBY");
    uint8_t buf[2] = { CMD_SET_STANDBY, 0x0 };
    write_spi(handle, buf, 2);
}

void lora_receive(const lora_handle* handle)
{
    ESP_LOGV(TAG, "RECEIVE");
    uint8_t buf[4] = { CMD_SET_RX, 0xFF, 0xFF, 0xFF };
    write_spi(handle, buf, 4);
}

uint8_t lora_received_packet(const lora_handle* handle)
{
    ESP_LOGV(TAG, "RECEIVED_PACKET");
    uint16_t irq = lora_get_irq_status(handle);
    return (irq & IRQ_RX_DONE);
}

uint8_t lora_read_packet(const lora_handle* handle, uint8_t* data, uint8_t len)
{
    ESP_LOGI(TAG, "READ_PACKET");
    uint16_t irq = lora_get_irq_status(handle);
    if (irq & IRQ_RX_DONE) {
        lora_clear_irq_status(handle, IRQ_ALL);
        uint8_t read_length = read_buffer(handle, data, len);

        return read_length;
    }

    return 0;
}

uint16_t lora_get_irq_status(const lora_handle* handle)
{
    // ESP_LOGV(TAG, "GET_IRQ_STATUS");
    uint8_t buf[4] = { CMD_GET_IRQ_STATUS, 0x0, 0x0, 0x0 };
    read_spi(handle, buf, buf, 4);

    return (((uint16_t)buf[2]) << 8) | buf[3];
}

void lora_clear_irq_status(const lora_handle* handle, uint16_t irq)
{
    ESP_LOGV(TAG, "CLEAR_IRQ_STATUS: 0x%X", irq);
    uint8_t buf[3] = { CMD_CLEAR_IRQ_STATUS, (irq >> 8) & 0xFF, irq & 0xFF };
    write_spi(handle, buf, 3);
}

void lora_set_packet_params(const lora_handle* handle, uint16_t preamble_length,
    uint8_t header_type, uint8_t payload_length, uint8_t crc_type, int8_t invert_iq)
{
    ESP_LOGV(TAG,
        "SET_PACKET_PARAMS: PREAMBLE_LENGTH:%d HEADER_TYPE: %d PAYLOAD_LENGTH %d CRC_TYPE: %d "
        "INVERT_IQ:%d",
        preamble_length, header_type, payload_length, crc_type, invert_iq);

    uint8_t data[10] = { CMD_SET_PACKET_PARAMS, (preamble_length >> 8) & 0xFF,
        preamble_length & 0xFF, header_type, payload_length, crc_type, invert_iq, 0x0, 0x0, 0x0 };
    write_spi(handle, data, 10);
}

void lora_get_packet_status(
    const lora_handle* handle, int8_t* rssi, int8_t* snr, int8_t* signal_rssi)
{
    uint8_t data[5] = { CMD_GET_PACKET_STATUS, 0x0, 0x0, 0x0, 0x0 };
    read_spi(handle, data, data, 5);

    if (rssi != NULL)
        *rssi = -((int8_t)data[2]) / 2;

    if (snr != NULL)
        *snr = ((int8_t)data[3]) / 4;

    if (signal_rssi != NULL)
        *signal_rssi = -data[4] / 2;
}

int8_t lora_get_packet_rssi(const lora_handle* handle)
{
    int8_t rssi;
    lora_get_packet_status(handle, &rssi, NULL, NULL);
    return rssi;
}

int8_t lora_get_rssi_inst(const lora_handle* handle)
{
    uint8_t data[3] = { CMD_GET_RSSI_INST, 0x0, 0x0 };
    read_spi(handle, data, data, 3);

    return -((int8_t)data[2]) / 2;
}

void lora_set_sync_word(const lora_handle* handle, uint16_t sync_word)
{
    uint8_t buf[1] = { (sync_word >> 8) & 0xFF };
    write_register(handle, REG_LORA_SYNC_WORD_MSB, buf, 1);
    buf[0] = sync_word & 0xFF;
    write_register(handle, REG_LORA_SYNC_WORD_LSB, buf, 1);
    ESP_LOGI(TAG, "Sync Word: %04X", sync_word);
}

void lora_set_byte_sync_word(const lora_handle* handle, uint8_t sync_word)
{
    // Set Sync word to 0xY4Z4 where sync_word has the form 0xYZ
    lora_set_sync_word(
        handle, ((uint16_t)(sync_word & 0xF0) << 8) | ((sync_word & 0x0F) << 4) | 0x0404);
}

uint16_t lora_get_sync_word(const lora_handle* handle)
{
    uint8_t buf[1];
    uint16_t sync_word = 0;
    read_register(handle, REG_LORA_SYNC_WORD_MSB, buf, 1);
    sync_word = buf[0] << 8;
    read_register(handle, REG_LORA_SYNC_WORD_LSB, buf, 1);
    sync_word |= buf[0];

    return sync_word;
}

void lora_set_mod_params(const lora_handle* handle, uint8_t sf, uint8_t bw, uint8_t cr)
{
    ESP_LOGV(TAG, "SET_MOD_PARAMS: SF: %d BW: %d CR %d", sf, bw, cr);

    uint8_t data[9] = { CMD_SET_MODULATION_PARAMS, sf, bw, cr, 0, 0, 0, 0, 0 };
    write_spi(handle, data, 9);
}

void lora_set_dio_irq_params(const lora_handle* handle, uint16_t irq_mask)
{
    ESP_LOGV(TAG, "SET_DIO_IRQ: IRQ_MASK: %x", irq_mask);

    uint8_t data[9]
        = { CMD_DIO_IRQ_PARAMS, (irq_mask >> 8) & 0xFF, (irq_mask) & 0xFF, 0, 0, 0, 0, 0, 0 };
    write_spi(handle, data, 9);
}
