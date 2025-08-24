#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STUI_IMPLEMENTATION
#include "stui.h"

void draw_text_len(size_t x, size_t y, char* text, int n, size_t width, size_t height, size_t* curOut_x, size_t* curOut_y){
    size_t cur_x = x;
    size_t cur_y = y;
    for(int i = 0; i < n; i++){
        if(cur_y >= height) break;
        if(cur_x >= width){
            cur_x = 0;
            cur_y++;
        }
        if(text[i] != '\n'){
            stui_putchar(cur_x, cur_y, text[i]);
            cur_x++;
        }else{
            cur_x = 0;
            cur_y++;
        }
    }
    if(curOut_x) *curOut_x = cur_x;
    if(curOut_y) *curOut_y = cur_y;
}

void draw_text(size_t x, size_t y, char* text, size_t width, size_t height, size_t* curOut_x, size_t* curOut_y){
    draw_text_len(x,y,text,strlen(text),width,height, curOut_x, curOut_y);
}

#define GAP_DEFAULT_SIZE 16

typedef struct{
   char* items;
   size_t count;
   size_t cap;

   size_t gap_begin;
   size_t gap_end;
} GapBuffer;

void GapBuffer_init(char* text, size_t n, GapBuffer* buf){
    buf->cap = GAP_DEFAULT_SIZE + n;
    buf->items = realloc(buf->items, buf->cap);
    buf->count = n;
    buf->gap_begin = 0;
    buf->gap_end = GAP_DEFAULT_SIZE;
    memcpy(buf->items+buf->gap_end, text, n);
}

void GapBuffer_get_strs(GapBuffer* buf, char** out1,  size_t* n1, char** out2, size_t* n2){
    *out1 = buf->items;
    *n1 = buf->gap_begin;
    *out2 = buf->items + buf->gap_end;
    *n2 = buf->cap - buf->gap_end;
}

void GapBuffer_insert_char(GapBuffer* buf, char c){
    if(buf->gap_end - buf->gap_begin == 0){
        size_t old_end_size = buf->cap - buf->gap_end;
        buf->cap+= GAP_DEFAULT_SIZE;
        buf->items = realloc(buf->items, buf->cap);
        buf->gap_end += GAP_DEFAULT_SIZE;
        memmove(buf->items + buf->gap_end, buf->items + buf->gap_begin, old_end_size);
    }
    buf->items[buf->gap_begin++] = c;
    buf->count++;
}

void GapBuffer_backspace(GapBuffer* buf){
    if(buf->gap_begin) {
        buf->gap_begin--;
        buf->count--;
    }
}

void GapBuffer_delete(GapBuffer* buf){
    if(buf->gap_end < buf->cap) {
        buf->gap_end++;
        buf->count--;
    }
}

void GapBuffer_left(GapBuffer* buf){
    if(buf->gap_begin == 0) return;
    buf->items[--buf->gap_end] = buf->items[--buf->gap_begin];
}

void GapBuffer_right(GapBuffer* buf){
    if(buf->gap_end >= buf->cap) return;
    buf->items[buf->gap_begin++] = buf->items[buf->gap_end++];
}

int main(){
    stui_term_disable_echo();
    stui_term_enable_instant();


    size_t width = 0, height = 0;
    stui_term_get_size(&width, &height);
    stui_setsize(width, height);

    GapBuffer text = {0};
    char textPtr[] = "Initial text for gap buffer!";
    GapBuffer_init(textPtr, strlen(textPtr), &text);

    char buf[256];


    stui_clear();
    for(;;){
        for(size_t y = 0; y < height; ++y) {
            for(size_t x = 0; x < width; ++x) {
                stui_putchar(x, y, ' ');
            }
        }
        size_t anchor_x = 0;
        size_t anchor_y = 0;

        snprintf(buf, sizeof(buf), "----------------\nGap Buff stats:\ncount: %zu\ncap: %zu\ngap_begin: %zu\ngap_end: %zu\n----------------\n", text.count, text.cap, text.gap_begin, text.gap_end);
        draw_text(anchor_x, anchor_y, buf, width, height, &anchor_x, &anchor_y);

        char* text1 = NULL;
        char* text2 = NULL;
        size_t n1 = 0;
        size_t n2 = 0;
        GapBuffer_get_strs(&text, &text1, &n1, &text2, &n2);

        snprintf(buf, sizeof(buf), "0x%p 0x%p:%zu 0x%p:%zu\n",text.items, text1, n1, text2, n2);
        draw_text(anchor_x, anchor_y, buf, width, height, &anchor_x, &anchor_y);

        size_t cur_x = anchor_x, cur_y = anchor_y;
        if(n1) draw_text_len(anchor_x, anchor_y, text1, n1, width, height, &cur_x, &cur_y);
        if(n2) draw_text_len(cur_x, cur_y, text2, n2, width, height, NULL, NULL);

        stui_refresh();
        if(n1) stui_goto(cur_x, cur_y);
        else stui_goto(anchor_x,anchor_y);

        char c = stui_get_key();
        if(c == STUI_KEY_ESC) break;
        if(c == STUI_KEY_LEFT) {
            GapBuffer_left(&text);
            continue;
        }
        if(c == STUI_KEY_RIGHT) {
            GapBuffer_right(&text);
            continue;
        }
        if(c == STUI_KEY_DELETE) {
            GapBuffer_delete(&text);
            continue;
        }
        if(c == 127) {
            GapBuffer_backspace(&text);
            continue;
        }
        if(c < 256){
            GapBuffer_insert_char(&text, c);
        }
    }

    stui_term_disable_instant();
    stui_term_enable_echo();
    return 0;
}
