#ifndef DAC121S101_H
#define DAC121S101_H

#include "hardware/spi.h"

/** 
 * \file dac121s101.h
 * \defgroup peri_dac_dac121s101 peri_dac_dac121s101
 * \brief Peripheral driver for the DAC121S101 TI SSI DAC
 * 
 * The DAC121S101 is a 12-bit 1 MSPS DAC with Texas Instruments Synchronous Serial Interface.
 * 
 * The interface of the DAC is compatible with SPI with the difference of not having a chip select line, but 
 * a SYNC line instead. The SYNC line is pulsed high before data is sent and it's used to enable the internal shift register.
 * 
 * The DAC has 3 power-down modes: power-down with 1 kOhm to GND, power-down with 100 kOhm to GND, power-down with Hi-Z. 
 * Power-down has no effect on the contents of the DAC register, so when coming out of power-down the DAC outputs voltage it 
 * was programmed to before entering power-down.
*/

/** 
 * \brief Number of DAC bits
 * \ingroup peri_dac_dac121s101
 * 
 * This define is useful for computing the register value needed for a given voltage
*/
#define DAC121S101_RESOLUTION 12

/** 
 * \brief Power-down bits bitmask
 * \ingroup peri_dac_dac121s101
*/
#define DAC121S101_PD_BITS 0x3000

/** 
 * \brief How many bits to left shift power-down state bits
 * \ingroup peri_dac_dac121s101
*/
#define DAC121S101_PD_SHIFT 12

/** 
 * \brief Data bits bitmask
 * \ingroup peri_dac_dac121s101
*/
#define DAC121S101_DATA_BITS 0xFFF

/**
 * \brief Power-down state enum
 * \ingroup peri_dac_dac121s101
 */
typedef enum
{
    DAC121S101_PD_NORM = 0x0,
    DAC121S101_PD_PULLDOWN_1K = 0x1,
    DAC121S101_PD_PULLDOWN_100k = 0x2,
    DAC121S101_PD_HIZ = 0x3
} dac121s101_pd_states_t;

/** 
 * \brief DAC121S101 instance struct
 * \ingroup peri_dac_dac121s101
 * 
 * This struct holds a pointer to the connected SPI instance and also holds copy of power-down state and data for 
 * access acceleration.
*/
typedef struct
{
    spi_inst_t *hw_spi_inst;
    dac121s101_pd_states_t pd_state_copy;
    uint16_t data_copy;
} dac121s101_inst_t;

/** 
 * \brief Assemble data frame to be sent to the DAC
 * \ingroup peri_dac_dac121s101
 * 
 * \param pd_state Power-down state to be set
 * \param data DAC register value to be sent
 * \return Resultant 16-bit frame
*/
uint16_t dac121s101_assemble_frame(dac121s101_pd_states_t pd_state, uint16_t data);

/** 
 * \brief Initialise the DAC connected to a SPI HW block
 * \ingroup peri_dac_dac121s101
 * 
 * Initializes the DAC instance struct, configures pins and the SPI HW block and presets the DAC to normal operation 
 * and output of 0.
 * 
 * \param inst DAC instance struct
 * \param hw_spi_inst SPI instance
 * \param sclk_pin SCLK pin number
 * \param mosi_pin MOSI pin number
 * \param sync_pin SYNC pin number
*/
void dac121s101_init_hw_spi(dac121s101_inst_t *inst, spi_inst_t *hw_spi_inst, uint sclk_pin, uint mosi_pin, uint sync_pin);

/** 
 * \brief Write data and power-down state
 * \ingroup peri_dac_dac121s101
 * 
 * \param inst DAC instance struct
 * \param data DAC register value to be sent
 * \param pd_state DAC power-down state to be set
*/
void dac121s101_write_all(dac121s101_inst_t *inst, uint16_t data, dac121s101_pd_states_t pd_state);

/** 
 * \brief Write only data to the DAC register and leave power-down state unchanged
 * \ingroup peri_dac_dac121s101
 * 
 * \param inst DAC instance struct
 * \param data DAC register value to be sent
*/
void dac121s101_write_data(dac121s101_inst_t *inst, uint16_t data);

/** 
 * \brief Set only power-down state and leave DAC register data unchanged
 * \ingroup peri_dac_dac121s101
 * 
 * \param inst DAC instance struct
 * \param pd_state Power-down state to be set
*/
void dac121s101_change_pd_state(dac121s101_inst_t *inst, dac121s101_pd_states_t pd_state);

#endif