#include <firmware_info.h>

/* Needed to make room for the header, it will be filled by signing tool */
__attribute__((section(".fw_header"))) struct fw_header_t fw_header = {0};
