#ifndef PI4IOE5V6408_H
#define PI4IOE5V6408_H

#include <inttypes.h>
#include <stdbool.h>

#include "hardware/i2c.h"

/** 
 * \file pi4ioe5v6408.h
 * \defgroup peri_i2c_pi4ioe5v6408 peri_i2c_pi4ioe5v6408
 * \brief Peripheral driver for the PI4IOE5V6408 I2C IO expander
 * 
 * The PI4IOE5V6408 is an I2C IO expander with 8 IO pins, nALERT IRQ pin, address pin and a separate logic and IO supply.
 * 
 * This driver accelerates access to the internal registers of the IO expander by locally caching them.
*/

/** 
 * \addtogroup peri_i2c_pi4ioe5v6408
 * @{
*/
#define PI4IO_I2C_ADDR0 0b1000011 ///< I2C address with ADDR pin low
#define PI4IO_I2C_ADDR1 0b1000100 ///< I2C address with ADDR pin high
/**@} */

//Registers
#define PI4IO_REG_DEVICE_ID_AND_CONTROL 0x1
#define PI4IO_REG_IO_DIRECTION 0x3
#define PI4IO_REG_OUTPUT_STATE 0x5
#define PI4IO_REG_OUTPUT_HIZ 0x7
#define PI4IO_REG_INPUT_DEFAULT_STATE 0x9
#define PI4IO_REG_PULL_UP_DOWN_EN 0xB
#define PI4IO_REG_PULL_UP_DOWN_SEL 0xD
#define PI4IO_REG_INPUT_STATUS 0xF
#define PI4IO_REG_INTERRUPT_MASK 0x11
#define PI4IO_REG_INTERRUPT_STATUS 0x13

//Device ID and Control Register bit masks
#define PI4IO_MANUFACTURE_ID_BITS 0xE0
#define PI4IO_FIRMWARE_REVISION_BITS 0x1C
#define PI4IO_RESET_INTERRUPT_BIT 0x1
#define PI4IO_SOFTWARE_RESET_BIT 0x0

/** 
 * \brief PI4IOE5V6408 pin IRQ callback type define
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param pin Number of the pin that generated the interrupt
*/
typedef void (*pi4io_irq_callback_t)(uint8_t pin);

/** 
 * \brief PI4IOE5V6408 reset callback type define
 * \ingroup peri_i2c_pi4ioe5v6408
*/
typedef void (*pi4io_rst_callback_t)(void);

/** 
 * \brief Register cache struct
 * \ingroup peri_i2c_pi4ioe5v6408
*/
typedef struct
{
    uint8_t device_id_and_control;
    uint8_t io_direction;
    uint8_t output_state;
    uint8_t output_hiz;
    uint8_t input_default_state;
    uint8_t pull_up_down_en;
    uint8_t pull_up_down_sel;
    uint8_t input_status;
    uint8_t interrupt_mask;
    uint8_t interrupt_status;
} pi4io_reg_copies_t;


/** 
 * \brief PI4IOE5V6408 instance struct
 * \ingroup peri_i2c_pi4ioe5v6408
*/
typedef struct
{
    i2c_inst_t *i2c;
    uint8_t addr;
    pi4io_reg_copies_t reg_copies;
    pi4io_irq_callback_t irq_callback;
    pi4io_rst_callback_t rst_callback;
} pi4io_inst_t;

/** 
 * \brief Lookup local register copy from the cache struct by register address
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * Primarily an internal function used to retrieve the local register copy from its address.
*/
uint8_t *pi4io_reg_copy_lookup(pi4io_inst_t *inst, uint8_t reg);

/** 
 * \brief Write to a register
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param reg Register address
 * \param data Data to write
*/
void pi4io_write_register_raw(pi4io_inst_t *inst, uint8_t reg, uint8_t data);

/** 
 * \brief Read from a register
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param reg Register address
 * \return Value of the register
*/
uint8_t pi4io_read_register_raw(pi4io_inst_t *inst, uint8_t reg);

/** 
 * \brief Write select bits to a register using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param reg Register address
 * \param mask Bit mask
 * \param data Data to write
*/
void pi4io_write_register_masked(pi4io_inst_t *inst, uint8_t reg, uint8_t mask, uint8_t data);

/** 
 * \brief Write a single bit to a register
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param reg Register address
 * \param bit Bit index
 * \param value Bit value to write
*/
void pi4io_write_register_bit(pi4io_inst_t *inst, uint8_t reg, uint8_t bit, bool value);

/** 
 * \brief Read a single bit from a register
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param reg Register address
 * \param bit Bit index
 * \return Value of the bit in the register
*/
bool pi4io_read_register_bit(pi4io_inst_t *inst, uint8_t reg, uint8_t bit);

/** 
 * \brief Initialise register cache struct
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
*/
void pi4io_init_copies(pi4io_inst_t *inst);

/** 
 * \brief Perform a software reset of the IO expander
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
*/
void pi4io_reset(pi4io_inst_t *inst);

/** 
 * \brief Initialise the IO expander & the instance struct
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param i2c I2C instance
 * \param addr I2C address
*/
void pi4io_init(pi4io_inst_t *inst, i2c_inst_t *i2c, uint8_t addr);

/** 
 * \brief Write to all pins at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param values Pin value byte to write
*/
void pi4io_write_pins_all(pi4io_inst_t *inst, uint8_t values);

/** 
 * \brief Read all pins at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \return Pin value byte
*/
uint8_t pi4io_read_pins_all(pi4io_inst_t *inst);

/** 
 * \brief Set all pin directions at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param values Pin direction value byte to write
*/
void pi4io_set_pin_dirs_all(pi4io_inst_t *inst, uint8_t values);

/** 
 * \brief Set all pin high-impedance states at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param values Pin high-impedance value byte to write
*/
void pi4io_set_pins_hiz_all(pi4io_inst_t *inst, uint8_t values);

/** 
 * \brief Set all pin pull-up/down enables at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param values Pin pull-up/down enable value byte to write
*/
void pi4io_set_pin_pulls_en_all(pi4io_inst_t *inst, uint8_t values);

/** 
 * \brief Set all pin pull-up/down polarities at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param values Pin pull-up/down polarity value byte to write
*/
void pi4io_set_pin_pulls_pol_all(pi4io_inst_t *inst, uint8_t values);

/** 
 * \brief Set all pin interrupt disable states at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param values Pin interrupt disable state value byte to write
*/
void pi4io_set_pin_interrupts_all(pi4io_inst_t *inst, uint8_t values);

/** 
 * \brief Set all pin default states at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * When an input pin changes state from its default state, an interrupt is generated.
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param values Pin default state value byte to write
*/
void pi4io_set_pin_input_default_states_all(pi4io_inst_t *inst, uint8_t values);

/** 
 * \brief Write to multiple pins at once using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param mask Pin mask
 * \param values Pin value byte to write
*/
void pi4io_write_pins_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values);

/** 
 * \brief Write multiple pin directions at once using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param mask Pin direction mask
 * \param values Pin direction value byte to write
*/
void pi4io_set_pin_dirs_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values);

/** 
 * \brief Write multiple pin high-impedance states at once using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param mask Pin high-impedance state mask
 * \param values Pin high-impedance state value byte to write
*/
void pi4io_set_pins_hiz_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values);

/** 
 * \brief Write multiple pin pull-up/down enables at once using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param mask Pin pull-up/down enable mask
 * \param values Pin pull-up/down enable value byte to write
*/
void pi4io_set_pin_pulls_en_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values);

/** 
 * \brief Write multiple pin pull-up/down polarities at once using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param mask Pin pull-up/down polarity mask
 * \param values Pin pull-up/down polarity value byte to write
*/
void pi4io_set_pin_pulls_pol_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values);

/** 
 * \brief Write multiple pin interrupt disables at once using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param mask Pin interrupt disable mask
 * \param values Pin interrupt disable value byte to write
*/
void pi4io_set_pin_interrupts_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values);

/** 
 * \brief Write multiple pin default states at once using a mask
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param mask Pin default state mask
 * \param values Pin default state value byte to write
*/
void pi4io_set_pin_input_default_states_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values);

/** 
 * \brief Write a single pin
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param pin Pin index
 * \param val Pin value to write
*/
void pi4io_write_pin(pi4io_inst_t *inst, uint8_t pin, bool val);

/** 
 * \brief Read a single pin
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param pin Pin index
 * \return Pin value
*/
bool pi4io_read_pin(pi4io_inst_t *inst, uint8_t pin);

/** 
 * \brief Write a single pin direction
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param pin Pin index
 * \param out Set pin to output
*/
void pi4io_set_pin_dir(pi4io_inst_t *inst, uint8_t pin, bool out);

/** 
 * \brief Write a single pin high-impedance state
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param pin Pin index
 * \param hi_z Set pin to high-impedance
*/
void pi4io_set_pin_hiz(pi4io_inst_t *inst, uint8_t pin, bool hi_z);

/** 
 * \brief Write a single pin pull-up/down enable and polarity
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param pin Pin index
 * \param en Enable pull-up/down
 * \param pull_up Set the pull polarity to up
*/
void pi4io_set_pin_pull(pi4io_inst_t *inst, uint8_t pin, bool en, bool pull_up);

/** 
 * \brief Write a single pin interrupt enable and edge polarity
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param pin Pin index
 * \param rising Set the edge polarity to rising edge
 * \param en Enable the interrupt
*/
void pi4io_set_pin_interrupt(pi4io_inst_t *inst, uint8_t pin, bool rising, bool en);

/** 
 * \brief Read all pin interrupt states at once
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * The value is cleared after reading!
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \return Pin interrupt state byte
*/
uint8_t pi4io_read_interrupts(pi4io_inst_t *inst);

/** 
 * \brief Register pin IRQ callback
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param callback Pin IRQ callback pointer
*/
void pi4io_register_irq_callback(pi4io_inst_t *inst, pi4io_irq_callback_t callback);

/** 
 * \brief Register reset callback
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * \param inst PI4IOE5V6408 instance struct
 * \param callback Reset callback pointer
*/
void pi4io_register_rst_callback(pi4io_inst_t *inst, pi4io_rst_callback_t callback);

/** 
 * \brief PI4IOE5V6408 IRQ handler
 * \ingroup peri_i2c_pi4ioe5v6408
 * 
 * Call this handler in the appropriate host GPIO IRQ handler or callback if you wish to use the callback interface.
 * 
 * \param inst PI4IOE5V6408 instance struct
*/
void pi4io_irq_handler(pi4io_inst_t *inst);

#endif