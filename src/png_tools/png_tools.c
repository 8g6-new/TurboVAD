#include "../../headers/png_tools/png_tools.h"

inline unsigned char branchless_comp(unsigned char original, unsigned char new_val, unsigned char is_black) {
    return (is_black * new_val) + (!is_black * original);
}

void add_bg(unsigned char *image, size_t width, size_t height, const unsigned char color[4]) {
    size_t num_pixels = width * height;

    #pragma omp parallel for simd aligned(image:32) safelen(16)
    for (size_t i = 0; i < num_pixels; i++) {
        unsigned char *pixel = &image[i * 4];

        unsigned char is_black = (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0);

        pixel[0] = branchless_comp(pixel[0], color[0], is_black);
        pixel[1] = branchless_comp(pixel[1], color[1], is_black);
        pixel[2] = branchless_comp(pixel[2], color[2], is_black);
        pixel[3] = color[3]; 
    }
}

void save_png(const char *filename, const unsigned char *image, size_t width, size_t height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fprintf(stderr, "png_create_write_struct failed\n");
        fclose(fp);
        exit(1);
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        fprintf(stderr, "png_create_info_struct failed\n");
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        exit(1);
    }

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "Error during PNG creation\n");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        exit(1);
    }

    png_init_io(png, fp);
    png_set_IHDR(
        png, info,
        (png_uint_32)width, (png_uint_32)height,
        8, PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png, info);

    png_bytep *row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep));
    if (!row_pointers) {
        perror("malloc for row_pointers");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        exit(1);
    }

    #pragma omp parallel for
    for (size_t y = 0; y < height; ++y) {
        row_pointers[y] = (png_bytep)(image + 4 * width * y);
    }

    png_write_image(png, row_pointers);
    free(row_pointers);

    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

unsigned char* resize_image(const unsigned char *original, size_t orig_width, size_t orig_height, size_t new_width, size_t new_height) {
    unsigned char *resized = (unsigned char *)aligned_alloc(32, new_width * new_height * 4);
    if (!resized) {
        perror("aligned_alloc");
        exit(1);
    }

    float x_scale = (float)(orig_width - 1) / (new_width - 1);
    float y_scale = (float)(orig_height - 1) / (new_height - 1);

    size_t orig_row_size = orig_width * 4;
    size_t resized_row_size = new_width * 4;

    #pragma omp parallel for schedule(dynamic)
    for (size_t y = 0; y < new_height; ++y) {
        float orig_y = y * y_scale;
        size_t y0 = (size_t)orig_y;
        size_t y1 = (y0 + 1 < orig_height) ? y0 + 1 : y0;
        float wy = orig_y - y0;

        const unsigned char *row0 = original + y0 * orig_row_size;
        const unsigned char *row1 = original + y1 * orig_row_size;
        unsigned char *resized_row_ptr = resized + y * resized_row_size;

        #pragma omp simd
        for (size_t x = 0; x < new_width; ++x) {
            float orig_x = x * x_scale;
            size_t x0 = (size_t)orig_x;
            size_t x1 = (x0 + 1 < orig_width) ? x0 + 1 : x0;
            float wx = orig_x - x0;

            for (size_t c = 0; c < 4; ++c) {
                float top = (1 - wx) * row0[x0 * 4 + c] + wx * row0[x1 * 4 + c];
                float bottom = (1 - wx) * row1[x0 * 4 + c] + wx * row1[x1 * 4 + c];
                resized_row_ptr[x * 4 + c] = (unsigned char)((1 - wy) * top + wy * bottom);
            }
        }
    }

    return resized;
}
