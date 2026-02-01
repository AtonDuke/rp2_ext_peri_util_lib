#ifndef ADS7042_H
#define ADS7042_H

#include "hardware/spi.h"
#include "hardware/pio.h"

/** 
 * \file ads7042.h
 * \defgroup peri_adc_ads7042 peri_adc_ads7042
 * \brief Peripheral driver for the ADS7042 SPI ADC
 * 
 * The ADS7042 is a 12-bit 1 MSPS SAR ADC with a SPI interface.
 * 
 * The ADC interface doesn't have a MOSI line, only a MISO line, as it doesn't have any configuration capability
 * besides internal calibration, which can be triggered by inserting additional dummy clock cycles.
 * 
 * It can be connected to the RP2 series MCUs using either the HW SPI block or the PIO, and this driver supports both cases.
*/

/** 
 * \brief Number of ADC bits
 * \ingroup peri_adc_ads7042
 * 
 * This define is useful for computing the sensed voltage
*/
#define ADS7042_NBITS 12

/**
 * \brief ADS7042 SPI interface types
 * \ingroup peri_adc_ads7042
 * 
 * This is used to diferentiate if the ADC is connected via a HW SPI block or a PIO
*/
typedef enum
{
    ADS7042_PIO,
    ADS7042_HW_SPI
} ads7042_iface_type_t;

/** 
 * \brief ADS7042 calibration states
 * \ingroup peri_adc_ads7042
*/
typedef enum
{
    ADS7042_STATE_PWRON,
    ADS7042_STATE_UNCAL,
    ADS7042_STATE_CAL
} ads7042_state_t;

/** 
 * \brief ADS7042 instance struct
 * \ingroup peri_adc_ads7042
 * 
 * This instance struct holds the interface type, PIO & SPI instances, SM & CSn pin numbers and ADC calibration state
*/
typedef struct
{
    ads7042_iface_type_t iface_type;
    union
    {
        PIO pio;
        spi_inst_t *hw_spi_inst;
    };
    union
    {
        uint sm;
        uint csn_pin;
    };
    ads7042_state_t state;
} ads7042_inst_t;

/** 
 * \brief Initialize an ADS7042 instance connected via PIO
 * \ingroup peri_adc_ads7042
 * 
 * This function doesn't calibrate the ADC and assumes it was just powered on!
 * There is no way to check the actual internal calibration state of the ADC!
 * 
 * \param inst Pointer to the ADS7042 instance struct
 * \param pio Pointer to the PIO block
 * \param clock_khz SPI clock in kHz
 * \param sclk_pin SPI SCLK pin number
 * \param sdi_pin SPI MISO pin number
 * \param csn_pin SPI CSn pin number
 * \return PICO_OK if initilization succeeded, PICO_ERROR_GENERIC if initilization failed
*/
int ads7042_init_pio(ads7042_inst_t *inst, PIO pio, uint clock_khz, uint sclk_pin, uint sdi_pin, uint csn_pin);

/** 
 * \brief Initialize an ADS7042 instance connected via SPI HW
 * \ingroup peri_adc_ads7042
 * 
 * This function doesn't calibrate the ADC and assumes it was just powered on!
 * There is no way to check the actual internal calibration state of the ADC!
 * 
 * \param inst Pointer to the ADS7042 instance struct
 * \param spi SPI HW block instance
 * \param sclk_pin SPI SCLK pin number
 * \param sdi_pin SPI MISO pin number
 * \param csn_pin SPI CSn pin number
 * \return PICO_OK - this function never fails
*/
int ads7042_init_hw_spi(ads7042_inst_t *inst, spi_inst_t *spi, uint clock_khz, uint sclk_pin, uint sdi_pin, uint csn_pin);
// Maybe make this a void return instead

/** 
 * \brief Calibrate the ADC
 * \ingroup peri_adc_ads7042
 * 
 * This function calibrates the ADC by setting the number of bits on the interface to the required number of clocks
 * and executing a dummy read. And then it resets the number of bits back to normal
 * 
 * \param inst Pointer to the ADS7042 instance struct
*/
void ads7042_offset_cal(ads7042_inst_t *inst);

/** 
 * \brief Start asynchronous read
 * \ingroup peri_adc_ads7042
 * 
 * \param inst Pointer to the ADS7042 instance struct
 * \return PICO_OK if successful, PICO_ERROR_GENERIC if TX FIFO full
*/
int ads7042_start_async_read(ads7042_inst_t *inst);

/** 
 * \brief Start an asynchronous read, blocking if TX FIFO full
 * \ingroup peri_adc_ads7042
 * 
 * This function blocks if PIO or SPI TX FIFO is full. RP2 SPI HW needs dummy data for reads and the PIO program
 * waits for dummy data to start a read
 * 
 * \param inst Pointer to the ADS7042 instance struct
*/
void ads7042_start_async_read_blocking(ads7042_inst_t *inst);

/** 
 * \brief Check if data is available in the RX FIFO
 * \ingroup peri_adc_ads7042
 * 
 * \param inst Pointer to the ADS7042 instance struct
 * \return True if data is available, false if not
*/
bool ads7042_is_data_available(ads7042_inst_t *inst);

/** 
 * \brief Get received data
 * \ingroup peri_adc_ads7042
 * 
 * \param inst Pointer to the ADS7042 instance struct
 * \return Received data
*/
uint32_t ads7042_get_data(ads7042_inst_t *inst);

/** 
 * \brief Get received data, blocking if RX FIFO empty
 * \ingroup peri_adc_ads7042
 * 
 * \param inst Pointer to the ADS7042 instance struct
 * \return Received data
*/
uint32_t ads7042_get_data_blocking(ads7042_inst_t *inst);

/** 
 * \brief Read from the ADC and block until the data arrives
 * \ingroup peri_adc_ads7042
 * 
 * \param inst Pointer to the ADS7042 instance struct
 * \return Received data
*/
uint32_t ads7042_read_blocking(ads7042_inst_t *inst);

#endif