#ifndef AT24CS0X_H
#define AT24CS0X_H

#include "hardware/i2c.h"

/** 
 * \file at24cs0x.h
 * \defgroup peri_eeprom_at24cs0x peri_eeprom_at24cs0x
 * \brief Peripheral driver for the AT24CS01 & AT24CS02 I2C EEPROMs
 * 
 * The AT24CS01 has a capacity of 1 Kb (128 x 8) and the AT24CS02 has a capacity of 2 Kb (256 x 8).
 * 
 * The EEPROM includes a unique 128 bit serial number.
 * 
 * The write endurance of the EEPROM is 1 000 000 write cycles.
 * 
 * Supports multi-byte reads and writes. Reads are wrapped around the memory boundary and writes are wrapped on an 8 byte page boundary.
 * 
 * 8 pin packages include three address pins for setting the lower 3 bits of the device's address.
*/

/** 
 * \brief The 4 high bits of the device EEPROM address
 * \ingroup peri_eeprom_at24cs0x
*/
#define AT24CS0X_EEPROM_BUS_ADDR_HI 0x50

/** 
 * \brief The 4 high bits of the device serial number address
 * \ingroup peri_eeprom_at24cs0x
*/
#define AT24CS0X_SERIAL_NUMBER_BUS_ADDR_HI 0x58

/** 
 * \brief Address byte of the serial number
 * \ingroup peri_eeprom_at24cs0x
 * 
 * This address byte needs to be sent to the device to read the serial number.
 * 
 * Only the highest 2 bits matter, the rest are don't care bits
*/
#define AT24CS0X_SERIAL_NUMBER_ADDR_BYTE 0x80

/** 
 * \brief Size of an EEPROM page
 * \ingroup peri_eeprom_at24cs0x
 * 
 * Multi-byte writes are wrapped around this boundary.
*/
#define AT24CS0X_PAGE_SIZE 8

/** 
 * \brief Maximum write cycle time of the EEPROM
 * \ingroup peri_eeprom_at24cs0x
 * 
 * This is the maximum time it takes for the EEPROM to complete its write cycle
*/
#define AT24CS0X_MAX_WRITE_CYCLE_TIME_MS 5

/** 
 * \brief Erase EEPROM byte by byte
 * \ingroup peri_eeprom_at24cs0x
 * \hideinitializer
*/
#define AT24CS0X_ERASE_BYTEWISE 0

/** 
 * \brief Erase EEPROM page by page
 * \ingroup peri_eeprom_at24cs0x
 * \hideinitializer
*/
#define AT24CS0X_ERASE_PAGEWISE 1

/** 
 * \brief Erase EEPROM using a buffer of predetermined size
 * \ingroup peri_eeprom_at24cs0x
 * \hideinitializer
*/
#define AT24CS0X_ERASE_BUFFERED 2

/** 
 * \brief Size of erase buffer
 * \ingroup peri_eeprom_at24cs0x
 * \hideinitializer
*/
#ifndef AT24CS0X_ERASE_BUFFER_SIZE
#define AT24CS0X_ERASE_BUFFER_SIZE 64
#endif

/** 
 * \brief Erase method to be used (BYTEWISE, PAGEWISE or BUFFERED)
 * \ingroup peri_eeprom_at24cs0x
 * \hideinitializer
*/
#ifndef AT24CS0X_ERASE_METHOD
#define AT24CS0X_ERASE_METHOD AT24CS0X_ERASE_PAGEWISE
#endif

/** 
 * \brief AT24CS0x EEPROM size enum
 * \ingroup peri_eeprom_at24cs0x
*/
typedef enum
{
    AT24CS01_SIZE = 1024,
    AT24CS02_SIZE = 2048
} at24cs0x_size_t;

/** 
 * \brief AT24CS0x instance struct
 * \ingroup peri_eeprom_at24cs0x
 * 
 * Holds the I2C instance, device address, EEPROM size, serial number, current EEPROM address, if a write is in progress and the last write time.
*/
typedef struct
{
    i2c_inst_t *i2c;
    uint8_t addr;
    at24cs0x_size_t size;
    uint32_t serial_number[4];
    volatile uint8_t current_address;
    volatile bool write_in_progress;
    absolute_time_t last_write_time;
} at24cs0x_inst_t;

/** 
 * \brief Check whether a write is in progress
 * \ingroup peri_eeprom_at24cs0x
 * 
 * Checks whether a write is in progress by first checking the corresponding boolean in the EEPROM instance struct.
 * 
 * If no write was initiated it returns right away with false. If a write was initiated, it then checks whether the maximum
 * write cycle time has passed. If it has passed, it writes into the instance struct and returns false.
 * If it hasn't passed, then it does the ACK poll procedure once, where it checks whether the device responds to its address with
 * an ACK.
 * 
 * This function does the ACK poll procedure only ONCE!
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \return 1 - write is in progress, 0 - write is not in progress, PICO_ERROR_GENERIC - failure
*/
int at24cs0x_check_write_in_progress(at24cs0x_inst_t *inst);

/** 
 * \brief Initialise the AT24CS0x instance struct
 * \ingroup peri_eeprom_at24cs0x
 * 
 * First initializes the instance struct with the provided values and then reads the EEPROM serial number.
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \param i2c Pointer to the I2C instance, which the EEPROM is connected to
 * \param bus_addr_lo The three low device address bits (for 5 pin packages 0x0)
 * \param size Size of the EEPROM
 * \return PICO_OK - initialization successful, PICO_ERROR_GENERIC - initialization failed
*/
int at24cs0x_init(at24cs0x_inst_t *inst, i2c_inst_t *i2c, uint8_t bus_addr_lo, at24cs0x_size_t size);

/** 
 * \brief Read the value at the current EEPROM address
 * \ingroup peri_eeprom_at24cs0x
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \param dest Pointer to the destination variable
 * \return PICO_OK - success, PICO_ERROR_GENERIC - failure
*/
int at24cs0x_read_current_addr(at24cs0x_inst_t *inst, uint8_t *dest);

/** 
 * \brief Read value at the given EEPROM address
 * \ingroup peri_eeprom_at24cs0x
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \param addr EEPROM address
 * \param dest Pointer to the destination variable
 * \return PICO_OK - success, PICO_ERROR_GENERIC - failure
*/
int at24cs0x_read_single_byte(at24cs0x_inst_t *inst, uint8_t addr, uint8_t *dest);

/** 
 * \brief Read given number of values at the given EEPROM address
 * \ingroup peri_eeprom_at24cs0x
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \param addr EEPROM address
 * \param dest Destination array
 * \param len Number of bytes to read
 * \return PICO_OK - success, PICO_ERROR_GENERIC - failure
*/
int at24cs0x_read_bytes(at24cs0x_inst_t *inst, uint8_t addr, uint8_t *dest, uint8_t len);

/** 
 * \brief Write a single byte at the given EEPROM address
 * \ingroup peri_eeprom_at24cs0x
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \param addr EEPROM address
 * \param data Byte to write
 * \return PICO_OK - success, PICO_ERROR_GENERIC - failure
*/
int at24cs0x_write_single_byte(at24cs0x_inst_t *inst, uint8_t addr, uint8_t data);

/** 
 * \brief Write a single byte at the given EEPROM address
 * \ingroup peri_eeprom_at24cs0x
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \param addr EEPROM address
 * \param data Source data array
 * \param len Number of bytes to write
 * \return PICO_OK - success, PICO_ERROR_GENERIC - failure
*/
int at24cs0x_write_bytes(at24cs0x_inst_t *inst, uint8_t addr, uint8_t *data, uint8_t len);

/** 
 * \brief Erase the entire EEPROM
 * \ingroup peri_eeprom_at24cs0x
 * 
 * \param inst Pointer to the AT24CS0x instance struct
 * \return PICO_OK - success, PICO_ERROR_GENERIC - failure
*/
int at24cs0x_erase(at24cs0x_inst_t *inst);

#endif