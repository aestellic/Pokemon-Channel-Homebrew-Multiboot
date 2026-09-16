#include "base_include.h"
#include "input_handler.h"
#include "sprite_handler.h"
#include "options_handler.h"
#include "gen3_clock_events.h"
#include "useful_qualifiers.h"
#include "config_settings.h"

u8 handle_input_main_menu(u8* cursor_y_pos, u16 keys, u8* update) {    
    switch(*cursor_y_pos) {
        case 0:
            if(keys & KEY_A && get_valid_options_main())
                return START_INJECT_JIRACHI;
            else if(keys & KEY_DOWN) {
                (*cursor_y_pos) = 1;
                (*update) = 1;
            }
            else if(keys & KEY_UP) {
                (*cursor_y_pos) = 1;
                (*update) = 1;
            }
            break;
        case 1:
            if(keys & KEY_A)
                return START_SWAP_CARTRIDGE;
            else if(keys & KEY_DOWN && get_valid_options_main()) {
                (*cursor_y_pos) = 0;
                (*update) = 0;
            }
            else if(keys & KEY_UP && get_valid_options_main()) {
                (*cursor_y_pos) = 0;
                (*update) = 0;
            }
            break;
        default:
            (*cursor_y_pos) = 0;
            (*update) = 0;
            break;
    }
    return 0;
}

u8 handle_input_swap_cartridge_menu(u16 keys) {
    if(keys & KEY_A)
        return 1;

    return 0;
}

u8 handle_input_inject_jirachi_menu(u16 keys) {
    if(keys & KEY_A)
        return 1;

    return 0;
}
