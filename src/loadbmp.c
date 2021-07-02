#include "loadbmp.h"
#include "wminterop.h"
#include <stdio.h>

enum Bitmap_Loading_Status load_bmp_from_memory(const u8 *buffer, struct Wmi_Bitmap *result) {
    u32 w = *((u32*)(buffer+18));
    u32 h = *((u32*)(buffer+22));

    *result = wmi_bitmap_new(w, h);
    u32 texture_offset = *((u32*)(buffer+10));
    u32 *image = (u32*)(buffer+texture_offset);
    for (usize i = 0; i < w; i += 1) {
        for (usize j = 0; j < h; j += 1) {
            *wmi_bitmap_get_point_unsafe(result, i, j) = *(image+i+(h-j-1)*w);
        }
    }
    return BITMAP_LOADED_OK;
}

enum Bitmap_Loading_Status load_bmp_from_file(const char *filename, struct Wmi_Bitmap *result) {
    // Check if the file is availble
    if (access(filename, F_OK)) {
        return BITMAP_FILE_DOESNT_EXIST;
    }

    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    usize filesize = (usize) ftell(f);
    fseek(f, 0, SEEK_SET);
    u8 *data = malloc(filesize+1);
    fread(data, sizeof(u8), filesize, f);
    fclose(f);

    enum Bitmap_Loading_Status status = load_bmp_from_memory(data, result);
    free(data);
    return status;
}

