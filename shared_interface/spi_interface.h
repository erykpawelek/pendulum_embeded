/**
 * @file spi_protocol.h
 * @brief Communication protocol definition for Raspberry Pi 5 to ESP32 SPI bus.
 * @details This header defines the fixed-size binary frames used for full-duplex
 *          real-time control and state feedback in the rotary inverted pendulum system.
 *          Both frames are packed and padded to 16 bytes for 32-bit DMA alignment.
 */

#ifndef SPI_INTERFACE
#define SPI_INTERFACE

#include <stdint.h>
#include <stddef.h>

//Disable name mangling
#ifdef __cplusplus
extern "C" {
#endif

/** @brief Magic synchronization byte to verify frame boundary alignment. */
#define SPI_SYNC_BYTE              0xA5

/** @brief Fixed size of each SPI frame in bytes (DMA-friendly 16-byte alignment). */
#define SPI_FRAME_SIZE             16

/* ========================================================================= */
/* Command Flags (Master -> Slave)                                           */
/* ========================================================================= */

/** @brief Bitmask to enable motor stepper output driver. */
#define CMD_FLAG_ENABLE_MOTOR      (1 << 0)

/** @brief Bitmask to trigger immediate emergency deceleration and disable steps. */
#define CMD_FLAG_EMERGENCY_STOP    (1 << 1)

/** @brief Bitmask to request zero-position calibration for encoder/stepper. */
#define CMD_FLAG_RESET_ZERO        (1 << 2)

/* ========================================================================= */
/* Status Flags (Slave -> Master)                                            */
/* ========================================================================= */

/** @brief Bitmask indicating stepper driver is powered and accepting steps. */
#define STATUS_FLAG_MOTOR_ENABLED  (1 << 0)

/** @brief Bitmask indicating encoder signal is valid and tracking properly. */
#define STATUS_FLAG_ENCODER_READY  (1 << 1)

/** @brief Bitmask indicating a critical hardware or communications failure. */
#define STATUS_FLAG_FAULT          (1 << 2)

/* ========================================================================= */
/* Frame Structures                                                          */
/* ========================================================================= */

/**
 * @brief Control frame sent from Raspberry Pi 5 (Master) to ESP32 (Slave).
 * @note Fixed size: 16 bytes.
 */
#pragma pack(push, 1) // Synthetic padding disabled (by default compilators depending of architecture of cpu 32-64 add padding bytes)
typedef struct {
    /** @brief Frame synchronization delimiter, must be @ref SPI_SYNC_BYTE. */
    uint8_t  sync_byte;

    /** @brief Bitmask of operational commands (@ref CMD_FLAG_ENABLE_MOTOR, etc.). */
    uint8_t  command_flags;

    /** @brief Target angular velocity for the arm stepper motor in radians/sec. */
    float    target_velocity;

    /** @brief Monotonically increasing sequence ID for round-trip latency tracking. */
    uint16_t sequence_id;

    /** @brief Reserved padding bytes for future expansion and 16-byte alignment. */
    uint8_t  reserved[6];

    /** @brief CRC-16-CCITT checksum calculated over the preceding 14 bytes. */
    uint16_t checksum;
} ControlFrame;
#pragma pack(pop)

/**
 * @brief State feedback frame sent from ESP32 (Slave) to Raspberry Pi 5 (Master).
 * @note Fixed size: 16 bytes.
 */
#pragma pack(push, 1)
typedef struct {
    /** @brief Frame synchronization delimiter, must be @ref SPI_SYNC_BYTE. */
    uint8_t  sync_byte;

    /** @brief Hardware status bitmask (@ref STATUS_FLAG_MOTOR_ENABLED, etc.). */
    uint8_t  status_flags;

    /** @brief Measured pendulum deflection angle from upright vertical position in radians. */
    float    pendulum_angle;

    /** @brief Current estimated or accumulated stepper position in radians. */
    float    motor_position;

    /** @brief Echoed sequence ID from the corresponding ControlFrame. */
    uint16_t sequence_id;

    /** @brief Reserved padding bytes for future sensor telemetry. */
    uint8_t  reserved[2];

    /** @brief CRC-16-CCITT checksum calculated over the preceding 14 bytes. */
    uint16_t checksum;
} FeedbackFrame;
#pragma pack(pop)

/* Compile-time verification of memory layout */
_Static_assert(sizeof(ControlFrame) == SPI_FRAME_SIZE, "ControlFrame must be exactly 16 bytes");
_Static_assert(sizeof(FeedbackFrame) == SPI_FRAME_SIZE, "FeedbackFrame must be exactly 16 bytes");

/* ========================================================================= */
/* Utility Functions                                                         */
/* ========================================================================= */

/**
 * @brief Computes a standard CRC-16-CCITT checksum.
 * @param[in] data Pointer to the buffer over which the checksum is computed.
 * @param[in] length Number of bytes to process (typically @ref SPI_FRAME_SIZE - 2).
 * @return Computed 16-bit CRC value.
 */
static inline uint16_t compute_crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

#ifdef __cplusplus
}
#endif

#endif /* SPI_PROTOCOL_H_ */