#include <string.h>
#include "base_include.h"
#include "multiboot_handler.h"
#include "graphics_handler.h"
#include "party_handler.h"
#include "text_handler.h"
#include "sprite_handler.h"
#include "version_identifier.h"
#include "gen3_save.h"
#include "gen3_clock_events.h"
#include "options_handler.h"
#include "input_handler.h"
#include "menu_text_handler.h"
#include "sio_buffers.h"
#include "rng.h"
#include "pid_iv_tid.h"
#include "print_system.h"
#include "window_handler.h"
#include "sio.h"
#include "animations_handler.h"
#include <stddef.h>
#include "optimized_swi.h"
#include "timing_basic.h"
#include "config_settings.h"
//#include "save.h"

#include "ewram_speed_check_bin.h"

#define REG_MEMORY_CONTROLLER_ADDR 0x4000800
#define HW_SET_REG_MEMORY_CONTROLLER_VALUE 0x0D000020
#define REG_MEMORY_CONTROLLER *((u32*)(REG_MEMORY_CONTROLLER_ADDR))

#define WORST_CASE_EWRAM 1
#define TEST_LAG_EWRAM 0
#define MIN_WAITCYCLE 1

#define WAITING_TIME_MOVE_MESSAGES (2*FPS)
#define WAITING_TIME_BEFORE_RESPONSE (1*FPS)
#define WAITING_TIME_REJECTED (2*FPS)
#define MAX_RANDOM_WAIT_TIME (1*FPS)

#define BASE_SCREEN 0

#define LCRNG_GLIBC(val) (1103515245 * (val) + 24691)   // gba
#define LCRNG_MSVC(val) (214013 * (val) + 2531011)      // gamecube

__attribute__((section("common_data"))) u32 GLIBC_RngValue = 0;
__attribute__((section("common_data"))) u32 MSVC_RngValue = 0;

void vblank_update_function(void);
void find_optimal_ewram_settings(void);
void disable_all_irqs(void);
u8 init_cursor_y_pos_main_menu(void);
void cursor_update_main_menu(u8);
void change_nature(struct game_data_t*, u8, u8, u8*, u8);
void saving_print_screen(u8);
void loading_print_screen(void);
void waiting_init(s8);
void invalid_init(u8);
void trade_options_init(u8, u8*, u8, u8);
void main_menu_init(struct game_data_t*, struct game_data_priv_t*, u8*);
void info_menu_init(struct game_data_t*, u8, u8, u8*, u8);
void nature_menu_init(struct game_data_t*, u8, u8, u8*);
void load_warnings_menu_init(struct game_data_t*, struct game_data_priv_t*);
void prepare_crash_screen(enum CRASH_REASONS);
void crash_on_cartridge_removed(void);
void crash_on_bad_save(void);
void wait_frames(int);
void complete_save_menu(struct game_data_t*, struct game_data_priv_t*);
void complete_cartridge_loading(struct game_data_t*, struct game_data_priv_t*, u8*);
u16 RandomGBA(void);
u16 RandomGC(void);
void SeedGBA_Rng(u16);
void SeedGC_Rng(u16);
struct gen3_mon_data_unenc* channel_jirachi(u16, struct game_data_t*);
void inject_jirachi(struct game_data_t*, struct game_data_priv_t*, u8*);
int main(void);

enum STATE {MAIN_MENU, SWAP_CARTRIDGE_MENU, INJECT_JIRACHI_MENU, WARNINGS_WHEN_LOADING, PRINT_READ_INFO};
enum STATE curr_state;
u32 counter = 0;
u32 input_counter = 0;

IWRAM_CODE void vblank_update_function() {
    REG_IF |= IRQ_VBLANK;

    flush_screens();

    if(has_cartridge_been_removed())
        crash_on_cartridge_removed();

    move_sprites(counter);
    move_cursor_x(counter);
    advance_rng();
    counter++;
}

IWRAM_CODE void find_optimal_ewram_settings() {
    #ifdef __GBA__
    size_t size = ewram_speed_check_bin_size>>2;
    const u32* ewram_speed_check = (const u32*)ewram_speed_check_bin;
    u32 test_data[size];
    
    // Check for unsupported (DS)
    if(REG_MEMORY_CONTROLLER != HW_SET_REG_MEMORY_CONTROLLER_VALUE)
        return;
    
    if(TEST_LAG_EWRAM)
        REG_MEMORY_CONTROLLER &= ~(0xF<<24);
    
    // Check for worst case testing (Not for final release)
    if(WORST_CASE_EWRAM)
        return;
    
    // Prepare data to test against
    for(size_t i = 0; i < size; i++)
        test_data[i] = ewram_speed_check[i];
    
    // Detetmine minimum number of stable waitcycles
    for(int i = 0; i < (16-MIN_WAITCYCLE); i++) {
        REG_MEMORY_CONTROLLER &= ~(0xF<<24);
        REG_MEMORY_CONTROLLER |= (15-i-MIN_WAITCYCLE)<<24;
        u8 failed = 0;
        for(size_t j = 0; (!failed) && (j < size); j++)
            if(test_data[j] != ewram_speed_check[j])
                failed = 1;
        if(!failed)
            return;
    }
    #endif
}

void disable_all_irqs() {
    REG_IME = 0;
    REG_IE = 0;
}

u8 init_cursor_y_pos_main_menu() {
    if(!get_valid_options_main())
        return 1;
    return 0;
}

void cursor_update_main_menu(u8 cursor_y_pos) {
    update_cursor_y(BASE_Y_CURSOR_MAIN_MENU + (BASE_Y_CURSOR_INCREMENT_MAIN_MENU * cursor_y_pos));
}

void wait_frames(int num_frames) {
    int wait_counter = 0;
    while(wait_counter < num_frames) {
        VBlankIntrWait();
        wait_counter++;
    }
}

void loading_print_screen() {
    u8 prev_screen = get_screen_num();
    set_screen(LOADING_WINDOW_SCREEN);
    print_loading();
    enable_screen(LOADING_WINDOW_SCREEN);
    set_screen(prev_screen);
    prepare_flush();
}

void main_menu_init(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv, u8* cursor_y_pos) {
    curr_state = MAIN_MENU;
    prepare_main_options(game_data, game_data_priv);
    set_screen(BASE_SCREEN);
    set_bg_pos(BASE_SCREEN, 0, 0);
    print_main_menu(1, game_data, game_data_priv);
    disable_all_screens_but_current();
    reset_sprites_to_cursor(1);
    disable_all_cursors();
    *cursor_y_pos = init_cursor_y_pos_main_menu();
    update_cursor_base_x(BASE_X_CURSOR_MAIN_MENU);
    cursor_update_main_menu(*cursor_y_pos);
    enable_screen(BASE_SCREEN);
    prepare_flush();
}

void load_warnings_menu_init(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv) {
    curr_state = WARNINGS_WHEN_LOADING;
    set_screen(BASE_SCREEN);
    set_bg_pos(BASE_SCREEN, 0, 0);
    print_load_warnings(game_data, game_data_priv);
    disable_all_screens_but_current();
    reset_sprites_to_cursor(1);
    disable_all_cursors();
    enable_screen(BASE_SCREEN);
    prepare_flush();
}

void prepare_crash_screen(enum CRASH_REASONS reason) {
    set_screen(BASE_SCREEN);
    default_reset_screen();
    enable_screen(BASE_SCREEN);
    reset_sprites_to_cursor(1);
    disable_all_screens_but_current();
    disable_all_cursors();
    set_screen(CRASH_WINDOW_SCREEN);
    print_crash(reason);
    enable_screen(CRASH_WINDOW_SCREEN);
    prepare_flush();
    #if defined(__NDS__) && (!defined(__BLOCKSDS__))
    pmMainLoop();
    #endif
}

void crash_on_cartridge_removed() {
    prepare_crash_screen(CARTRIDGE_REMOVED);
    int curr_vcount = REG_VCOUNT + 1 + 1;
    if(curr_vcount >= SCANLINES)
        curr_vcount -= SCANLINES;
    while(REG_VCOUNT != curr_vcount);
    while(REG_VCOUNT != VBLANK_SCANLINES);
    disable_sprites_rendering();
    flush_screens();
    disable_all_irqs();
    while(1)
        Halt();
}

void crash_on_bad_save() {
    prepare_crash_screen(BAD_SAVE);
    VBlankIntrWait();
    disable_all_irqs();
    while(1)
        Halt();
}

void complete_save_menu(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv) {
    u8 success = complete_write_gen_3_data(game_data);
    if(!success)
        crash_on_bad_save();
    process_party_data(game_data, &game_data_priv->party_2, &game_data_priv->party_1);
}

void complete_cartridge_loading(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv, u8* cursor_y_pos) {
    init_game_data(game_data);
    u8 can_check_cart = 1;
    #ifdef __NDS__
    if(isDSiMode())
        can_check_cart = 0;
    #endif
    if(can_check_cart) {
        get_game_id(&game_data->game_identifier);
        read_gen_3_data(game_data, game_data_priv);
    }
    prepare_main_options(game_data, game_data_priv);
    if((!get_valid_options_main()) || (!loaded_data_has_warnings(game_data, game_data_priv)))
        main_menu_init(game_data, game_data_priv, cursor_y_pos);
    else
        load_warnings_menu_init(game_data, game_data_priv);
}

u16 RandomGBA(void)
{
    GLIBC_RngValue = LCRNG_GLIBC(GLIBC_RngValue);
    return GLIBC_RngValue >> 16;
}
u16 RandomGC(void)
{
    MSVC_RngValue = LCRNG_MSVC(MSVC_RngValue);
    return MSVC_RngValue >> 16;
}

void SeedGBA_Rng(u16 seed)
{
    GLIBC_RngValue = seed;
}

void SeedGC_Rng(u16 seed)
{
    MSVC_RngValue = seed;
}

struct gen3_mon_data_unenc* channel_jirachi(u16 seed, struct game_data_t* game_data) {
    struct gen3_mon dst;
    struct gen3_mon_data_unenc temp;
    struct gen3_mon_data_unenc* data_dst = &temp;
    data_dst->src = &dst;
    data_dst->successfully_decrypted = 0;
    data_dst->learnable_moves = NULL;
    SeedGC_Rng(seed);

    u8 menu = 0;
    while (menu != 14)
        menu |= (1 << (RandomGC()>>14))&14;

    for (u8 i=0; i<5; i++) {
        RandomGC();
    }
    if ((MSVC_RngValue >> 16) <= 0x4000)
        RandomGC();
    else {
        if (RandomGC() <= 0x547A)
            RandomGC();
        else {
            RandomGC();
            RandomGC();
        }
    }

    u16 tid = 40122;
    u16 sid = RandomGC();
    u16 upperPID = RandomGC();
    u16 lowerPID = RandomGC();
    if (tid ^ upperPID ^ sid ^ (lowerPID < 8))
        upperPID ^= 0x8000;
    u32 pid = (upperPID<<16)|lowerPID;

    u8 berry;
    u8 origin_game;
    u8 ot_gender;
    if ((RandomGC()>>15) == 1)
        berry = 0; // salac
    else
        berry = 1; // ganlon

    if ((RandomGC()>>15) == 1)
        origin_game = 2; // ruby
    else
        origin_game = 1; // sapphire

    if ((RandomGC()>>15) == 1)
        ot_gender = 1; // female
    else
        ot_gender = 0; // male

    u8 ball = 4; // pokeball
    u8 met_level = 0; // for some reason its illegal without this lmao
    u8 ot_name[OT_NAME_GEN3_MAX_SIZE] = {0xBD, 0xC2, 0xBB, 0xC8, 0xC8, 0xBF, 0xC6}; // CHANNEL
    u8 nickname[NICKNAME_GEN3_MAX_SIZE] = {0xC4, 0xC3, 0xCC, 0xBB, 0xBD, 0xC2, 0xC3, 0xFF}; // JIRACHI
    u8 level = 5;
    u32 ot_id = (tid<<0) | (sid<<16);
    u8 mail_id = GEN3_NO_MAIL;
    u8 marks = 0;
    u8 is_bad_egg = 0;
    u8 has_species = 1;
    u8 use_egg_name = 0;
    u8 language;
    if (game_data->game_identifier.language == JAPANESE_LANGUAGE)
        language = ENGLISH_LANGUAGE;
    else
        language = game_data->game_identifier.language;
    u8 block_box_rs = 0;

    // Reset everything
    for(size_t i = 0; i < sizeof(struct gen3_mon_growth); i++)
        ((u8*)(&data_dst->growth))[i] = 0;
    for(size_t i = 0; i < sizeof(struct gen3_mon_attacks); i++)
        ((u8*)(&data_dst->attacks))[i] = 0;
    for(size_t i = 0; i < sizeof(struct gen3_mon_evs); i++)
        ((u8*)(&data_dst->evs))[i] = 0;
    for(size_t i = 0; i < sizeof(struct gen3_mon_misc); i++)
        ((u8*)(&data_dst->misc))[i] = 0;

    // Set species, exp, level and item

    data_dst->growth.species = 409; // jirachi
    // data_dst.growth.item = ganlon or salac berry;
    if (berry == 0)
        data_dst->growth.item = 170; // salac berry
    else 
        data_dst->growth.item = 169; // ganlon berry
    data_dst->growth.friendship = BASE_FRIENDSHIP;
    data_dst->growth.exp = 156; // level 5
    data_dst->attacks.moves[0] = 273; // wish
    data_dst->attacks.pp[0] = 10;
    data_dst->attacks.moves[1] = 93;  //confusion
    data_dst->attacks.pp[1] = 25;
    data_dst->attacks.moves[2] = 156; // rest
    data_dst->attacks.pp[2] = 5;
    data_dst->attacks.moves[3] = 0;   // null
    data_dst->attacks.pp[3] = 0;
    data_dst->misc.pokerus = 0;
    data_dst->misc.origins_info = ((ot_gender&1)<<15) | ((ball&0xF)<<11) | ((origin_game&0xF)<<7) | ((met_level&0x7F)<<0);
    data_dst->misc.obedience = 0;
    data_dst->misc.met_location = 0xFF;
    data_dst->misc.ability = 0;
    data_dst->misc.ribbons = 0;

    data_dst->misc.hp_ivs = RandomGC()>>11;
    data_dst->misc.atk_ivs = RandomGC()>>11;
    data_dst->misc.def_ivs = RandomGC()>>11;
    data_dst->misc.spe_ivs = RandomGC()>>11;
    data_dst->misc.spa_ivs = RandomGC()>>11;
    data_dst->misc.spd_ivs = RandomGC()>>11;
    data_dst->evs.evs[0] = 0;
    data_dst->evs.evs[1] = 0;
    data_dst->evs.evs[2] = 0;
    data_dst->evs.evs[3] = 0;
    data_dst->evs.evs[4] = 0;
    data_dst->evs.evs[5] = 0;

    dst.pid = pid;
    dst.ot_id = ot_id;
    dst.level = level;
    dst.marks = marks;
    dst.mail_id = mail_id;
    dst.is_bad_egg = is_bad_egg;
    dst.has_species = has_species;
    dst.use_egg_name = use_egg_name;
    dst.language = language;
    dst.block_box_rs = block_box_rs;
    dst.unused2 = 0;
    memcpy(&dst.nickname[0],&nickname,sizeof(nickname));
    memcpy(&dst.ot_name[0],&ot_name,sizeof(ot_name));
    
    place_and_encrypt_gen3_data(data_dst, &dst);
    recalc_stats_gen3(data_dst, &dst);
    
    return data_dst;
}

void inject_jirachi(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv, u8* cursor_y_pos) {
    u16 seed = 0;
    u8 party_slot = get_new_party_entry_index(&game_data[0]); // validate party size < 6 before doing this!!!
    u8 success;
    replace_party_entry(&game_data[0], channel_jirachi(seed, game_data), party_slot);

    success = pre_write_gen_3_data(&game_data[0], game_data_priv, 1);
    if(!success)
        crash_on_bad_save();
    complete_save_menu(&game_data[0], game_data_priv);
    main_menu_init(&game_data[0], game_data_priv, cursor_y_pos);
}

int main(void)
{
    #ifdef __GBA__
    RegisterRamReset(RESET_SIO|RESET_SOUND|RESET_OTHER);
    disable_all_irqs();
    #else
    #ifndef __BLOCKSDS__
    gbacartOpen();
    #endif
    #endif
    curr_state = MAIN_MENU;
    counter = 0;
    input_counter = 0;
    find_optimal_ewram_settings();
    set_default_settings();
    init_text_system();
    init_enc_positions();
    init_rng(0,0);
    init_save_data();
    u32 keys;
    struct game_data_t game_data[2];
    struct game_data_priv_t game_data_priv;    
    init_sprites();
    init_oam_palette();
    init_sprite_counter();
    enable_sprites_rendering();
    init_numbers();
    
    init_unown_tsv();
    #ifdef HAS_SIO
    sio_stop_irq_slave();
    #endif
    #ifdef __GBA__
    irqInit();
    #endif
    irqSet(IRQ_VBLANK, vblank_update_function);
    irqEnable(IRQ_VBLANK);
    
    init_item_icon();
    init_cursor();
    
    u8 returned_val;
    u8 update = 0;
    u8 cursor_y_pos = 0;
    complete_cartridge_loading(&game_data[0], &game_data_priv, &cursor_y_pos);
    
    //load_pokemon_sprite_raw(&game_data[1].party_3_undec[0], 1, 0, 0);
    //worst_case_conversion_tester(&counter);
    //PRINT_FUNCTION("\n\n0x\x0D: 0x\x0D\n", REG_MEMORY_CONTROLLER_ADDR, 8, REG_MEMORY_CONTROLLER, 8);
    scanKeys();
    keys = keysDown();

    while(1) {
        
        do {
            #if defined(__NDS__) && (!defined(__BLOCKSDS__))
            pmMainLoop();
            #endif
            prepare_flush();
            VBlankIntrWait();
            scanKeys();
            keys = keysDown();
            if (curr_state == WARNINGS_WHEN_LOADING) {
                if(keys)
                    main_menu_init(&game_data[0], &game_data_priv, &cursor_y_pos);
                keys = 0;
            }
        } while ((!(keys & KEY_LEFT)) && (!(keys & KEY_RIGHT)) && (!(keys & KEY_A)) && (!(keys & KEY_B)) && (!(keys & KEY_UP)) && (!(keys & KEY_DOWN)));
        
        input_counter++;
        switch(curr_state) {
            case MAIN_MENU:
                returned_val = handle_input_main_menu(&cursor_y_pos, keys, &update);
                print_main_menu(update, &game_data[0], &game_data_priv);
                cursor_update_main_menu(cursor_y_pos);
                if(returned_val == START_SWAP_CARTRIDGE) {
                    curr_state = SWAP_CARTRIDGE_MENU;
                    disable_cursor();
                    init_save_data();
                    print_swap_cartridge_menu();
                }
                else if(returned_val == START_INJECT_JIRACHI) {
                    curr_state = INJECT_JIRACHI_MENU;
                    disable_cursor();
                    init_save_data();
                    print_inject_jirachi_menu();
                }
                break;
            case SWAP_CARTRIDGE_MENU:
                returned_val = handle_input_swap_cartridge_menu(keys);
                if(returned_val) {
                    loading_print_screen();
                    complete_cartridge_loading(&game_data[0], &game_data_priv, &cursor_y_pos);
                }
                break;
            case INJECT_JIRACHI_MENU:
                returned_val = handle_input_inject_jirachi_menu(keys);
                if(returned_val) {
                    loading_print_screen();
                    inject_jirachi(&game_data[0], &game_data_priv, &cursor_y_pos);
                }
                break;
            case WARNINGS_WHEN_LOADING:
                main_menu_init(&game_data[0], &game_data_priv, &cursor_y_pos);
                break;
            default:
                main_menu_init(&game_data[0], &game_data_priv, &cursor_y_pos);
                break;
        }
        update = 0;
    }

    return 0;
}
