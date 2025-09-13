#pragma once

#define LORA_IRQ_ALL               0b1111111111
#define LORA_IRQ_TX_DONE           1 << 0
#define LORA_IRQ_RX_DONE           1 << 1
#define LORA_IRQ_PREAMBLE_DETECTED 1 << 2
#define LORA_IRQ_SYNC_WORD_VALID   1 << 3
#define LORA_IRQ_HEADER_VALId      1 << 4
#define LORA_IRQ_HEADER_ERR        1 << 5
#define LORA_IRQ_CRC_ERR           1 << 6
#define LORA_IRQ_CAD_DONE          1 << 7
#define LORA_IRQ_CAD_DETECTED      1 << 8
#define LORA_IRQ_TIMEOUT           1 << 9

#define LORA_HEADER_TYPE_EXP 0x0
#define LORA_HEADER_TYPE_IMP 0x1
#define LORA_CRC_OFF         0x0
#define LORA_CRC_ON          0x1
#define LORA_IQ_STANDARD     0x0
#define LORA_IQ_INVERT       0x1

#define LORA_SF5  0x05
#define LORA_SF6  0x06
#define LORA_SF7  0x07
#define LORA_SF8  0x08
#define LORA_SF9  0x09
#define LORA_SF10 0x0A
#define LORA_SF11 0x0B
#define LORA_SF12 0x0C

#define LORA_BW_7   0x00
#define LORA_BW_10  0x08
#define LORA_BW_15  0x01
#define LORA_BW_20  0x09
#define LORA_BW_31  0x02
#define LORA_BW_41  0x0A
#define LORA_BW_62  0x03
#define LORA_BW_125 0x04
#define LORA_BW_250 0x05
#define LORA_BW_500 0x06

#define LORA_CR_4_5 0x01
#define LORA_CR_4_6 0x02
#define LORA_CR_4_7 0x03
#define LORA_CR_4_8 0x04
