#include "dac081c08x.h"

uint16_t dac081c08x_assemble_frame(dac081c08x_pd_states_t pd_state, uint8_t data)
{
    return (data & DAC081C08X_DATA_BITS) | ((pd_state << DAC081C08X_PD_SHIFT) & DAC081C08X_PD_BITS);
}

void dac081c08x_init(dac081c08x_inst_t *inst, i2c_inst_t *i2c, uint8_t addr)
{
    inst->i2c = i2c;
    inst->addr = addr;
    inst->data_copy = 0;
    inst->pd_state_copy = DAC081C08X_PD_NORM;

    const uint16_t frame = dac081c08x_assemble_frame(DAC081C08X_PD_NORM, 0);
    i2c_write_blocking(inst->i2c, inst->addr, (uint8_t*)&frame, 2, false);
}

void dac081c08x_write_all(dac081c08x_inst_t *inst, uint8_t data, dac081c08x_pd_states_t pd_state)
{
    inst->pd_state_copy = pd_state;
    inst->data_copy = data;
    const uint16_t frame = dac081c08x_assemble_frame(pd_state, data);
    i2c_write_blocking(inst->i2c, inst->addr, (uint8_t*)&frame, 2, false);
}

void dac081c08x_write_data(dac081c08x_inst_t *inst, uint8_t data)
{
    inst->data_copy = data;
    const uint16_t frame = dac081c08x_assemble_frame(inst->pd_state_copy, data);
    i2c_write_blocking(inst->i2c, inst->addr, (uint8_t*)&frame, 2, false);
}

void dac081c08x_change_pd_state(dac081c08x_inst_t *inst, dac081c08x_pd_states_t pd_state)
{
    inst->pd_state_copy = pd_state;
    const uint16_t frame = dac081c08x_assemble_frame(pd_state, inst->data_copy);
    i2c_write_blocking(inst->i2c, inst->addr, (uint8_t*)&frame, 2, false);
}