#pragma once

void boot_set_vector_table(void);
__attribute__((noreturn)) void boot_jump_to_firmware(void);
