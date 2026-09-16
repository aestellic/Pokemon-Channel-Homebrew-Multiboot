#include "base_include.h"
#include "party_handler.h"
#include "options_handler.h"

void set_valid_options_main(struct game_data_t*, struct game_data_priv_t*);
void fill_options_main_array(struct game_data_t*, struct game_data_priv_t*);
gen3_party_total_t prepare_trade_options_num(u8*);
void fill_trade_options(u8*, struct game_data_t*, u8);

u8 options_main[MAIN_OPTIONS_NUM];
u8 valid_options_main;
u8 options_trade[2][PARTY_SIZE];
u8 num_options_trade[2];

u8 is_valid_for_gen(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv, u8 gen) {
    if((!get_is_cartridge_loaded()) || (can_trade(game_data_priv, game_data->game_identifier.game_main_version) == TRADE_IMPOSSIBLE))
        return 0;
    u8 found_size = get_party_usable_num(game_data);
    if(!found_size)
        return 0;
    if(gen == 1)
        return (game_data_priv->party_1.total > 0);
    if(gen == 2)
        return (game_data_priv->party_2.total > 0);
    return 1;
}

void set_valid_options_main(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv) {
    valid_options_main = (is_valid_for_gen(game_data, game_data_priv, 1) ? 1: 0) | (is_valid_for_gen(game_data, game_data_priv, 2) ? 2: 0) | (is_valid_for_gen(game_data, game_data_priv, 3) ? 4: 0);
}

void fill_options_main_array(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv) {
    u8 curr_slot = MAIN_OPTIONS_NO_OPTION;
    for(int i = 0; i < MAIN_OPTIONS_NUM; i++)
        options_main[i] = 0;
    if(is_valid_for_gen(game_data, game_data_priv, 1)) {
        options_main[curr_slot++] = 1;
        for(int i = curr_slot; i < MAIN_OPTIONS_NUM; i++)
            options_main[i] = 1;
    }
    if(is_valid_for_gen(game_data, game_data_priv, 2)) {
        options_main[curr_slot++] = 2;
        for(int i = curr_slot; i < MAIN_OPTIONS_NUM; i++)
            options_main[i] = 2;
    }
    if(is_valid_for_gen(game_data, game_data_priv, 3)) {
        options_main[curr_slot++] = 3;
        for(int i = curr_slot; i < MAIN_OPTIONS_NUM; i++)
            options_main[i] = 3;
    }
}

void prepare_main_options(struct game_data_t* game_data, struct game_data_priv_t* game_data_priv) {
    fill_options_main_array(game_data, game_data_priv);
    set_valid_options_main(game_data, game_data_priv);
}

u8 get_valid_options_main() {
    return valid_options_main;
}

u8* get_options_main() {
    return options_main;
}

u8 get_number_of_higher_ordered_options(u8* options, u8 limit) {
    u8 curr_num = 0;
    u8 highest_found = 0;
    for(int i = 0; i < limit; i++)
        if(options[i] > highest_found) {
            curr_num++;
            highest_found = options[i];
        }
    return curr_num;
}

u8 get_number_of_lower_ordered_options(u8* options, u8 limit) {
    u8 curr_num = 0;
    u8 lowest_found = 0;
    u8 start = limit-1;
    if(limit) 
        for(int i = start; i >= 0; i--)
            if(options[i] < lowest_found) {
                curr_num++;
                lowest_found = options[i];
            }
    return curr_num;
}
