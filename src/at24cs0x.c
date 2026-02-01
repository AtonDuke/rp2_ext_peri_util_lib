#include "at24cs0x.h"

#include "pico/stdio.h"
#include "hardware/i2c.h"

#include <stdio.h>

//unsigned int check_counter = 0;
//static char debug_buffer[256];

/*char *get_addr_check_count(uint8_t addr)
{
    sprintf(debug_buffer, "Debug info\nAddress: %u\nCheck counter: %u\n", addr, check_counter);
    return debug_buffer;
}*/

int at24cs0x_check_write_in_progress(at24cs0x_inst_t *inst)
{
    if(!inst->write_in_progress) return 0; //no point in figuring out if a write is in progress if it's not

    //no need to do ACK polling if max write cycle time was reached
    if(delayed_by_ms(inst->last_write_time, AT24CS0X_MAX_WRITE_CYCLE_TIME_MS) < get_absolute_time())
    {
        inst->write_in_progress = false;
        return 0;
    }

    while (!i2c_get_write_available(inst->i2c))
    {
        tight_loop_contents();
    }

    inst->i2c->hw->enable = 0;
    inst->i2c->hw->tar = inst->addr;
    inst->i2c->hw->enable = 1;

    //send current address and don't do anything else to not increment the address counter
    inst->i2c->hw->data_cmd = inst->current_address | I2C_IC_DATA_CMD_STOP_BITS;

    //wait for STOP condition
    while(!(inst->i2c->hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_STOP_DET_BITS)) tight_loop_contents();
    inst->i2c->hw->clr_stop_det;

    //read transmit abort source after transmission stops
    uint32_t abort_reason = inst->i2c->hw->tx_abrt_source;
    inst->i2c->hw->clr_tx_abrt;

    //panic if there's an abort for any reason other than NACK on address
    if(abort_reason & ~I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_BITS) return PICO_ERROR_GENERIC;

    //if there's a NACK on address, the EEPROM is busy writing
    inst->write_in_progress = abort_reason & I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_BITS;

    //++check_counter;

    return inst->write_in_progress ? 1 : 0;
}

int at24cs0x_init(at24cs0x_inst_t *inst, i2c_inst_t *i2c, uint8_t bus_addr_lo, at24cs0x_size_t size)
{
    inst->i2c = i2c;
    inst->addr = AT24CS0X_EEPROM_BUS_ADDR_HI | bus_addr_lo;
    inst->size = size;
    
    int ret_val;
    ret_val = i2c_write_blocking(i2c, AT24CS0X_SERIAL_NUMBER_BUS_ADDR_HI | bus_addr_lo, &(uint8_t){AT24CS0X_SERIAL_NUMBER_ADDR_BYTE}, 1, true);
    if(ret_val < 1) return PICO_ERROR_GENERIC;
    ret_val = i2c_read_blocking(i2c, AT24CS0X_SERIAL_NUMBER_BUS_ADDR_HI | bus_addr_lo, (uint8_t*)inst->serial_number, 4*4, false);
    if(ret_val < 4*4) return PICO_ERROR_GENERIC;

    ret_val = i2c_write_blocking(inst->i2c, inst->addr, &(uint8_t){0x0}, 1, false);
    if(ret_val < 1) return PICO_ERROR_GENERIC;

    inst->current_address = 0x0;
    inst->write_in_progress = false;
    return PICO_OK;
}

int at24cs0x_read_current_addr(at24cs0x_inst_t *inst, uint8_t *dest)
{
    //check_counter = 0;
    int ret_val = at24cs0x_check_write_in_progress(inst);
    while(ret_val > 0)
    {
        ret_val = at24cs0x_check_write_in_progress(inst);
    }
    if(ret_val < 0) return PICO_ERROR_GENERIC;

    //hw_set_bits(&i2c_get_hw(i2c1)->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);
    ret_val = i2c_read_blocking(inst->i2c, inst->addr, dest, 1, false);
    if(ret_val < 1) return PICO_ERROR_GENERIC;
    //hw_clear_bits(&i2c1->hw->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);

    ++inst->current_address;
    inst->current_address %= inst->size / 8;

    //printf("Read current address check count: %u\n", check_counter);

    return PICO_OK;
}

int at24cs0x_read_single_byte(at24cs0x_inst_t *inst, uint8_t addr, uint8_t *dest)
{
    //check_counter = 0;
    int ret_val = at24cs0x_check_write_in_progress(inst);
    while(ret_val > 0)
    {
        ret_val = at24cs0x_check_write_in_progress(inst);
    }
    if(ret_val < 0) return PICO_ERROR_GENERIC;

    //i2c1->hw->intr_mask = I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
    ret_val = i2c_write_blocking(inst->i2c, inst->addr, &addr, 1, true);
    if(ret_val < 1) return PICO_ERROR_GENERIC;

    ret_val = i2c_read_blocking(inst->i2c, inst->addr, dest, 1, false);
    if(ret_val < 1) return PICO_ERROR_GENERIC;
    //i2c1->hw->intr_mask = 0;

    inst->current_address = addr + 1;
    inst->current_address %= inst->size / 8;

    //printf("Read single byte check counter: %u\n", check_counter);

    return PICO_OK;
}

int at24cs0x_read_bytes(at24cs0x_inst_t *inst, uint8_t addr, uint8_t *dest, uint8_t len)
{
    //check_counter = 0;
    int ret_val = at24cs0x_check_write_in_progress(inst);
    while(ret_val > 0)
    {
        ret_val = at24cs0x_check_write_in_progress(inst);
    }
    if(ret_val < 0) return PICO_ERROR_GENERIC;

    //hw_set_bits(&i2c_get_hw(i2c1)->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);
    ret_val = i2c_write_blocking(inst->i2c, inst->addr, &addr, 1, true);
    if(ret_val < 1) return PICO_ERROR_GENERIC;

    //hw_set_bits(&i2c_get_hw(i2c1)->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);
    ret_val = i2c_read_blocking(inst->i2c, inst->addr, dest, len, false);
    if(ret_val < len) return PICO_ERROR_GENERIC;
    //hw_clear_bits(&i2c1->hw->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);

    inst->current_address = addr + len;
    inst->current_address %= inst->size / 8;

    //printf("Read bytes check counter: %u\n", check_counter);
    return PICO_OK;
}

int at24cs0x_write_single_byte(at24cs0x_inst_t *inst, uint8_t addr, uint8_t data)
{
    //check_counter = 0;
    int ret_val = at24cs0x_check_write_in_progress(inst);
    while(ret_val > 0)
    {
        ret_val = at24cs0x_check_write_in_progress(inst);
    }
    if(ret_val < 0) return PICO_ERROR_GENERIC;

    inst->write_in_progress = true;

    uint8_t frame[] = {addr, data};
    //hw_set_bits(&i2c_get_hw(i2c1)->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);
    ret_val = i2c_write_blocking(inst->i2c, inst->addr, frame, 2, false);
    if(ret_val < 2) return PICO_ERROR_GENERIC;
    //hw_clear_bits(&i2c1->hw->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);

    inst->last_write_time = get_absolute_time();

    inst->current_address = addr + 1;
    if(inst->current_address % AT24CS0X_PAGE_SIZE == 0) //on write the address counter wraps on page boundary
    {
        inst->current_address -= AT24CS0X_PAGE_SIZE;
    }

    //printf("Write single byte check counter: %u\n");
    return PICO_OK;
}

int at24cs0x_write_bytes(at24cs0x_inst_t *inst, uint8_t addr, uint8_t *data, uint8_t len)
{
    int remains = len;
    int i = 0;
    while(remains > 0)
    {
        //check_counter = 0;
        int ret_val = at24cs0x_check_write_in_progress(inst);
        while(ret_val > 0)
        {
            ret_val = at24cs0x_check_write_in_progress(inst);
        }
        if(ret_val < 0) return PICO_ERROR_GENERIC;

        size_t frame_size = addr % AT24CS0X_PAGE_SIZE > 0 ? AT24CS0X_PAGE_SIZE - addr % AT24CS0X_PAGE_SIZE : AT24CS0X_PAGE_SIZE;
        if(frame_size > remains)
        {
            frame_size = remains;
        }

        inst->write_in_progress = true;

        //hw_set_bits(&i2c_get_hw(i2c1)->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);
        ret_val = i2c_write_burst_blocking(inst->i2c, inst->addr, &addr, 1);
        if(ret_val < 1) return PICO_ERROR_GENERIC;

        //hw_set_bits(&i2c_get_hw(i2c1)->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);
        ret_val = i2c_write_blocking(inst->i2c, inst->addr, &(data[i]), frame_size, false);
        if(ret_val < 1) return PICO_ERROR_GENERIC;
        //hw_clear_bits(&i2c1->hw->intr_mask, I2C_IC_INTR_MASK_M_TX_ABRT_BITS);

        inst->last_write_time = get_absolute_time();

        addr += frame_size;
        remains -= frame_size;
        i += frame_size;

        //printf("Write byte check counter: %u\n", check_counter);
    }

    inst->current_address  = addr + len;
    if(inst->current_address % AT24CS0X_PAGE_SIZE == 0) //on write the address counter wraps on page boundary
    {
        inst->current_address -= AT24CS0X_PAGE_SIZE;
    }
    return PICO_OK;
}

int at24cs0x_erase(at24cs0x_inst_t *inst)
{
    #if AT24CS0X_ERASE_METHOD == AT24CS0X_ERASE_BYTEWISE //TODO: does not work for unknown reasons
    for(int i = 0; i < inst->size; ++i)
    {
        uint8_t val;
        if(at24cs0x_read_single_byte(inst, i, &val) < 0) return PICO_ERROR_GENERIC;
        if(val != 255)
        {
            if(at24cs0x_write_single_byte(inst, i, 255) < 0) return PICO_ERROR_GENERIC;
        }
    }
    return PICO_OK;
    #elif AT24CS0X_ERASE_METHOD == AT24CS0X_ERASE_PAGEWISE
    for(int i = 0; i < inst->size; i += AT24CS0X_PAGE_SIZE)
    {
        uint8_t page_buffer[AT24CS0X_PAGE_SIZE];
        if(at24cs0x_read_bytes(inst, i, page_buffer, AT24CS0X_PAGE_SIZE) < 0) return PICO_ERROR_GENERIC;
        int start_index = -1, stop_index = -1;
        for(int j = 0; j < AT24CS0X_PAGE_SIZE; ++j)
        {
            if(page_buffer[j] != 255)
            {
                stop_index = j;
                if(start_index < 0)
                {
                    start_index = j;
                }
                page_buffer[j] = 255;
            }
            else if(start_index > -1)
            {
                if(at24cs0x_write_bytes(inst, i + start_index, page_buffer, stop_index - start_index + 1) < 0) return PICO_ERROR_GENERIC;
                start_index = -1;
            }
        }
        if(start_index > -1)
        {
            if(at24cs0x_write_bytes(inst, i + start_index, page_buffer, stop_index - start_index + 1) < 0) return PICO_ERROR_GENERIC;
        }
    }
    return PICO_OK;
    #elif AT24CS0X_ERASE_METHOD == AT24CS0X_ERASE_BUFFERED
    for(int i = 0; i < inst->size; i += AT24CS0X_ERASE_BUFFER_SIZE)
    {
        uint8_t buffer[AT24CS0X_ERASE_BUFFER_SIZE];
        if(at24cs0x_read_bytes(inst, i, buffer, AT24CS0X_ERASE_BUFFER_SIZE) < 0) return PICO_ERROR_GENERIC;
        int start_index = -1, stop_index = -1;
        for(int j = 0; j < AT24CS0X_ERASE_BUFFER_SIZE; ++j)
        {
            if(buffer[j] != 255)
            {
                stop_index = j;
                if(start_index < 0)
                {
                    start_index = j;
                }
                buffer[j] = 255;
            }
            else if(start_index > -1)
            {
                if(at24cs0x_write_bytes(inst, i + start_index, buffer, stop_index - start_index + 1) < 0) return PICO_ERROR_GENERIC;
                start_index = -1;
            }
        }
        if(start_index > -1)
        {
            if(at24cs0x_write_bytes(inst, i + start_index, buffer, stop_index - start_index + 1) < 0) return PICO_ERROR_GENERIC;
        }
    }
    #else
        #error "AT24CS0X unknown erase method"
    #endif
}
