#include "pi4ioe5v6408.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"

uint8_t *pi4io_reg_copy_lookup(pi4io_inst_t *inst, uint8_t reg)
{
    switch(reg)
    {
        case PI4IO_REG_DEVICE_ID_AND_CONTROL:
            return &inst->reg_copies.device_id_and_control;
            break;
        case PI4IO_REG_IO_DIRECTION:
            return &inst->reg_copies.io_direction;
            break;
        case PI4IO_REG_OUTPUT_STATE:
            return &inst->reg_copies.output_state;
            break;
        case PI4IO_REG_OUTPUT_HIZ:
            return &inst->reg_copies.output_hiz;
            break;
        case PI4IO_REG_INPUT_DEFAULT_STATE:
            return &inst->reg_copies.input_default_state;
            break;
        case PI4IO_REG_PULL_UP_DOWN_EN:
            return &inst->reg_copies.pull_up_down_en;
            break;
        case PI4IO_REG_PULL_UP_DOWN_SEL:
            return &inst->reg_copies.pull_up_down_sel;
            break;
        case PI4IO_REG_INPUT_STATUS:
            return &inst->reg_copies.input_status;
            break;
        case PI4IO_REG_INTERRUPT_MASK:
            return &inst->reg_copies.interrupt_mask;
            break;
        case PI4IO_REG_INTERRUPT_STATUS:
            return &inst->reg_copies.interrupt_status;
            break;
        default:
            return NULL;
    }
}

void pi4io_write_register_raw(pi4io_inst_t *inst, uint8_t reg, uint8_t data)
{
    uint8_t *copy = pi4io_reg_copy_lookup(inst, reg);
    *copy = data;
    uint8_t msg[2];
    msg[0] = reg;
    msg[1] = data;

    int rval = i2c_write_blocking(inst->i2c, inst->addr, msg, 2, false);
    if(rval == PICO_ERROR_GENERIC) panic("PI4IO write error!");
    if(rval < 2) panic("PI4IO written less than 2 bytes!");
}

uint8_t pi4io_read_register_raw(pi4io_inst_t *inst, uint8_t reg)
{
    uint8_t data;

    int rval = i2c_write_blocking(inst->i2c, inst->addr, &reg, 1, true);
    if(rval == PICO_ERROR_GENERIC) panic("PI4IO read reg addr write error!");
    if(rval < 1) panic("PI4IO reg addr not written!");

    rval = i2c_read_blocking(inst->i2c, inst->addr, &data, 1, false);
    if(rval == PICO_ERROR_GENERIC) panic("PI4IO read error!");
    if(rval < 1) panic("PI4IO didn't read anything!");

    uint8_t *copy = pi4io_reg_copy_lookup(inst, reg);
    *copy = data;
    return data;
}

void pi4io_write_register_masked(pi4io_inst_t *inst, uint8_t reg, uint8_t mask, uint8_t data)
{
    uint8_t *copy = pi4io_reg_copy_lookup(inst, reg);
    uint8_t value = *copy ^ ((*copy ^ data) & mask);
    pi4io_write_register_raw(inst, reg, value);
}

void pi4io_write_register_bit(pi4io_inst_t *inst, uint8_t reg, uint8_t bit, bool value)
{
    pi4io_write_register_masked(inst, reg, 1u<<bit, (!!value)<<bit);
}

bool pi4io_read_register_bit(pi4io_inst_t *inst, uint8_t reg, uint8_t bit)
{
    return (pi4io_read_register_raw(inst, reg) & (1u<<bit)) >> bit;
}

void pi4io_init_copies(pi4io_inst_t *inst)
{
    inst->reg_copies.io_direction = 0x0;
    inst->reg_copies.output_state = 0x0;
    inst->reg_copies.output_hiz = 0xFF;
    inst->reg_copies.input_default_state = 0x0;
    inst->reg_copies.pull_up_down_en = 0xFF;
    inst->reg_copies.pull_up_down_sel = 0x0;
    inst->reg_copies.interrupt_mask = 0x0;
    pi4io_read_register_raw(inst, PI4IO_REG_DEVICE_ID_AND_CONTROL);
    pi4io_read_register_raw(inst, PI4IO_REG_INPUT_STATUS);
    pi4io_read_register_raw(inst, PI4IO_REG_INTERRUPT_STATUS);
    inst->reg_copies.device_id_and_control &= ~PI4IO_RESET_INTERRUPT_BIT;
}

void pi4io_reset(pi4io_inst_t *inst)
{
    pi4io_write_register_bit(inst, PI4IO_REG_DEVICE_ID_AND_CONTROL, PI4IO_SOFTWARE_RESET_BIT, true);
    pi4io_init_copies(inst);
}

void pi4io_init(pi4io_inst_t *inst, i2c_inst_t *i2c, uint8_t addr)
{
    inst->i2c = i2c;
    inst->addr = addr;
    inst->irq_callback = NULL;
    inst->rst_callback = NULL;
    pi4io_reset(inst);
}

void pi4io_write_pins_all(pi4io_inst_t *inst, uint8_t values)
{
    pi4io_write_register_raw(inst, PI4IO_REG_OUTPUT_STATE, values);
}

uint8_t pi4io_read_pins_all(pi4io_inst_t *inst)
{
    return pi4io_read_register_raw(inst, PI4IO_REG_INPUT_STATUS);
}

void pi4io_set_pin_dirs_all(pi4io_inst_t *inst, uint8_t values)
{
    pi4io_write_register_raw(inst, PI4IO_REG_IO_DIRECTION, values);
}

void pi4io_set_pins_hiz_all(pi4io_inst_t *inst, uint8_t values)
{
    pi4io_write_register_raw(inst, PI4IO_REG_OUTPUT_HIZ, values);
}

void pi4io_set_pin_pulls_en_all(pi4io_inst_t *inst, uint8_t values)
{
    pi4io_write_register_raw(inst, PI4IO_REG_PULL_UP_DOWN_EN, values);
}

void pi4io_set_pin_pulls_pol_all(pi4io_inst_t *inst, uint8_t values)
{
    pi4io_write_register_raw(inst, PI4IO_REG_PULL_UP_DOWN_SEL, values);
}

void pi4io_set_pin_interrupts_all(pi4io_inst_t *inst, uint8_t values)
{
    pi4io_write_register_raw(inst, PI4IO_REG_INTERRUPT_MASK, values);
}

void pi4io_set_pin_input_default_states_all(pi4io_inst_t *inst, uint8_t values)
{
    pi4io_write_register_raw(inst, PI4IO_REG_INPUT_DEFAULT_STATE, values);
}

void pi4io_write_pins_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values)
{
    pi4io_write_register_masked(inst, PI4IO_REG_OUTPUT_STATE, mask, values);
}

void pi4io_set_pin_dirs_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values)
{
    pi4io_write_register_masked(inst, PI4IO_REG_IO_DIRECTION, mask, values);
}

void pi4io_set_pins_hiz_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values)
{
    pi4io_write_register_masked(inst, PI4IO_REG_OUTPUT_HIZ, mask, values);
}

void pi4io_set_pin_pulls_en_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values)
{
    pi4io_write_register_masked(inst, PI4IO_REG_PULL_UP_DOWN_EN, mask, values);
}

void pi4io_set_pin_pulls_pol_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values)
{
    pi4io_write_register_masked(inst, PI4IO_REG_PULL_UP_DOWN_SEL, mask, values);
}

void pi4io_set_pin_interrupts_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values)
{
    pi4io_write_register_masked(inst, PI4IO_REG_INTERRUPT_MASK, mask, values);
}

void pi4io_set_pin_input_default_states_masked(pi4io_inst_t *inst, uint8_t mask, uint8_t values)
{
    pi4io_write_register_masked(inst, PI4IO_REG_INPUT_DEFAULT_STATE, mask, values);
}

void pi4io_write_pin(pi4io_inst_t *inst, uint8_t pin, bool val)
{
    pi4io_write_register_bit(inst, PI4IO_REG_OUTPUT_STATE, pin, val);
}

bool pi4io_read_pin(pi4io_inst_t *inst, uint8_t pin)
{
    return pi4io_read_register_bit(inst, PI4IO_REG_INPUT_STATUS, pin);
}

void pi4io_set_pin_dir(pi4io_inst_t *inst, uint8_t pin, bool dir)
{
    pi4io_write_register_bit(inst, PI4IO_REG_IO_DIRECTION, pin, dir);
}

void pi4io_set_pin_hiz(pi4io_inst_t *inst, uint8_t pin, bool hi_z)
{
    pi4io_write_register_bit(inst, PI4IO_REG_OUTPUT_HIZ, pin, hi_z);
}

void pi4io_set_pin_pull(pi4io_inst_t *inst, uint8_t pin, bool en, bool pull_up)
{
    pi4io_write_register_bit(inst, PI4IO_REG_PULL_UP_DOWN_EN, pin, en);
    pi4io_write_register_bit(inst, PI4IO_REG_PULL_UP_DOWN_SEL, pin, pull_up);
}

void pi4io_set_pin_interrupt(pi4io_inst_t *inst, uint8_t pin, bool rising, bool en)
{
    pi4io_write_register_bit(inst, PI4IO_REG_INTERRUPT_MASK, pin, !en);
    pi4io_write_register_bit(inst, PI4IO_REG_INPUT_DEFAULT_STATE, pin, !rising);
}

uint8_t pi4io_read_interrupts(pi4io_inst_t *inst)
{
    return pi4io_read_register_raw(inst, PI4IO_REG_INTERRUPT_STATUS);
}

void pi4io_register_irq_callback(pi4io_inst_t *inst, pi4io_irq_callback_t callback)
{
    inst->irq_callback = callback;
}

void pi4io_register_rst_callback(pi4io_inst_t *inst, pi4io_rst_callback_t callback)
{
    inst->rst_callback = callback;
}

void pi4io_irq_handler(pi4io_inst_t *inst)
{
    uint8_t irqs = pi4io_read_interrupts(inst);
    for(uint8_t pin = 0; irqs && pin < 8; ++pin)
    {
        if((irqs & 1u) && inst->irq_callback != NULL)
        {
            inst->irq_callback(pin);
        }
        irqs >> 1;
    }

    bool reset_intr = pi4io_read_register_bit(inst, PI4IO_REG_DEVICE_ID_AND_CONTROL, PI4IO_RESET_INTERRUPT_BIT);
    if(reset_intr && inst->rst_callback != NULL)
    {
        pi4io_init_copies(inst);
        inst->rst_callback();
    }
}
