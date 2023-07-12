#ifndef INCLUDE_MSPSYNCIOUTILS_H
#define INCLUDE_MSPSYNCIOUTILS_H

#include <libmatAbstract/mat.h>
#include <libmspprintf/mspprintf.h>
#include <libmspio/uartio.h>
#include <mspioutils.h>

#include <stdarg.h>
#include <stdint.h>

/*
* This is a wrapper on libmspio library.
*
* To send a byte array or a string, the library will send a header first,
* which contains a 1-byte data type description (`uartio_msg.h`) and a few bytes for the
* data description (in big endian) in one message. Then, the library will send the data
* in one or multiple messages.
*
* The library can only receive byte array. It will received a 3-byte header first,
* with a 1-byte data type description and a 2-byte size. Then, the library will receive
* the data.
*
* The uart buffer size is defined as `UARTIO_BUFFER_SIZE` (in `uartio_msg.h`).
* Any array that is bigger than this will need to be sent/received in multiple message chunks.
*
* Each sent/received message needs to be communicated in a synchronized. In other words,
* each sent message will wait for a responding acknowledgement, and each received message
* will send an acknowledgement. The acknowledgement will be increased each time.
*/

#ifdef CONFIG_PRINT_PORT

#define _PUTC(c) PUTC(CONFIG_PRINT_PORT, c)
#define _PUT_BYTES(ptr, len) uartio_putbytes(CONFIG_PRINT_PORT, ptr, len)
#define _RECVC() uartio_getchar(CONFIG_PRINT_PORT)

/*
* send a matrix in a synchronized way
* The header message will be:
*   1. a type description byte (UARTIO_MSG_TYPE_MAT)
*   2. 2 bytes of dimension array size
*   3. 2*N bytes of sizes in each dimension, N is the dimension
*/
static size_t msp_send_mat(mat_t* mat);

/*
* send a byte array in a synchronized way
* The header message will be:
*   1. a type description byte (UARTIO_MSG_TYPE_BYTES)
*   2. 2 bytes array size
*/
static size_t msp_send_bytes(uint8_t* bytes, size_t len);

/*
* send a printf string in a synchronized way
* The header message will be:
*   1. a type description byte (UARTIO_MSG_TYPE_PRINTF_STR)
*/
static size_t msp_send_printf(const char *format, ...);

/* Print debug information
 * Formatting guidlines:
 * %c - Character
 * %s - String
 * %i - signed Integer (16 bit)
 * %u - Unsigned integer (16 bit)
 * %x - heXadecimal (16 bit)
 * %l - signed Long (32 bit)
 * %n - uNsigned loNg (32 bit)
 */
static inline int msp_print_debug(const char *format, ...) {
  va_list a;
  va_start(a, format);
  size_t ret = msp_send_printf(CONFIG_PRINT_PORT, format, a);
  va_end(a); 

  return ret;
}
#endif

#endif /* INCLUDE_MSPSYNCIOUTILS_H */