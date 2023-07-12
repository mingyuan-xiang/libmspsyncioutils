#include <include/mspsyncioutils.h>

static uint8_t ack;

static inline void wait_ack() {
  while (_RECVC() != ack)
    ;
  ack++;
}

/*
* Send a 16-bit number in big endian
*/
static inline size_t _PUT16(uint16_t num) {
  size_t ret = 0;
  ret += _PUTC((uint8_t)((num >> 8) & 0xFF));
  ret += _PUTC((uint8_t)(num & 0xFF));
  return ret;
}

/* send a matrix in a synchronized way */
static size_t msp_send_mat(mat_t* mat) {
  size_t ret = 0;
  // send the type
  ret += _PUTC(UARTIO_MSG_TYPE_MAT);
  // send the matrix shape
  ret += _PUT16(mat->len_dims);
  for (int i = 0; i < mat->len_dims; i++) {
    ret += _PUT16(mat->dims[i]);
  }
  // wait for acknowledgement for sent header
  wait_ack();

  // send the matrix data
  size_t size = mat_get_size(mat);
  size_t st = 0;
  size_t ed = UARTIO_16BIT_BUFFER_SIZE;
  while (size > UARTIO_16BIT_BUFFER_SIZE) {
    for (int i = st; i < ed; i++) {
      ret += _PUT16((uint16_t)(mat->data[i]));
    }
    // wait for acknowledgement for sent header
    wait_ack();

    st += UARTIO_16BIT_BUFFER_SIZE;
    ed += UARTIO_16BIT_BUFFER_SIZE;
    size -= UARTIO_16BIT_BUFFER_SIZE;
  }

  ed = mat_get_size(mat);
  for (int i = st; i < ed; i++) {
    ret += _PUT16((uint16_t)(mat->data[i]));
  }
  // wait for acknowledgement for sent header
  wait_ack();

  return ret;
}

/* send a byte array in a synchronized way */
static size_t msp_send_bytes(uint8_t* bytes, size_t len) {
  size_t ret = 0;
  // send the type
  ret += _PUTC(UARTIO_MSG_TYPE_BYTES);
  // send the array size
  ret += _PUT16(len);
  // wait for acknowledgement for sent header
  wait_ack();

  // send the data
  size_t size = len;
  uint8_t* ptr = bytes;
  while (size > UARTIO_BUFFER_SIZE) {
    ret += _PUT_BYTES(ptr, UARTIO_BUFFER_SIZE);
    // wait for acknowledgement for sent header
    wait_ack();

    ptr += UARTIO_BUFFER_SIZE
    size -= UARTIO_BUFFER_SIZE;
  }

  ret += _PUT_BYTES(ptr, size);
  // wait for acknowledgement for sent header
  wait_ack();

  return ret;
}

/* send a printf string in a synchronized way */
static size_t msp_send_printf(const char *format, ...) {
  size_t ret = 0;
  // send the type
  ret += _PUTC(UARTIO_MSG_TYPE_PRINTF_STR);
  // wait for acknowledgement for sent header
  wait_ack();

  va_list a;
  va_start(a, format);
  ret += _msp_port_vprintf(CONFIG_PRINT_PORT, format, a);
  va_end(a); 
  // wait for acknowledgement for sent header
  wait_ack();

  return ret;
}