#ifndef DAC081C08X_H
#define DAC081C08X_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include <inttypes.h>

/** 
 * \file dac081s08x.h
 * \defgroup peri_dac_dac081s08x peri_dac_dac081s08x
 * \brief Peripheral driver for the DAC081C08X TI I2C DAC
 * 
 * The DAC081S081 and DAC081S085 are 8-bit DACs with I2C interface.
 * 
 * The DAC has 3 power-down modes: power-down with 1 kOhm to GND, power-down with 100 kOhm to GND, power-down with Hi-Z. 
 * Power-down has no effect on the contents of the DAC register, so when coming out of power-down the DAC outputs voltage it 
 * was programmed to before entering power-down.
 * 
 * The data is sent to the DAC in two bytes. The first contains the power down values and the second contains the DAC 
 * register data.
*/

/** 
 * \brief Number of DAC bits
 * \ingroup peri_dac_dac081c08x
 * 
 * This define is useful for computing the register value needed for a given voltage
*/
#define DAC081C08X_RESOLUTION 8

/** 
 * \brief Power-down bits bitmask in the upper byte
 * \ingroup peri_dac_dac081c08x
*/
#define DAC081C08X_PD_BITS 0x30

/** 
 * \brief How many bits to left shift power-down state bits in the upper byte
 * \ingroup peri_dac_dac081c08x
*/
#define DAC081C08X_PD_SHIFT 4

/** 
 * \brief Data bits bitmask
 * \ingroup peri_dac_dac081c08x
*/
#define DAC081C08X_DATA_BITS 0xFF

/**
 * \brief Power-down state enum
 * \ingroup peri_dac_dac081c08x
*/
typedef enum
{
    DAC081C08X_PD_NORM = 0x0,
    DAC081C08X_PD_PULLDOWN_2K5 = 0x1,
    DAC081C08X_PD_PULLDOWN_100k = 0x2,
    DAC081C08X_PD_HIZ = 0x3
} dac081c08x_pd_states_t;

/**
 * \brief I2C address enum
 * \ingroup peri_dac_dac081c08x
 * 
 * This enum holds all the DAC I2C addresses, including the broadcast address.
 * 
 * \note Warning! Using the broadcast address will break all other instances on the same bus! Use it only with a single global instance for a bus!
*/
typedef enum
{
    DAC081C08X_ADDR0 = 0b0001100,
    DAC081C08X_ADDR1 = 0b0001101,
    DAC081C08X_ADDR2 = 0b0001110,
    DAC081C08X_ADDR3 = 0b0001000,
    DAC081C08X_ADDR4 = 0b0001001,
    DAC081C08X_ADDR5 = 0b0001010,
    DAC081C08X_ADDR6 = 0b1001100,
    DAC081C08X_ADDR7 = 0b1001101,
    DAC081C08X_ADDR8 = 0b1001110,
    DAC081C08X_BROADCAST_ADDR = 0b1001000
} dac081c08x_addresses_t;

/** 
 * \brief DAC081C08X instance struct
 * \ingroup peri_dac_dac081c08x
 * 
 * This struct holds a pointer to the connected I2C instance, I2C address and also copy of power-down state and data for 
 * access acceleration.
*/
typedef struct
{
    i2c_inst_t *i2c;
    uint8_t addr;
    dac081c08x_pd_states_t pd_state_copy;
    uint8_t data_copy;
} dac081c08x_inst_t;

/** 
 * \brief Assemble data frame to be sent to the DAC
 * \ingroup peri_dac_dac081c08x
 * 
 * \param frame 2 x uint8_t array in which the resultant data frame is stored
 * \param pd_state Power-down state to be set
 * \param data DAC register value to be sent
 * \return Resultant 2x8-bit array frame
*/
void dac081c08x_assemble_frame(uint8_t *frame, dac081c08x_pd_states_t pd_state, uint8_t data);

/** 
 * \brief Initialise the DAC connected to a SPI HW block
 * \ingroup peri_dac_dac081c08x
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
void dac081c08x_init(dac081c08x_inst_t *inst, i2c_inst_t *i2c, uint8_t addr);

/** 
 * \brief Write data and power-down state
 * \ingroup peri_dac_dac081c08x
 * 
 * \param inst DAC instance struct
 * \param data DAC register value to be sent
 * \param pd_state DAC power-down state to be set
*/
void dac081c08x_write_all(dac081c08x_inst_t *inst, uint8_t data, dac081c08x_pd_states_t pd_state);

/** 
 * \brief Write only data to the DAC register and leave power-down state unchanged
 * \ingroup peri_dac_dac081c08x
 * 
 * \param inst DAC instance struct
 * \param data DAC register value to be sent
*/
void dac081c08x_write_data(dac081c08x_inst_t *inst, uint8_t data);

/** 
 * \brief Set only power-down state and leave DAC register data unchanged
 * \ingroup peri_dac_dac081c08x
 * 
 * \param inst DAC instance struct
 * \param pd_state Power-down state to be set
*/
void dac081c08x_change_pd_state(dac081c08x_inst_t *inst, dac081c08x_pd_states_t pd_state);

#endif