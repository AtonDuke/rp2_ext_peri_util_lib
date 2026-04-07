#include "serial_io_utils.h"

#include "pico/error.h"
#include "pico/stdio.h"

#include <stdio.h>
#include <string.h>

static int parse_character(serial_read_buffer_t *buffer, serial_read_settings_t settings, char c)
{
    switch(c)
    {
        case '\0': //Null
            return PICO_OK;
        case 0x01: //Start of Heading
        case 0x02: //Start of Text
        case 0x03: //End of Text
        case 0x04: //End of Transmission
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x05: //Enquiry
            if(settings.enquiry_callback != NULL)
            {
                settings.enquiry_callback();
            }
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x06: //Acknowledge
            if(settings.ack_nak_callback != NULL)
            {
                settings.ack_nak_callback(true);
            }
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case '\a': //Bell
            if(settings.bell_callback != NULL)
            {
                settings.bell_callback();
            }
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case '\b': //Backspace
            if(!settings.raw_input)
            {
                if(buffer->index > 0)
                {
                    --buffer->index;
                }
                if(settings.echo)
                {
                    stdio_putchar_raw('\b');
                }
                if(settings.del_on_backspace)
                {
                    buffer->data[buffer->index] = '\0';
                    if(settings.echo)
                    {
                        stdio_put_string(SERIAL_DEL_CTRL_SEQ, strlen(SERIAL_DEL_CTRL_SEQ), false, false);
                    }
                }
                return PICO_OK;
            }
            break;
        case '\t': //Horizontal Tab
            break;
        case '\n': //Line Feed
            if(settings.echo)
            {
                stdio_putchar_raw('\n');
            }
            buffer->data[buffer->index++] = '\n';
            buffer->data[buffer->index] = '\0';
            if(settings.strip_crlf_ending)
            {
                buffer->data[--buffer->index] = '\0';
                if(buffer->data[buffer->index - 1] == '\r')
                {
                    buffer->data[--buffer->index] = '\0';
                }
            }
            unsigned int data_length = buffer->index;
            buffer->index = 0;
            return data_length;
        case '\v': //Vertical Tab
        case '\f': //Form Feed
            break;
        case '\r': //Carriage Return
            break;
        case 0x0E: //Shift Out
        case 0x0F: //Shift In
        case 0x10: //Data Link Escape
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x11: //DC1, XON
            if(settings.xon_xoff_callback != NULL)
            {
                settings.xon_xoff_callback(true);
            }
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x12: //DC2, TAPE
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x13: //DC3, XOFF
            if(settings.xon_xoff_callback != NULL)
            {
                settings.xon_xoff_callback(false);
            }
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x14: //DC4, !TAPE
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x15: //Negative Acknowledge
            if(settings.ack_nak_callback != NULL)
            {
                settings.ack_nak_callback(false);
            }
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        case 0x16: //Synchronous Idle
        case 0x17: //End of Transmission Block
        case 0x18: //Cancel
        case 0x19: //End of Medium
        case 0x1A: //Substitute
        case '\e': //Escape
        case 0x1C: //File Separator
        case 0x1D: //Group Separator
        case 0x1E: //Record Separator
        case 0x1F: //Unit Separator
        case 0x7F: //Delete
            if(!settings.raw_input)
            {
                return PICO_OK;
            }
            break;
        default: //Should be only non-control characters
            break;
    }

    buffer->data[buffer->index++] = c;

    if(settings.echo)
    {
        stdio_putchar_raw(c);
    }

    if(buffer->index >= buffer->size - 1) //Max index value is size - 1 & the last character must be a Null
    {
        buffer->data[buffer->size - 1] = '\0'; //Make sure the last character is Null
        printf("\nBuffer overflow!\n"); //Preceeding newline, because this function terminates on newline & therefore we didn't receive it.
        return PICO_ERROR_GENERIC;
    }

    return PICO_OK;
}

int serial_read_line_blocking(serial_read_buffer_t *buffer, serial_read_settings_t settings)
{
    if(buffer == NULL) panic("Serial read buffer is NULL!\n");
    if(buffer->data == NULL) panic("Serial read buffer data pointer is NULL!\n");

	buffer->index = 0;
	while(true)
	{
		int c = stdio_getchar_timeout_us(0);
        if(c < 0)
        {
            continue;
        }

        int ret_val = parse_character(buffer, settings, c);
        if(ret_val != PICO_OK)
        {
            return ret_val;
        }
	}
}

int serial_read_line(serial_read_buffer_t *buffer, serial_read_settings_t settings)
{
    if(buffer == NULL) panic("Serial read buffer is NULL!\n");
    if(buffer->data == NULL) panic("Serial read buffer data pointer is NULL!\n");
    
    int ret_val = 0;
    do
    {
        int c = stdio_getchar_timeout_us(0);
        if(c < 0)
        {
            return PICO_OK;
        }

        ret_val = parse_character(buffer, settings, c);
    } while(ret_val <= 0);
    
    return ret_val;
}
