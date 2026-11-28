/**
 * @file spi_interface.h
 * @brief Communication protocol definition for Raspberry Pi 5 to STM32 SPI bus.
 * @details This header defines the fixed-size binary frames used for full-duplex
 *          real-time control and state feedback in the rotary inverted pendulum system.
 *          Both frames are packed and padded to 24 bytes (multiple of 4 for 32-bit DMA
 *          alignment). The Raspberry Pi is the SPI master, the STM32 is the slave.
 *
 *          Angle convention (matches the MuJoCo simulation used for training):
 *            - pendulum_angle: 0 = hanging straight down, +/-pi = upright.
 *            - motor_position: arm angle, multi-turn (NOT wrapped to +/-pi).
 *          Both are measured by AS5600 magnetic encoders read by the STM32 over I2C.
 */

#ifndef SPI_INTERFACE_H
#define SPI_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

// Disable C++ name mangling
#ifdef __cplusplus
extern "C" {
#endif

/** @brief Magic synchronization byte to verify frame boundary alignment. */
#define SPI_SYNC_BYTE              0xA5

/** @brief Fixed size of each SPI frame in bytes (24 = multiple of 4 for DMA alignment). */
#define SPI_FRAME_SIZE             24

/**
 * @brief Slave-side watchdog timeout in milliseconds.
 * @details If the STM32 does not receive a valid ControlFrame (sync byte, CRC and sequence
 *          OK) within this time, it must on its own decelerate and disable the motor, set
 *          @ref STATUS_FLAG_WATCHDOG_TRIPPED and ignore ENABLE until the master sends a
 *          valid frame with @ref CMD_FLAG_EMERGENCY_STOP cleared. The control period is
 *          20 ms (50 Hz), so this tolerates a few missed frames only.
 */
#define SPI_WATCHDOG_TIMEOUT_MS    100

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
#define STATUS_FLAG_MOTOR_ENABLED     (1 << 0)

/** @brief Bitmask indicating both encoder signals are valid and tracking properly. */
#define STATUS_FLAG_ENCODER_READY     (1 << 1)

/** @brief Bitmask indicating a critical hardware or communications failure. */
#define STATUS_FLAG_FAULT             (1 << 2)

/** @brief Bitmask indicating the slave watchdog expired and the motor was stopped. */
#define STATUS_FLAG_WATCHDOG_TRIPPED  (1 << 3)

/* ========================================================================= */
/* Frame Structures                                                          */
/* ========================================================================= */

/**
 * @brief Control frame sent from Raspberry Pi 5 (Master) to STM32 (Slave).
 * @note Fixed size: 24 bytes. All multi-byte fields are little-endian (native on both MCUs).
 */
#pragma pack(push, 1) // Disable compiler padding; the layout below is the wire format
typedef struct {
    /** @brief Frame synchronization delimiter, must be @ref SPI_SYNC_BYTE. */
    uint8_t  sync_byte;

    /** @brief Bitmask of operational commands (@ref CMD_FLAG_ENABLE_MOTOR, etc.). */
    uint8_t  command_flags;

    /** @brief Monotonically increasing sequence ID for round-trip latency tracking. */
    uint16_t sequence_id;

    /** @brief Target angular velocity for the arm stepper motor in radians/sec. */
    float    target_velocity;

    /** @brief Reserved padding bytes for future expansion and 24-byte alignment. */
    uint8_t  reserved[14];

    /** @brief CRC-16-CCITT checksum calculated over the preceding 22 bytes. */
    uint16_t checksum;
} ControlFrame;
#pragma pack(pop)

/**
 * @brief State feedback frame sent from STM32 (Slave) to Raspberry Pi 5 (Master).
 * @note Fixed size: 24 bytes. All multi-byte fields are little-endian.
 * @note SPI is full-duplex: this frame is clocked out while the ControlFrame is clocked in,
 *       so it is prepared before that ControlFrame arrives.
 */
#pragma pack(push, 1)
typedef struct {
    /** @brief Frame synchronization delimiter, must be @ref SPI_SYNC_BYTE. */
    uint8_t  sync_byte;

    /** @brief Hardware status bitmask (@ref STATUS_FLAG_MOTOR_ENABLED, etc.). */
    uint8_t  status_flags;

    /** @brief Sequence ID of the last valid ControlFrame the slave had received when this
     *         frame was prepared (i.e. usually the previous exchange, not the current one). */
    uint16_t sequence_id;

    /** @brief Measured pendulum angle in radians (0 = hanging down, +/-pi = upright). */
    float    pendulum_angle;

    /** @brief Measured arm (motor shaft) angle in radians, accumulated over multiple turns. */
    float    motor_position;

    /** @brief Pendulum angular velocity in rad/s, computed on the STM32 from the encoder. */
    float    pendulum_velocity;

    /** @brief Arm (motor shaft) angular velocity in rad/s, computed on the STM32 from the encoder. */
    float    motor_velocity;

    /** @brief Reserved padding bytes for future sensor telemetry. */
    uint8_t  reserved[2];

    /** @brief CRC-16-CCITT checksum calculated over the preceding 22 bytes. */
    uint16_t checksum;
} FeedbackFrame;
#pragma pack(pop)

/* Compile-time layout verification (static_assert is the C++ spelling of _Static_assert) */
#ifdef __cplusplus
static_assert(sizeof(ControlFrame) == SPI_FRAME_SIZE, "ControlFrame must be exactly 24 bytes");
static_assert(sizeof(FeedbackFrame) == SPI_FRAME_SIZE, "FeedbackFrame must be exactly 24 bytes");
#else
_Static_assert(sizeof(ControlFrame) == SPI_FRAME_SIZE, "ControlFrame must be exactly 24 bytes");
_Static_assert(sizeof(FeedbackFrame) == SPI_FRAME_SIZE, "FeedbackFrame must be exactly 24 bytes");
#endif

/* ========================================================================= */
/* Utility Functions                                                         */
/* ========================================================================= */

/**
 * @brief Computes a standard CRC-16-CCITT (CCITT-FALSE) checksum.
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
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

/**
 * @brief Fills the sync byte and CRC of a ControlFrame that has its payload set.
 * @param[in,out] frame Frame whose other fields are already filled.
 */
static inline void control_frame_seal(ControlFrame *frame) {
    frame->sync_byte = SPI_SYNC_BYTE;
    frame->checksum = compute_crc16((const uint8_t *)frame, SPI_FRAME_SIZE - 2);
}

/**
 * @brief Builds a sealed "stop the motor now" ControlFrame (zero velocity, E-stop, no enable).
 * @param[out] frame Frame to initialise (fully overwritten, reserved bytes zeroed).
 * @param[in] sequence_id Sequence ID to stamp into the frame.
 */
static inline void control_frame_make_stop(ControlFrame *frame, uint16_t sequence_id) {
    uint8_t *raw = (uint8_t *)frame;
    for (size_t i = 0; i < SPI_FRAME_SIZE; i++) {
        raw[i] = 0;
    }
    frame->command_flags = CMD_FLAG_EMERGENCY_STOP;
    frame->sequence_id = sequence_id;
    frame->target_velocity = 0.0f;
    control_frame_seal(frame);
}

/**
 * @brief Checks sync byte and CRC of a received FeedbackFrame.
 * @param[in] frame Received frame.
 * @return 1 if the frame is valid, 0 otherwise.
 */
static inline int feedback_frame_is_valid(const FeedbackFrame *frame) {
    return frame->sync_byte == SPI_SYNC_BYTE &&
           frame->checksum == compute_crc16((const uint8_t *)frame, SPI_FRAME_SIZE - 2);
}

#ifdef __cplusplus
}
#endif

#endif /* SPI_INTERFACE_H */
