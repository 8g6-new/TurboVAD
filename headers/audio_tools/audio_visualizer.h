#ifndef AUDIO_VISUALIZER_H
#define AUDIO_VISUALIZER_H 
    
    #include "audio_io.h"
    #include "../libheatmap/heatmap_tools.h"
    #include "spectral_features.h"
    #include <cblas.h>
    
    void spectrogram(stft_d *result, const char *output_file,float min, float max,unsigned char bg_clr[4],bool db,int cs_enum);
    float *mel_spectrogram(stft_d *result,const char *output_file,size_t num_filters,float *mel_filter_bank,unsigned char bg_clr[4],bool db,int cs_enum);
    void mfcc(float *mel_values,float *coeffs,const char *output_file,size_t w,size_t num_filters,size_t num_coff,unsigned char bg_clr[4],int cs_enum);
    void  precompute_cosine_coeffs(float *coeffs, size_t num_filters, size_t num_coff);


    float branchless_db(float mag,bool db);

#endif