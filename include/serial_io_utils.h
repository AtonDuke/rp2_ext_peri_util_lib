#ifndef SERIAL_IO_UTILS_H
#define SERIAL_IO_UTILS_H

/**
 * \file serial_io_utils.h
 * \defgroup util_serial_io_utils util_serial_io_utils
 * \brief Serial IO utilities
 * 
 * For now includes just two functions for reading entire lines in a non-blocking and blocking manner, and supporting data 
 * structures.
 * 
 * The line reading functions allow the use of callbacks for certain special characters. BEL, ENQ, ACK, NAK, XON and XOFF.
 * 
 * The line reading fuction settings include echo on/off, stripping CR/LF line ends, capturing raw input and processing backspace.
*/

/**
 * \brief Delete control sequence
 * \ingroup util_serial_io_utils
 * 
 * This sequence deletes a character.
*/
#define SERIAL_DEL_CTRL_SEQ "\e[P"

/**
 * \brief BEL character callback
 * \ingroup util_serial_io_utils
*/
typedef void (*serial_read_bell_callback)(void);

/**
 * \brief ENQ character callback
 * \ingroup util_serial_io_utils
 */
typedef void (*serial_read_enquiry_callback)(void);

/**
 * \brief ACK/NAK character callback
 * \ingroup util_serial_io_utils
 * 
 * \param ack true - ACK detected, false - NAK detected
 */
typedef void (*serial_read_ack_nak_callback)(bool ack);

/**
 * \brief XON/XOFF character callback
 * \ingroup util_serial_io_utils
 * 
 * \param xon true - XON detected, false - XOFF detected
 */
typedef void (*serial_read_xon_xoff_callback)(bool xon);

/**
 * \brief Line read function settings struct
 * \ingroup util_serial_io_utils
 * 
 * This struct holds pointers to special character callbacks and line read function behavior settings.
 * 
 * \note Setting raw_input to true disables parsing except calling callbacks and terminating on line ends!
 */
typedef struct
{
    bool echo;
    bool strip_crlf_ending;
    bool raw_input;
    bool del_on_backspace;
    serial_read_bell_callback bell_callback;
    serial_read_enquiry_callback enquiry_callback;
    serial_read_ack_nak_callback ack_nak_callback;
    serial_read_xon_xoff_callback xon_xoff_callback;
} serial_read_settings_t;

/**
 * \brief Line read character buffer
 * \ingroup util_serial_io_utils
 * 
 * This buffer struct holds pointer to char array with characters read, size of the buffer and buffer index.
 */
typedef struct
{
    char *data;
    unsigned int size;
    unsigned int index;
} serial_read_buffer_t;


/**
 * \brief Blocking line read function
 * \ingroup util_serial_io_utils
 * 
 * This function reads an entire line in a blocking manner.
 * 
 * \param buffer Pointer to character buffer struct
 * \param settings Settings struct
 * \return PICO_ERROR_GENERIC if a buffer overflow occurs, length of line on success
 */
int serial_read_line_blocking(serial_read_buffer_t *buffer, serial_read_settings_t settings);

/**
 * \brief Blocking line read function
 * \ingroup util_serial_io_utils
 * 
 * This function reads an entire line in a non-blocking manner.
 * 
 * \param buffer Pointer to character buffer struct
 * \param settings Settings struct
 * \return PICO_ERROR_GENERIC if a buffer overflow occurs, PICO_OK if line end not received, length of line on success
 */
int serial_read_line(serial_read_buffer_t *buffer, serial_read_settings_t settings);

#endif