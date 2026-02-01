#include "nv_struct.h"

#include "hardware/dma.h"
#include "pico/stdio.h"

#include <stdio.h>

void nv_struct_init(nv_struct_iface_t iface, nv_struct_header_t *header, nv_struct_callbacks_t callbacks)
{
    //Read the stored header
    nv_struct_header_t stored_header;
    iface.read_func(0, (uint8_t*)&stored_header, sizeof(nv_struct_header_t));

    //Version check
    if(stored_header.version != header->version)
    {
        if(callbacks.version_mismatch_callback(iface, header, stored_header))
        {
            callbacks.corrupted_callback(iface, header);
        }
        return;
    }

    //Size check
    if(stored_header.size != header->size)
    {
        callbacks.corrupted_callback(iface, header);
        return;
    }

    //Read stored data
    iface.read_func(sizeof(nv_struct_header_t), (uint8_t*)(header) + sizeof(nv_struct_header_t), header->size - sizeof(nv_struct_header_t));

    //Compute checksum
    uint32_t computed_crc32 = nv_struct_compute_crc32(header);
    if(stored_header.crc32 != computed_crc32)
    {
        callbacks.corrupted_callback(iface, header);
        return;
    }

    header->crc32 = stored_header.crc32;
}

void nv_struct_writeback(nv_struct_iface_t iface, nv_struct_header_t *header, uint8_t *src, size_t length)
{
    header->crc32 = nv_struct_compute_crc32(header);
    iface.write_func(0, (uint8_t*)&(header->crc32), sizeof(uint32_t));
    iface.write_func(src - (uint8_t*)header, src, length);
}

void nv_struct_read(nv_struct_iface_t iface, nv_struct_header_t *header, uint8_t *dst, size_t length)
{
    iface.read_func(dst - (uint8_t*)header, dst, length);
}

uint32_t nv_struct_compute_crc32(nv_struct_header_t *header)
{
    //Compute CRC32 using the DMA sniffer
    uint32_t dummy;
    uint dma_chan = dma_claim_unused_channel(true);
    dma_sniffer_set_data_accumulator(0xFFFFFFFF);
    dma_sniffer_enable(dma_chan, DMA_SNIFF_CTRL_CALC_VALUE_CRC32, true);
    dma_channel_config dma_conf = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_8);
    channel_config_set_irq_quiet(&dma_conf, true);
    channel_config_set_sniff_enable(&dma_conf, true);
    dma_channel_configure(dma_chan, &dma_conf, &dummy, &(header->version), header->size - sizeof(header->crc32), true);

    dma_channel_wait_for_finish_blocking(dma_chan);
    
    uint32_t computed_crc32 = dma_sniffer_get_data_accumulator();
    dma_channel_cleanup(dma_chan);
    dma_channel_unclaim(dma_chan);
    return computed_crc32;
}
