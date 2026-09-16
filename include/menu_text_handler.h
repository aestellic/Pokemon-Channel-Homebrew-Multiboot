#ifndef MENU_TEXT_HANDLER__
#define MENU_TEXT_HANDLER__

#include "party_handler.h"
#include "gen3_save.h"
#include "multiboot_handler.h"
#include "config_settings.h"

enum MOVES_PRINTING_TYPE{LEARNT_P, DID_NOT_LEARN_P, LEARNABLE_P};
enum CRASH_REASONS{BAD_SAVE, BAD_TRADE, CARTRIDGE_REMOVED};

void print_game_info(struct game_data_t*, int);
void print_crash(enum CRASH_REASONS);
void print_main_menu(u8, struct game_data_t*, struct game_data_priv_t*);
void print_saving(void);
void print_loading(void);
void print_swap_cartridge_menu(void);
void print_inject_jirachi_menu(void);
void print_load_warnings(struct game_data_t*, struct game_data_priv_t*);

#endif
