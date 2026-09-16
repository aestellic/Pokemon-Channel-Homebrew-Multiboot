#include "base_include.h"
#include "menu_text_handler.h"
#include "gen3_clock_events.h"
#include "text_handler.h"
#include "options_handler.h"
#include "input_handler.h"
#include "sprite_handler.h"
#include "version_identifier.h"
#include "print_system.h"
#include "sio_buffers.h"
#include "window_handler.h"
#include "config_settings.h"
#include <stddef.h>

#define NUM_LINES 10
#define MAIN_MENU_DISTANCE_FROM_BORDER 2

#define ROM_SIZE 0x1000000

#define SUMMARY_LINE_MAX_SIZE 18
#define PRINTABLE_INVALID_STRINGS 3

#if ENABLED_PRINT_INFO
#if ENABLED_LARGE_CRC_TABLE
const u32 poly8_lookup[256] =
{
 0, 0x77073096, 0xEE0E612C, 0x990951BA,
 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
#endif
#endif

u32 crc32b(u8* target, size_t size);
void print_basic_alter_conf_data(struct gen3_mon_data_unenc*, struct alternative_data_gen3*);
void print_pokemon_base_data(u8, struct gen3_mon_data_unenc*, u8, u8, u8);
void print_pokemon_base_info(u8, struct gen3_mon_data_unenc*, u8);
void print_bottom_info(void);
void print_pokemon_page1(struct gen3_mon_data_unenc*);
void print_pokemon_page2(struct gen3_mon_data_unenc*);
void print_pokemon_page3(struct gen3_mon_data_unenc*);
void print_pokemon_page4(struct gen3_mon_data_unenc*);
void print_pokemon_page5(struct gen3_mon_data_unenc*);
void print_evolution_animation_internal(struct gen3_mon_data_unenc*, u8);
const u8* get_language_string(u8);
void print_single_colour_info(u8);

const char* person_strings[] = {"You", "Other"};
const char* maingame_strings[] = {"RS", "FRLG", "E"};
const char* unidentified_string = "Unidentified";
const char* subgame_rs_strings[] = {"R", "S"};
const char* subgame_frlg_strings[] = {"FR", "LG"};
const char* game_strings[] = {"???", "Sapphire", "Ruby", "Emerald", "Fire Red", "Leaf Green"};
const char* region_strings[] = {"Int", "Jpn"};
const char* stat_strings[] = {"Hp", "Atk", "Def", "SpA", "SpD", "Spe"};
const char* contest_strings[] = {"Coolness", "Beauty", "Cuteness", "Smartness", "Toughness", "Feel"};
const char* language_strings[NUM_LANGUAGES+1] = {"???", "Japanese", "English", "French", "Italian", "German", "Korean", "Spanish", "Unknown"};
const char* trade_start_state_strings[] = {"Unknown", "Entering Room", "Starting Trade", "Ending Trade", "Waiting Trade", "Trading Party Data", "Synchronizing", "Completed"};
const char* offer_strings[] = {"Sending ", "Receiving"};
const char colours_chars[] = {'R', 'G', 'B'};
const char* invalid_strings[][PRINTABLE_INVALID_STRINGS] = {{"      TRADE REJECTED!", "The Pok\xE9mon offered by the", "other player has issues!"}, {"      TRADE REJECTED!", "The trade would leave you", "with no usable Pok\xE9mon!"}};

const u8 ribbon_print_pos[NUM_LINES*2] = {0,1,2,3,4,5,6,7,8,9,14,15,13,16,10,0xFF,11,0xFF,12,0xFF};
typedef void (*print_info_functions_t)(struct gen3_mon_data_unenc*);
print_info_functions_t print_info_functions[PAGES_TOTAL] = {print_pokemon_page1, print_pokemon_page2, print_pokemon_page3, print_pokemon_page4, print_pokemon_page5};

// calculate a checksum on a buffer -- start address = p, length = bytelength
#if ENABLED_PRINT_INFO
u32 crc32b(u8* target, size_t size)
#else
u32 crc32b(u8* UNUSED(target), size_t UNUSED(size))
#endif
{
    u32 crc = 0xffffffff;
    #if ENABLED_PRINT_INFO
    while (size-- !=0) {
        #if ENABLED_LARGE_CRC_TABLE
        crc = poly8_lookup[((u8) crc ^ *(target++))] ^ (crc >> 8);
        #else
        crc ^= *(target++);
        for(int i = 0; i < 8; i++)
        {
            u32 t = ~((crc & 1) - 1);
            crc = (crc >> 1) ^ (0xEDB88320 & t);
        }
        #endif
    }
    #endif
    return (crc ^ 0xffffffff);
}

void print_game_info(struct game_data_t* game_data, int index) {
    default_reset_screen();
    PRINT_FUNCTION("\n Game: ");
    const char* chosen_str = maingame_strings[game_data[index].game_identifier.game_main_version];
    switch(game_data[index].game_identifier.game_main_version) {
        case RS_MAIN_GAME_CODE:
            if(!game_data[index].game_identifier.game_sub_version_undetermined)
                chosen_str = subgame_rs_strings[game_data[index].game_identifier.game_sub_version];
            break;
        case FRLG_MAIN_GAME_CODE:
            if(!game_data[index].game_identifier.game_sub_version_undetermined)
                chosen_str = subgame_frlg_strings[game_data[index].game_identifier.game_sub_version];
            break;
        case E_MAIN_GAME_CODE:
            break;
        default:
            chosen_str = unidentified_string;
            break;
    }
    PRINT_FUNCTION("\x01\n", chosen_str);
    PRINT_FUNCTION("\n0x\x04\n", crc32b((u8*)ROM, ROM_SIZE));
    PRINT_FUNCTION("\n0x\x04\n", crc32b((u8*)EWRAM, EWRAM_SIZE));
    PRINT_FUNCTION("\n0x\x04\n", read_magic_number(0, 0));
    PRINT_FUNCTION("\n0x\x04\n", read_magic_number(1, 0));
    PRINT_FUNCTION("\n0x\x04\n", read_magic_number(1, 6));
    
    PRINT_FUNCTION("\nPress A to go back!");
}

void print_crash(enum CRASH_REASONS reason) {
    reset_screen(BLANK_FILL);
    
    init_crash_window();
    clear_crash_window();
    
    set_text_y(CRASH_WINDOW_Y);
    set_text_x(CRASH_WINDOW_X);
    PRINT_FUNCTION("      CRASHED!\n\n");
    set_text_x(CRASH_WINDOW_X);
    switch(reason) {
        case BAD_SAVE:
            PRINT_FUNCTION("ISSUES WHILE SAVING!\n\n");
            break;
        case BAD_TRADE:
            PRINT_FUNCTION("ISSUES WHILE TRADING!\n\n");
            break;
        case CARTRIDGE_REMOVED:
            PRINT_FUNCTION(" CARTRIDGE REMOVED!\n\n");
            break;
        default:
            break;
    }
    set_text_x(CRASH_WINDOW_X);
    PRINT_FUNCTION("TURN OFF THE CONSOLE.");
}

void print_swap_cartridge_menu() {
    default_reset_screen();

    PRINT_FUNCTION("Insert the new cartridge,\n\n");
    PRINT_FUNCTION("then press A to continue.\n\n");
}

void print_inject_jirachi_menu() {
    default_reset_screen();
    
    PRINT_FUNCTION("Press A to inject Jirachi.\n\n");
}

const u8* get_language_string(u8 language) {
    if(language > NUM_LANGUAGES)
        language = NUM_LANGUAGES;
    return (const u8*)language_strings[language];
}

void print_saving(){
    reset_screen(BLANK_FILL);
    init_saving_window();
    clear_saving_window();
    set_text_y(SAVING_WINDOW_Y);
    set_text_x(SAVING_WINDOW_X);
    PRINT_FUNCTION("Saving...");
}

void print_loading(){
    reset_screen(BLANK_FILL);
    init_loading_window();
    clear_loading_window();
    set_text_y(LOADING_WINDOW_Y);
    set_text_x(LOADING_WINDOW_X);
    PRINT_FUNCTION("Loading...");
}

void print_bottom_info(){
    set_text_y(Y_LIMIT-1);
    PRINT_FUNCTION("B: Go Back");
}


void print_load_warnings(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv) {
    default_reset_screen();

    set_text_x((X_LIMIT-8)>>1);
    PRINT_FUNCTION("WARNING!\n\n");
    if(!is_in_pokemon_center(game_data_priv, game_data->game_identifier.game_main_version)){
        PRINT_FUNCTION("The player did not save\n");
        PRINT_FUNCTION("in a Pok\xE9mon Center!\n\n");
    }
    if(game_data->party_3.total > (PARTY_SIZE - 1)){
        PRINT_FUNCTION("Party is full!\n");
        PRINT_FUNCTION("JIRACHI will replace 6th mon!\n\n");
    }
    if(can_trade(game_data_priv, game_data->game_identifier.game_main_version) == PARTIAL_TRADE_POSSIBLE) {
        PRINT_FUNCTION("This game has not acquired\n");
        PRINT_FUNCTION("a National Pokedex!\n\n");
    }
    if(can_trade(game_data_priv, game_data->game_identifier.game_main_version) == TRADE_IMPOSSIBLE) {
        PRINT_FUNCTION("This game has not acquired\n");
        PRINT_FUNCTION("a Pokedex!\n\n");
    }
    set_text_y(Y_LIMIT-3);
    PRINT_FUNCTION("  Proceed at your own risk!\n\n");
    PRINT_FUNCTION("Press any button to continue!");
}

void print_main_menu(u8 update, struct game_data_t* game_data, struct game_data_priv_t* game_data_priv) {
    if(!update)
        return;
    
    default_reset_screen();
    
    if(!get_valid_options_main()) {
        set_text_y(1);
        set_text_x(MAIN_MENU_DISTANCE_FROM_BORDER);
        if(game_data_priv->game_is_suspended)
            PRINT_FUNCTION("Error: Found Suspend data!");
        else if(!get_is_cartridge_loaded())
            PRINT_FUNCTION("Error reading the data!");
        else if(can_trade(game_data_priv, game_data->game_identifier.game_main_version) == TRADE_IMPOSSIBLE)
            PRINT_FUNCTION("Pok\xE9""dex not obtained!");
        else
            PRINT_FUNCTION("No valid Pok\xE9mon found!");
        set_text_y(3);
        set_text_x(MAIN_MENU_DISTANCE_FROM_BORDER);
        PRINT_FUNCTION("Load Cartridge");
    }
    else {
        set_text_y(1);
        set_text_x(MAIN_MENU_DISTANCE_FROM_BORDER);
        PRINT_FUNCTION("Inject Jirachi");
        set_text_y(3);
        set_text_x(MAIN_MENU_DISTANCE_FROM_BORDER);
        PRINT_FUNCTION("Swap Cartridge");
    }
}
