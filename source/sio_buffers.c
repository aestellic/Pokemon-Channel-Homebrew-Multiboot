#include "base_include.h"
#include "sio_buffers.h"
#include "text_handler.h"
#include "options_handler.h"
#include "party_handler.h"
#include "gen3_save.h"
#include "config_settings.h"
#include <stddef.h>

#include "default_gift_ribbons_bin.h"

void prepare_number_of_sizes(void);
u8 get_number_of_buffers(void);
void copy_bytes(const void*, void*, size_t, u8, u8);
void prepare_random_data_gen12(struct random_data_t*);
size_t check_patch_set_limit(u8*, size_t, size_t);
void prepare_patch_set(u8*, u8*, size_t, size_t, size_t, size_t);
void apply_patch_set(u8*, u8*, size_t, size_t, size_t, size_t);

u32 communication_buffers[2][BUFFER_SIZE>>2];
size_t buffer_sizes[NUM_SIZES];
u8 number_of_sizes;

u32* get_communication_buffer(u8 requested) {
    if(!requested)
        return communication_buffers[OWN_BUFFER];
    return communication_buffers[OTHER_BUFFER];
}

void prepare_number_of_sizes() {
    number_of_sizes = 0;
    for(int i = 0; i < NUM_SIZES; i++)
        if(buffer_sizes[i] != SIZE_STOP)
            number_of_sizes++;
        else
            return;
}

IWRAM_CODE u8 get_number_of_buffers() {
    return number_of_sizes;
}

size_t* get_buffer_sizes() {
    return buffer_sizes;
}

size_t get_buffer_size(int index) {
    if((index < 0) || (index >= number_of_sizes))
        index = 0;
    return buffer_sizes[index];
}

void copy_bytes(const void* src, void* dst, size_t size, u8 src_offset, u8 dst_offset) {
    const u8* src_data = (const u8*)src;
    u8* dst_data = (u8*)dst;
    for(size_t i = 0; i < size; i++)
        dst_data[dst_offset+i] = src_data[src_offset+i];
}

void prepare_random_data_gen12(struct random_data_t* random_data) {
    for(size_t i = 0; i < RANDOM_DATA_SIZE; i++)
        random_data->data[i] = DEFAULT_FILLER;
}

size_t check_patch_set_limit(u8* patch_set_buffer, size_t cursor_data, size_t patch_set_size) {
    if(cursor_data >= patch_set_size) {
        cursor_data = patch_set_size - 1;
        patch_set_buffer[cursor_data] = 0xFF;
    }
    return cursor_data;
}

void prepare_patch_set(u8* buffer, u8* patch_set_buffer, size_t size, size_t start_pos, size_t patch_set_size, size_t base_pos) {
    size_t cursor_data = base_pos;
    
    for(size_t i = 0; i < patch_set_size; i++)
        patch_set_buffer[i] = 0;

    if(!patch_set_size)
        return;

    u32 base = start_pos;
    for(size_t i = 0; i < size; i++) {
        if((start_pos + i - base) == (NO_ACTION_BYTE-2)) {
            base += NO_ACTION_BYTE-2;
            patch_set_buffer[cursor_data++] = 0xFF;
            cursor_data = check_patch_set_limit(patch_set_buffer, cursor_data, patch_set_size);
        }
        if(buffer[start_pos + i] == NO_ACTION_BYTE) {
            buffer[start_pos + i] = 0xFF;
            patch_set_buffer[cursor_data++] = start_pos + i + 1 -base;
            cursor_data = check_patch_set_limit(patch_set_buffer, cursor_data, patch_set_size);
        }
    }

    if((size+start_pos-base) > 0)
        patch_set_buffer[cursor_data] = 0xFF;
}

void apply_patch_set(u8* buffer, u8* patch_set_buffer, size_t size, size_t start_pos, size_t patch_set_size, size_t base_pos) {    
    size_t base = 0;
    for(size_t i = base_pos; i < patch_set_size; i++) {
        if(patch_set_buffer[i]) {
            if(patch_set_buffer[i] == 0xFF) {
                base += NO_ACTION_BYTE-2;
                if(base >= size)
                    return;
            }
            else if(patch_set_buffer[i] <= NO_ACTION_BYTE-2)
                if((patch_set_buffer[i]+base-1) < size)
                    buffer[patch_set_buffer[i]+start_pos+base-1] = 0xFE;
        }
    }
}

u8 are_checksum_same_gen3(struct gen3_trade_data* td) {
    u32 checksum = 0;
    for(gen3_party_total_t i = 0; i < PARTY_SIZE; i++) {
        u32* mail_buf = (u32*)&td->mails_3[i];
        for(size_t j = 0; j < (sizeof(struct mail_gen3)>>2); j++)
            checksum += mail_buf[j];
    }
    
    if(td->checksum_mail != checksum)
        return 0;

    checksum = 0;
    
    u32* party_buf = (u32*)&td->party_3;
    for(size_t i = 0; i < (sizeof(struct gen3_party)>>2); i++)
        checksum += party_buf[i];
    
    if(td->checksum_party != checksum)
        return 0;
    
    checksum = 0;
    u32* buffer = (u32*)td;
    
    for(size_t i = 0; i < ((sizeof(struct gen3_trade_data) - 4)>>2); i++)
        checksum += buffer[i];
    
    if(td->final_checksum != checksum)
        return 0;
    
    return 1;
}
