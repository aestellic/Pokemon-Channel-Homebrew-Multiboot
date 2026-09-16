#ifndef INPUT_HANDLER__
#define INPUT_HANDLER__

#include "party_handler.h"
#include "gen3_save.h"

#define START_PRINT_READ_INFO 0x57
#define START_SWAP_CARTRIDGE 0xA3
#define START_INJECT_JIRACHI 0xB5

#define PAGES_TOTAL 5
#define FIRST_PAGE 1

u8 handle_input_main_menu(u8*, u16, u8*);
u8 handle_input_swap_cartridge_menu(u16);
u8 handle_input_inject_jirachi_menu(u16);
u8 handle_input_print_read_info(u16);

#endif
