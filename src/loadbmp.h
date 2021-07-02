/**
 * BMP loader
 * ishidex2 2021, MIT license
 */

#include "wminterop.h"
#include <stdbool.h>

enum Bitmap_Loading_Status {
    BITMAP_LOADED_OK,
    BITMAP_FILE_DOESNT_EXIST
};

// This assumes the buffer will be freed manually by the user
enum Bitmap_Loading_Status load_bmp_from_memory(const u8 *buffer, struct Wmi_Bitmap *result);

// This will just use the function above but read the file beforehand
enum Bitmap_Loading_Status load_bmp_from_file(const char *filename, struct Wmi_Bitmap *result);

