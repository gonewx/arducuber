#ifndef HOST_PGMSPACE_H
#define HOST_PGMSPACE_H

#include <stdint.h>

#define PROGMEM
#define pgm_read_byte_near(addr) (*(const uint8_t *)(addr))

#endif
