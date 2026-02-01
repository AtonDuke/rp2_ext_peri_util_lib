#ifndef NV_STRUCT_H
#define NV_STRUCT_H

#include <stdint.h>
#include <stddef.h>

/** 
 * \file nv_struct.h
 * \defgroup util_nv_struct util_nv_struct
 * \brief Library for storage of data in external non-volatile memories
 * 
 * As external memories connected via I2C, SPI and the like aren't typically mapped to the hosts address space, accessing data 
 * stored on them isn't as easy as simply doing an assignment operation in code. Furthermore, access to these memories is typically 
 * rather slow.
 * 
 * This library aims to solve these problems by defining a universal header and interface. 
 * All that is needed is to supply your own data structure that includes the predefined header struct at the top, 
 * interface functions that connect this library's interface to the memory and callbacks.
 * 
 * Read access is accelerated by caching the data inside the data structure.
*/

/** 
 * \brief Non-volatile data structure header struct
 * \ingroup util_nv_struct
 * 
 * This header struct stores the CRC32 checksum, version number and size of the data.
*/
typedef struct
{
    uint32_t crc32; //Polynomial: 0x04C11DB7, seed: 0xFFFFFFFF, no reflection, no XOR
    uint16_t version; //Starts at 1 and ends at 254
    uint16_t size; //Size of the entire nv_struct struct in bytes (including the header)
} nv_struct_header_t;

/** 
 * \brief Read interface function type definition
 * \ingroup util_nv_struct
 * 
 * \param addr Address in external memory
 * \param dst Destination byte array
 * \param n_bytes Number of bytes to read
*/
typedef void (*nv_struct_iface_read)(uint16_t addr, uint8_t *dst, size_t n_bytes);

/** 
 * \brief Write interface function type definition
 * \ingroup util_nv_struct
 * 
 * \param addr Addres in external memory
 * \param src Source byte array
 * \param n_bytes Number of bytes to read
*/
typedef void (*nv_struct_iface_write)(uint16_t addr, uint8_t *src, size_t n_bytes);

/** 
 * \brief Interface struct that holds pointers to the interface functions
 * \ingroup util_nv_struct
*/
typedef struct
{
    nv_struct_iface_read read_func;
    nv_struct_iface_write write_func;
} nv_struct_iface_t;

/** 
 * \brief Version mismatch callback type definition
 * \ingroup util_nv_struct
 * 
 * This callback is called when the initialization function detects a version mismatch between the data structure in the code 
 * and the data structure stored in the external memory.
 * 
 * \param iface Interface struct
 * \param header Data structure header
 * \param stored_header Header present in the external memory
*/
typedef int (*nv_struct_version_mismatch_callback)(nv_struct_iface_t iface, nv_struct_header_t *header, nv_struct_header_t stored_header);

/** 
 * \brief Corrupted data callback
 * \ingroup util_nv_struct
 * 
 * This callback is called when the checksum calculated by the initilization functions and the stored checksum don't match.
 * 
 * \param iface Interface struct
 * \param header Data structure header
*/
typedef void (*nv_struct_corrupted_callback)(nv_struct_iface_t iface, nv_struct_header_t *header);

/** 
 * \brief Callbacks struct that holds pointers to the callbacks
 * \ingroup util_nv_struct
*/
typedef struct
{
    nv_struct_version_mismatch_callback version_mismatch_callback;
    nv_struct_corrupted_callback corrupted_callback;
} nv_struct_callbacks_t;

/** 
 * \brief Initialize the data structure
 * \ingroup util_nv_struct
 * 
 * This function pre-caches the data, detects version mismatch, computes the checksum and calls appropriate callbacks.
 * 
 * \param iface Interface struct
 * \param header Data structure header
 * \param callbacks Callbacks struct
*/
void nv_struct_init(nv_struct_iface_t iface, nv_struct_header_t *header, nv_struct_callbacks_t callbacks);

/** 
 * \brief Write back modified data
 * \ingroup util_nv_struct
 * 
 * This function computes the new checksum and writes the modified data and checksum to the external memory.
 * 
 * \param iface Interface struct
 * \param header Data structure header
 * \param src Pointer to the source data cast to bytes
 * \param length Size of data in bytes
*/
void nv_struct_writeback(nv_struct_iface_t iface, nv_struct_header_t *header, uint8_t *src, size_t length);

/** 
 * \brief Read data directly from the external memory
 * \ingroup util_nv_struct
 * 
 * \param iface Interface struct
 * \param header Data structure header
 * \param dst Pointer to the destination entry in the data structure cast to bytes
 * \param length Size of data in bytes
*/
void nv_struct_read(nv_struct_iface_t iface, nv_struct_header_t *header, uint8_t *dst, size_t length);

/** 
 * \brief Compute the CRC32 checksum
 * \ingroup util_nv_struct
 * 
 * \param header Data structure header
*/
uint32_t nv_struct_compute_crc32(nv_struct_header_t *header);

#endif