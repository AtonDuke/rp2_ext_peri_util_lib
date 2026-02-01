#include "ads7042.h"

#include "adc_spi_master_unidir.pio.h"
#include "hardware/clocks.h"

#define ADS7042_FRAME_LEN 14
#define ADS7042_CAL_CLOCKS 16
#define ADS7042_RECAL_CLOCKS 32

#if NUM_PIOS > 2
    static int ads7042_prog_offsets[3] = {-1, -1, -1};
#else
    static int ads7042_prog_offsets[2] = {-1, -1};
#endif

int ads7042_init_pio(ads7042_inst_t *inst, PIO pio, uint clock_khz, uint sclk_pin, uint sdi_pin, uint csn_pin)
{
    uint pio_index = pio_get_index(pio);
    int offset;
    if(ads7042_prog_offsets[pio_index] < 0)
    {
        offset = pio_add_program(pio, &adc_spi_master_unidir_program);
        ads7042_prog_offsets[pio_index] = offset;
        if(offset == PICO_ERROR_GENERIC) return PICO_ERROR_GENERIC;
    }

    offset = ads7042_prog_offsets[pio_index];
    inst->iface_type = ADS7042_PIO;
    inst->pio = pio;
    inst->sm = pio_claim_unused_sm(pio, false);
    if(inst->sm == PICO_ERROR_GENERIC) return PICO_ERROR_GENERIC;
    float div = (float)clock_get_hz(clk_sys) / (clock_khz * 4 * KHZ);
    adc_spi_master_unidir_init(pio, inst->sm, offset, sclk_pin, sdi_pin, csn_pin, div, ADS7042_FRAME_LEN);

    inst->state = ADS7042_STATE_PWRON;

    return PICO_OK;
}

int ads7042_init_hw_spi(ads7042_inst_t *inst, spi_inst_t *spi, uint clock_khz, uint sclk_pin, uint sdi_pin, uint csn_pin)
{
    inst->iface_type = ADS7042_HW_SPI;
    inst->hw_spi_inst = spi;
    gpio_set_function(sclk_pin, GPIO_FUNC_SPI);
    gpio_set_function(sdi_pin, GPIO_FUNC_SPI);
    gpio_set_function(csn_pin, GPIO_FUNC_SPI);
    gpio_set_dir(csn_pin, GPIO_OUT);
    gpio_put(csn_pin, 1);
    inst->csn_pin = csn_pin;
    spi_init(spi, clock_khz * KHZ);
    spi_set_format(spi, ADS7042_FRAME_LEN, 0, 0, SPI_MSB_FIRST);
    return PICO_OK;
}

// Private function to change number of bits in the SPI master
static void change_n_bits(ads7042_inst_t *inst, uint n_bits)
{
    if(inst->iface_type == ADS7042_PIO)
    {
        pio_sm_set_enabled(inst->pio, inst->sm, false);
        adc_spi_master_unidir_restart(inst->pio, inst->sm, ads7042_prog_offsets[pio_get_index(inst->pio)]);

        volatile uint32_t *shiftctrl = &inst->pio->sm[inst->sm].shiftctrl;
        uint32_t xor_mask = (*shiftctrl ^ (n_bits << PIO_SM0_SHIFTCTRL_PUSH_THRESH_LSB)) & PIO_SM0_SHIFTCTRL_PUSH_THRESH_BITS;
        hw_xor_bits(&inst->pio->sm[inst->sm].shiftctrl, xor_mask);

        pio_sm_exec(inst->pio, inst->sm, pio_encode_set(pio_x, n_bits-2));
        pio_sm_exec(inst->pio, inst->sm, pio_encode_set(pio_y, n_bits-2));

        pio_sm_set_enabled(inst->pio, inst->sm, true);
    }
    else
    {
        // RP2 SPI HW supports max 16 bits
        spi_set_format(inst->hw_spi_inst, n_bits > 16 ? 16 : n_bits, 0, 0, SPI_MSB_FIRST);
    }
}

// Private function to perform a dummy read
static void dummy_read(ads7042_inst_t *inst)
{
    if(inst->iface_type == ADS7042_PIO)
    {
        pio_sm_clear_fifos(inst->pio, inst->sm);
        pio_sm_put(inst->pio, inst->sm, 1);
        pio_sm_get_blocking(inst->pio, inst->sm);
    }
    else
    {
        // RP2 SPI HW supports max 16 bits
        if(inst->state == ADS7042_STATE_PWRON)
        {
            uint16_t dummy;
            spi_read16_blocking(inst->hw_spi_inst, 0, &dummy, 1);
        }
        else
        {
            uint16_t dummy[2];
            gpio_set_function(inst->csn_pin, GPIO_FUNC_SIO);
            gpio_put(inst->csn_pin, 0);
            spi_read16_blocking(inst->hw_spi_inst, 0, dummy, 2);
            gpio_put(inst->csn_pin, 1);
            gpio_set_function(inst->csn_pin, GPIO_FUNC_SPI);
        }
    }
}

void ads7042_offset_cal(ads7042_inst_t *inst)
{
    if(inst->state == ADS7042_STATE_PWRON)
    {
        change_n_bits(inst, ADS7042_CAL_CLOCKS);
    }
    else
    {
        change_n_bits(inst, ADS7042_RECAL_CLOCKS);
    }
    dummy_read(inst);
    change_n_bits(inst, ADS7042_FRAME_LEN);
    inst->state = ADS7042_STATE_CAL;
}

int ads7042_start_async_read(ads7042_inst_t *inst)
{
    if(inst->iface_type == ADS7042_PIO)
    {
        if(pio_sm_is_tx_fifo_full(inst->pio, inst->sm)) return PICO_ERROR_GENERIC;
        pio_sm_put(inst->pio, inst->sm, 1);
    }
    else
    {
        if(!spi_is_writable(inst->hw_spi_inst)) return PICO_ERROR_GENERIC;
        spi_get_hw(inst->hw_spi_inst)->dr = 1;
    }

    if(inst->state == ADS7042_STATE_PWRON)
    {
        inst->state = ADS7042_STATE_UNCAL;
    }
    return PICO_OK;
}

void ads7042_start_async_read_blocking(ads7042_inst_t *inst)
{
    if(inst->iface_type == ADS7042_PIO)
    {
        pio_sm_put_blocking(inst->pio, inst->sm, 1);
    }
    else
    {
        while(!spi_is_writable(inst->hw_spi_inst))
        {
            tight_loop_contents();
        }
        spi_get_hw(inst->hw_spi_inst)->dr = 1;
    }
    
    if(inst->state == ADS7042_STATE_PWRON)
    {
        inst->state = ADS7042_STATE_UNCAL;
    }
}

bool ads7042_is_data_available(ads7042_inst_t *inst)
{
    if(inst->iface_type == ADS7042_PIO)
    {
        return !pio_sm_is_rx_fifo_empty(inst->pio, inst->sm);
    }
    else
    {
        return spi_is_readable(inst->hw_spi_inst);
    }
}

uint32_t ads7042_get_data(ads7042_inst_t *inst)
{
    if(inst->iface_type == ADS7042_PIO)
    {
        return pio_sm_get(inst->pio, inst->sm);
    }
    else
    {
        return spi_get_hw(inst->hw_spi_inst)->dr;
    }
}

uint32_t ads7042_get_data_blocking(ads7042_inst_t *inst)
{
    if(inst->iface_type == ADS7042_PIO)
    {
        return pio_sm_get_blocking(inst->pio, inst->sm);
    }
    else
    {
        while(!spi_is_readable(inst->hw_spi_inst))
        {
            tight_loop_contents();
        }
        return spi_get_hw(inst->hw_spi_inst)->dr;
    }
}

uint32_t ads7042_read_blocking(ads7042_inst_t *inst)
{
    ads7042_start_async_read_blocking(inst);
    
    return ads7042_get_data_blocking(inst);
}
