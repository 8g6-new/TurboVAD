#include "../../headers/audio_tools/audio_visualizer.h"


inline float  branchless_db(float mag,bool db){
    return !db*mag + db*log1pf(mag*mag);
}

inline void spectrogram(stft_d *result, const char *output_file, float min, float max, 
                        unsigned char bg_clr[4], bool db, int cs_enum) {

    const size_t w     = result->output_size;
    const size_t h     = (size_t)(result->num_frequencies * max) / (result->sample_rate / 2);
    const size_t min_f = (size_t)(result->num_frequencies * min) / (result->sample_rate / 2);

    heatmap_t *hm = NULL;
    unsigned char *image = heatmap_get_vars(w, (h - min_f), &hm);

    if (!image || !hm) {
        fprintf(stderr, "Failed to allocate heatmap.\n");
        return;
    }

    float *magnitudes = result->magnitudes;

    #pragma omp parallel for schedule(static,512)
    for (size_t t = 0; t < w; t++) {
        const size_t offset = t * result->num_frequencies;
        for (size_t f = min_f; f < h; f++) {
            size_t inv_f = h - f;  
            float mag = magnitudes[offset + inv_f];
            heatmap_add_weighted_point(hm, t, f,branchless_db(mag,db));
        }
    }
    save_heatmap(image, &hm, output_file, w, h - min_f, bg_clr, cs_enum);
}


inline float *mel_spectrogram(stft_d *result,const char *output_file,size_t num_filters,float *mel_filter_bank,unsigned char bg_clr[4],bool db,int cs_enum){
    
    size_t w             = result->output_size;
    size_t h             = result->num_frequencies;

    
    float *mel_values    = (float*) malloc(num_filters * w * sizeof(float));

    heatmap_t *hm        = NULL;
    
    unsigned char *image = heatmap_get_vars(w, num_filters, &hm);

    if (!image || !hm) {
        fprintf(stderr, "Failed to allocate heatmap.\n");
        return NULL;
    }

    float sum=0;
    #pragma omp parallel for schedule(dynamic,256)
    for (size_t t = 0; t < w; t++) {
            #pragma omp parallel for
            for (size_t mel = 0; mel < num_filters; mel++){ 
                size_t offset_mel = (num_filters - mel - 1) * h; 
                size_t offset_mag = t * h;
                sum = cblas_sdot(h, &mel_filter_bank[offset_mel], 1, &result->magnitudes[offset_mag], 1);
                // mel_values[t*num_filters+mel] =  sum;
                heatmap_add_weighted_point(hm, t, mel,branchless_db(sum,db)); 
            }
    }

    save_heatmap(image,&hm,output_file,w,num_filters,bg_clr,cs_enum);

   return mel_values;
}

void precompute_cosine_coeffs(float *coeffs, size_t num_filters, size_t num_coff) {
    float scale = sqrtf(2.0 / num_filters);
   #pragma omp parallel for schedule(static,256)
    for (size_t n = 0; n < num_coff; n++) {
        for (size_t mel = 0; mel < num_filters; mel++) {
            coeffs[n * num_filters + mel] = scale * cos((M_PI / num_filters) * (mel + 0.5) * n);
        }
    }
}


inline void mfcc(float *mel_values, float *coeffs, const char *output_file,
                 size_t w, size_t num_filters, size_t num_coff, 
                 unsigned char bg_clr[4], int cs_enum) {

    heatmap_t *hm = NULL;
    unsigned char *image = heatmap_get_vars(w, num_coff, &hm);

    #pragma omp parallel for schedule(static,256)
    for (size_t n = 0; n < num_coff; n++) { 
        for (size_t t = 0; t < w; t++) {
            float sum = cblas_sdot(num_filters, &coeffs[n * num_filters], 1, &mel_values[t * num_filters], 1);
            heatmap_add_weighted_point(hm, t, num_coff - n - 1, sum * sum);
        }
    }

    save_heatmap(image, &hm, output_file, w, num_coff, bg_clr, cs_enum);
}
