#include <stdint.h>

#include "dac121s101.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

uint16_t dac121s101_assemble_frame(dac121s101_pd_states_t pd_state, uint16_t data)
{
    return (data & DAC121S101_DATA_BITS) | ((pd_state << DAC121S101_PD_SHIFT) & DAC121S101_PD_BITS);
}

void dac121s101_init_hw_spi(dac121s101_inst_t *inst, spi_inst_t *hw_spi_inst, uint sclk_pin, uint mosi_pin, uint sync_pin)
{
    inst->hw_spi_inst = hw_spi_inst;
    inst->data_copy = 0;
    inst->pd_state_copy = DAC121S101_PD_NORM;

    gpio_set_function(sclk_pin, GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(sync_pin, GPIO_FUNC_SPI);

    spi_init(hw_spi_inst, 30 * MHZ);
    spi_set_format(spi1, 16, 0, 1, SPI_MSB_FIRST);
    hw_write_masked(&spi_get_hw(hw_spi_inst)->cr0, 0x1F, SPI_SSPCR0_FRF_BITS | SPI_SSPCR0_DSS_BITS);
    hw_write_masked(&spi_get_hw(hw_spi_inst)->cr1, 0x2, SPI_SSPCR1_MS_BITS | SPI_SSPCR1_SSE_BITS | SPI_SSPCR1_LBM_BITS);

    const uint16_t frame = dac121s101_assemble_frame(DAC121S101_PD_NORM, 0);
    spi_write16_blocking(hw_spi_inst, &frame, 1);
}

void dac121s101_write_all(dac121s101_inst_t *inst, uint16_t data, dac121s101_pd_states_t pd_state)
{
    inst->pd_state_copy = pd_state;
    inst->data_copy = data;
    const uint16_t frame = dac121s101_assemble_frame(pd_state, data);
    spi_write16_blocking(inst->hw_spi_inst, &frame, 1);
}

void dac121s101_write_data(dac121s101_inst_t *inst, uint16_t data)
{
    inst->data_copy = data;
    const uint16_t frame = dac121s101_assemble_frame(inst->pd_state_copy, data);
    spi_write16_blocking(inst->hw_spi_inst, &frame, 1);
}

void dac121s101_change_pd_state(dac121s101_inst_t *inst, dac121s101_pd_states_t pd_state)
{
    inst->pd_state_copy = pd_state;
    const uint16_t frame = dac121s101_assemble_frame(pd_state, inst->data_copy);
    spi_write16_blocking(inst->hw_spi_inst, &frame, 1);
}
