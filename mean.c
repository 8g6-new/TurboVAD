#include <stdio.h>
#include <sys/time.h> 

#include "headers/utils/bench.h"
#include "headers/audio_tools/spectral_features.h"
#include "headers/audio_tools/audio_ana.h"
#include "headers/audio_tools/wav.h"
#include "headers/audio_tools/audio_visualizer.h"


char check(int i, int lim) {
    return (i < lim - 1) ? '\n' : ' ';
}

struct timeval strat,global_start;


double get_elapsed_time(struct timeval start) {
    struct timeval end;
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    return seconds + microseconds * 1e-6;
}

void show_progress(size_t i, size_t c, struct timeval start) {
    double progress = (double)(i + 1) * 100.0 / c;
    double elapsed  = get_elapsed_time(start) * 1000;
    
    double estimated_total = (elapsed / (i + 1)) * c;
    double eta = estimated_total - elapsed;
    
    int h = (int)(eta / 3600);
    int m = (int)((eta - h * 3600) / 60);
    int s = (int)(eta - h * 3600 - m * 60);
    int ms = (int)((eta - h * 3600 - m * 60 - s) * 1000);

    fflush(stdout);
    printf("[%zu/%zu] %.2f%% ETA: %02d:%02d:%02d.%03d\r", i + 1, c, progress, h, m, s, ms);
}



char *find_fn(const char *s) {
    char *token = strdup(s);  
    if (token == NULL) {
        return NULL;
    }

    char *original = token; 
    char *start = NULL;
    char *end = NULL;

    while (*token != '\0') {
        if (*token == '/') {
            start = token + 1;
        }
        if (*token == '.') {
            end = token;
        }
        token++;
    }

    if (start == NULL || end == NULL || start >= end) {
        free(original); 
        return NULL;
    }

    int length = end - start;
    char *out = malloc(length + 1);
    if (out == NULL) {
        free(original);
        return NULL;
    }

    memcpy(out, start, length);
    out[length] = '\0';

    free(original);  

    return out;
}


int main(int argc, char *argv[]) {
    if (argc != 19) {
        fprintf(stderr, "Usage: %s <filename> <window_size_det> <window_size_img> <hop_size_det> <hop_size_img_img> <window_type_det> <window_type_img> <seg_len> <num_mel> <min_mel> <max_mel> <wav_fol> <stft_fol> <mel_fol> <cache_fol> <cs_stft> <cs_mel>  <th>\n", argv[0]);
        return 1;
    }

    const char         *filename         = argv[1];
    const size_t        window_size_det  = atoi(argv[2]);
    const size_t        window_size_img  = atoi(argv[3]);
    const size_t        hop_size_det     = atoi(argv[4]);
    const size_t        hop_size_img     = atoi(argv[5]);
    const char         *window_type_det  = argv[6];
    const char         *window_type_img  = argv[7];
    const float         seg_len          = atof(argv[8]);
    const size_t        num_mel          = atoi(argv[9]);
    const size_t        mel_min          = atoi(argv[10]);
    const size_t        mel_max          = atoi(argv[11]);
    const char         *wav_fol          = argv[12];
    const char         *stft_fol         = argv[13];
    const char         *mel_fol          = argv[14];
    const char         *cache_fol        = argv[15];
    const unsigned int  cs_stft          = atoi(argv[16]);
    const unsigned int  cs_mel           = atoi(argv[17]);
    const float            th            = atof(argv[18]);

    char *fn                             = find_fn(filename);
    audio_data audio                     = auto_detect(filename);

    
    fft_d fft_plan_1;
    fft_d fft_plan_2;
    
    filter_param param = {0};
    
    printf("\n");
    print_ad(&audio);
    printf("\n");

    if (audio.samples != NULL) {
        
        START_TIMING();
        float *window_values = (float*)malloc(window_size_det * sizeof(float));
        if (!window_values) {
            fprintf(stderr, "Memory allocation failed for window values.\n");
            return 1;
        }
        window_function(window_values, window_size_det, window_type_det);
        END_TIMING("init_window_function");
        
        START_TIMING();
        fft_plan_1 = init_fftw_plan(window_size_det,cache_fol);
        END_TIMING("fetch fft cache 1");

        START_TIMING();
        stft_d stft_for_pred = stft(&audio, window_size_det, hop_size_det, window_values, &fft_plan_1);
        END_TIMING("compute STFT");
        
        START_TIMING();
        if(window_size_det != window_size_img){
            fftw_forget_wisdom(); // wisdom is stored globally and we need to reomve it to use another wisdom with diff fft size
            fft_plan_2 = init_fftw_plan(window_size_img,cache_fol); 
        }
        END_TIMING("fetch fft cache 2");
          
              
        if (stft_for_pred.phasers == NULL || stft_for_pred.magnitudes == NULL) {
            fprintf(stderr, "STFT computation failed.\n");
            return 1;
        }

        char fn_out[50];
        snprintf(fn_out,50, "./csv/%s_log.csv", fn);

        param.seg_length_index  = (size_t)((stft_for_pred.sample_rate * seg_len)/hop_size_det);
        param.output_size       = (size_t)(stft_for_pred.output_size / param.seg_length_index) + 1;
        param.seg_length        = seg_len;
        param.min_freq          = hz_to_index(stft_for_pred.num_frequencies, stft_for_pred.sample_rate,mel_min);
        param.max_freq          = hz_to_index(stft_for_pred.num_frequencies, stft_for_pred.sample_rate,mel_max);
        param.abs_max           = stft_for_pred.num_frequencies;
        param.log_txt           = strdup(fn_out);


        unsigned char bg_clr[4] = {0, 0, 0, 255};


        res *detections = stft_pred(&stft_for_pred,&param,th);
         


        START_TIMING();
        float* window_values_2 = (float*)malloc(window_size_img * sizeof(float));
        window_function(window_values_2, window_size_img, window_type_img);
        float* mel_filter_bank = (float*)calloc((size_t)(window_size_img / 2 + 1) * (num_mel + 2), sizeof(float));
        mel_filter(mel_min, mel_max, num_mel, audio.sample_rate, window_size_img, mel_filter_bank);
        END_TIMING("initialize_mel_filter");

        // START_TIMING();
        // snprintf(fn_out,50, "%s_stft.png", fn);
        // spectrogram(&stft_for_pred,fn_out, mel_min, mel_max, bg_clr, false, cs_stft);
        // END_TIMING("stft_no_log");
        // START_TIMING();
        // snprintf(fn_out,50, "%s_log.png", fn);
        // spectrogram(&stft_for_pred,fn_out, mel_min, mel_max, bg_clr, true, cs_stft);
        // END_TIMING("stft_log");
    

        printf("Audio Detections:\n");

        unsigned short c = 0;
        det detected[param.output_size];

        for (size_t t = 0; t < param.output_size; t++) {
            if(!detections[t].all_zero){
                if (detections[t].preds.pred) {
                    //printf("%zu [%0.3f,%0.3f] => %f\n", t, seg_len * t, seg_len * (t + 1), detections[t].preds.val);
                    detected[c].t = t;
                    snprintf(detected[c].wav_name, MAX_FILENAME_LENGTH, "%s/bird/%s_%s.wav", wav_fol, fn,detections[t].vals_str);
                    snprintf(detected[c].stft_name, MAX_FILENAME_LENGTH, "%s/bird/%s_%s.png", stft_fol, fn,detections[t].vals_str);
                    snprintf(detected[c].mel_name, MAX_FILENAME_LENGTH, "%s/bird/%s_%s.png", mel_fol, fn,detections[t].vals_str);
                    c++;
                }
                else{
                    detected[c].t = t;
                    snprintf(detected[c].wav_name, MAX_FILENAME_LENGTH, "%s/nobird/%s_%s.wav", wav_fol, fn,detections[t].vals_str);
                    snprintf(detected[c].stft_name, MAX_FILENAME_LENGTH, "%s/nobird/%s_%s.png", stft_fol, fn,detections[t].vals_str);
                    snprintf(detected[c].mel_name, MAX_FILENAME_LENGTH, "%s/nobird/%s_%s.png", mel_fol, fn,detections[t].vals_str);
                    c++;
                } 
            }
            
            
        }
        START_TIMING();
        
        free_res(detections, param.output_size);

        fft_d fft_plan = window_size_det != window_size_img ? fft_plan_2 : fft_plan_1;
        
      
        // for (size_t i = 0; i < c; i++) {
        //     struct timeval start;
        //     gettimeofday(&start, NULL);
        //     audio_data out    = sliced_write_wave(&audio, detected[i].wav_name, detected[i].t, seg_len);
        //     stft_d slice      = stft(&out, window_size_img, hop_size_img, window_values_2,&fft_plan);
        //     slice.sample_rate = stft_for_pred.sample_rate;
        //     spectrogram(&slice, detected[i].stft_name, mel_min, mel_max, bg_clr,true, cs_stft);
        //     //float *mel_vals   = mel_spectrogram(&slice, detected[i].mel_name, num_mel, mel_filter_bank, bg_clr, true, cs_mel);
        //     show_progress(i, c, start);
            
        //     free_stft(&slice);
        //     //free(mel_vals);
        //     free_audio(&out);
        // }
        END_TIMING("IO_Loop");
        
        printf("\nsegments:%zu\n",param.output_size);
        print_bench();

        free_audio(&audio);
        free_stft(&stft_for_pred);
        free(window_values);
        free(window_values_2);
        free(mel_filter_bank);
        free(fn);
        free_fft(&fft_plan_1);
        // free_fft(&fft_plan);
        free(param.log_txt);
        fftw_forget_wisdom();
    }
    
    return 0;        

}

