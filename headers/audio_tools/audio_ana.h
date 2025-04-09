#ifndef AUDIO_ANA_H
#define AUDIO_ANA_H

    #include <stdlib.h>
    #include <math.h>
    #include <stdbool.h>
    #include <cblas.h>
    #include <float.h>

    #include "spectral_features.h"
    #include "../utils/bench.h"
    #include "../ml/model.h"
 

    #define MAX_FILENAME_LENGTH 500


    #define MIN_HARMONIC 50
    #define MAX_HARMONIC 7500 
    #define HARMONIC_TOLERANCE 0.001
    #define EPSILON 1e-6  

    #define SPECTRAL_CENTROID_MIN 50.0f
    #define SPECTRAL_CENTROID_MAX 5000.0f

    #define SPECTRAL_ENTROPY_MIN 0.0f
    #define SPECTRAL_ENTROPY_MAX 5.0f

    #define SPECTRAL_FLATNESS_MIN 0.0f
    #define SPECTRAL_FLATNESS_MAX 1.0f

    #define HARMONIC_NOISE_RATIO_MIN -15.0f
    #define HARMONIC_NOISE_RATIO_MAX 15.0f

    #define SPECTRAL_CENTROID_SD_MIN 0.0f
    #define SPECTRAL_CENTROID_SD_MAX 5000.0f

    #define SPECTRAL_ENTROPY_SD_MIN 0.0f
    #define SPECTRAL_ENTROPY_SD_MAX 5.0f

    #define SPECTRAL_FLATNESS_SD_MIN 0.0f
    #define SPECTRAL_FLATNESS_SD_MAX 0.5f

    #define HARMONIC_NOISE_RATIO_SD_MIN 0.0f
    #define HARMONIC_NOISE_RATIO_SD_MAX 15.0f
    
    #define BUFFER_SIZE 256

    
    typedef struct {
        float mag;  
        float freq;  
    } freq_mag_pair;


    typedef struct {
        float spectral_centroid;  
        float spectral_entropy;
        float spectral_flatness;
        float harmonic_noise_ratio;
    } freq_feats;

    typedef struct {
        float spectral_centroid;  
        float spectral_entropy;
        float spectral_flatness;
        float harmonic_noise_ratio;
        float spectral_centroid_sd;  
        float spectral_entropy_sd;
        float spectral_flatness_sd;
        float harmonic_noise_ratio_sd;
    } freq_feats_stats;
    

    typedef struct{
        char wav_name[MAX_FILENAME_LENGTH];
        char stft_name[MAX_FILENAME_LENGTH];
        char mel_name[MAX_FILENAME_LENGTH];
        size_t t;
    } det;
   
    
    typedef struct{
        pred_t preds;
        char *vals_str;
        freq_feats_stats stats; 
        bool all_zero;
    } res;

    typedef struct {
        size_t abs_max;     // max possible freq = sr/2;
        size_t min_freq;    // mel band min (user defined) 
        size_t max_freq;    // mel band max (user defined) 
        size_t band_fmin;   // max band     (auto detection)
        size_t band_fmax;   // max band     (auto detection)
        size_t seg_length_index;
        size_t output_size;
        float  seg_length;       
        char  *log_txt;
    } filter_param;

    #define CLIP(x, min, max) (x < min ? min : (x > max ? max : x))

    void    minmax(float comp, float *max, float *min);
    void    minmax_parr(float *array, size_t size, float *min_val, float *max_val);
    void    init_params_stft_stats(filter_param *params, stft_d *result, float fmin, float fmax);
    float   norm(float *array, const size_t size,const float th);
    size_t  hz_to_index(size_t num_freq, size_t sample_rate, float f);
    float   index_to_hz(size_t num_freq, size_t sample_rate, size_t index);
    res     *stft_pred(stft_d *result, filter_param *param,float th);
    bool    is_freq_feats_stats_zero(freq_feats_stats *feats);
    bool    is_freq_feats_zero(freq_feats *feats);
    void    free_res(res *preds, size_t size);
    
#endif 