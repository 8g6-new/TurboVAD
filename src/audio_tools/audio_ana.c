#include "../../headers/audio_tools/audio_ana.h"

inline size_t hz_to_index(size_t num_freq, size_t sample_rate, float f) {
    return (size_t)((num_freq * f) / (sample_rate / 2.0f));
}

float safe_div(float a, float b) {
    float mask  = (float)(b > EPSILON); 
    return mask * (a / (b + (1.0f - mask) * EPSILON));
}



bool is_freq_feats_zero(freq_feats *feats) {
    return (feats->spectral_centroid == 0.0f &&
            feats->spectral_entropy == 0.0f &&
            feats->spectral_flatness == 0.0f &&
            feats->harmonic_noise_ratio == 0.0f);
}

bool is_freq_feats_stats_zero(freq_feats_stats *feats) {
    return (feats->spectral_centroid == 0.0f &&
            feats->spectral_entropy == 0.0f &&
            feats->spectral_flatness == 0.0f &&
            feats->harmonic_noise_ratio == 0.0f &&
            feats->spectral_centroid_sd == 0.0f &&
            feats->spectral_entropy_sd == 0.0f &&
            feats->spectral_flatness_sd == 0.0f &&
            feats->harmonic_noise_ratio_sd == 0.0f);
}


void free_res(res *preds, size_t size) {
    if (!preds) return;
    
    for (size_t t = 0; t < size; t++) {
        free(preds[t].vals_str);
    }
    free(preds);
}


void print_spectral_features(freq_feats_stats *feats, size_t t, float seg_len)  {
    printf("\n%zu) [%f %f] : ", t, seg_len * t, seg_len * (t + 1));
    printf("\"SC\": %.3f (SD: %.3f), ", feats->spectral_centroid, feats->spectral_centroid_sd);
    printf("\"SE\": %.3f (SD: %.3f), ", feats->spectral_entropy, feats->spectral_entropy_sd);
    printf("\"SF\": %.3f (SD: %.3f), ", feats->spectral_flatness, feats->spectral_flatness_sd);
    printf("\"HNR\": %.3f (SD: %.3f)\n", feats->harmonic_noise_ratio, feats->harmonic_noise_ratio_sd);
}


float dynamic_th(float *array, const size_t size){
    float avg_noise = 0.0f;

    #pragma omp parallel for reduction(+:avg_noise)
    for (size_t i = 0; i < size; i++) {
        avg_noise += array[i];
    } 

    avg_noise/=size;

    return avg_noise;
}

inline void minmax(float comp,float *max,float *min){
    *max = (comp > *max) ? comp : *max;
    *min = (comp < *min) ? comp : *min;
}

void minmax_parr(float *array, size_t size, float *min_val, float *max_val) {
    float min_local = array[0];
    float max_local = array[0];

    #pragma omp parallel for reduction(min:min_local) reduction(max:max_local)
    for (size_t i = 1; i < size; i++) {
        if (array[i] < min_local) min_local = array[i];
        if (array[i] > max_local) max_local = array[i];
    }

    *min_val = min_local;
    *max_val = max_local;
}


inline float norm(float *array, const size_t size, const float th) {

    float min_val = array[0], max_val = array[0];
    
    minmax_parr(array, size, &min_val, &max_val);

    const float range     = max_val - min_val;

    if (size == 0) return 0.0f; 

    float avg_noise = th==0 ? (dynamic_th(array, size) - min_val) / range : th;
     


    const float inv_range = 1.0f / range; 

    #pragma omp parallel for simd
    for (size_t i = 0; i < size; i++) {
      float val = (array[i] - min_val) * inv_range;
       array[i] = (val > avg_noise) * val; 
    }

    return avg_noise;

}


void accumulate_spectral_features(char *org,freq_feats_stats *feats, size_t t, float seg_len) {
    char temp[BUFFER_SIZE];
    snprintf(temp, sizeof(temp), "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
             seg_len * t, seg_len * (t + 1),
             feats->spectral_centroid, feats->spectral_centroid_sd,
             feats->spectral_entropy, feats->spectral_entropy_sd,
             feats->spectral_flatness, feats->spectral_flatness_sd,
             feats->harmonic_noise_ratio, feats->harmonic_noise_ratio_sd);
    strcpy(org,temp);
}


float spectral_centroid(freq_mag_pair *phaser, size_t count) {
    float numerator = 0.0f, denominator = 0.0f;

    #pragma omp parallel for reduction(+: numerator, denominator)
    for (size_t i = 0; i < count; i++) {
        numerator   += phaser[i].freq * phaser[i].mag;
        denominator += phaser[i].mag;
    }

    return safe_div(numerator, denominator);
}

float spectral_entropy(freq_mag_pair *phaser, size_t count) {
    float sum = 0.0f;
    const float alpha = 2.0f; // Renyi entropy parameter (2 = more senstive to harmaonic )
    
    #pragma omp parallel for reduction(+: sum)
    for (size_t i = 0; i < count; i++) {
        sum += phaser[i].mag;
    }
    
    if (sum <= EPSILON) return 0.0f;
    
    float entropy_sum = 0.0f;
    
    #pragma omp parallel for reduction(+: entropy_sum)
    for (size_t i = 0; i < count; i++) {
        float prob = phaser[i].mag / sum;
        entropy_sum += powf(prob, alpha);
    }
    
    // Renyi entropy formula
    return (1.0f / (1.0f - alpha)) * logf(entropy_sum);
}




float spectral_flatness(freq_mag_pair *phaser, size_t count) {
    float sum = 0.0f, log_sum = 0.0f;

    #pragma omp parallel for reduction(+: sum, log_sum)
    for (size_t i = 0; i < count; i++) {
        sum     += phaser[i].mag;
        log_sum += logf(phaser[i].mag);
    }

    if (sum == 0 || count == 0) return 0.0f; // Handle edge cases

    float geometric_mean = expf(log_sum / count);
    return safe_div(geometric_mean, sum);
}


float estimate_F0(freq_mag_pair *peaks, size_t count) {
    if (count == 0) return 0.0f;

    float max_mag = 0.0f, F0 = 0.0f;

    #pragma omp parallel
    {
        float local_max_mag = 0.0f, local_F0 = 0.0f;

        #pragma omp for nowait
        for (size_t i = 0; i < count; i++) {
            if (peaks[i].freq >= MIN_HARMONIC && peaks[i].freq <= MAX_HARMONIC) {
                if (peaks[i].mag > local_max_mag) {
                    local_max_mag = peaks[i].mag;
                    local_F0 = peaks[i].freq;
                }
            }
        }

        // Atomic update only when necessary
        #pragma omp critical
        {
            if (local_max_mag > max_mag) {
                max_mag = local_max_mag;
                F0 = local_F0;
            }
        }
    }

    return F0;
}


float harmonic_noise_ratio(freq_mag_pair *peaks, size_t peak_count) {
    float F0 = estimate_F0(peaks, peak_count);
    if (F0 == 0.0f) return 0.0f;

    float harmonic_power = 0.0f, noise_power = 0.0f;

    #pragma omp parallel for reduction(+: harmonic_power, noise_power)
    for (size_t i = 0; i < peak_count; i++) {
        float ratio = peaks[i].freq / F0;
        float deviation = fabsf(ratio - roundf(ratio));
        if (deviation < HARMONIC_TOLERANCE) {
            harmonic_power += peaks[i].mag * peaks[i].mag;
        } else {
            noise_power += peaks[i].mag * peaks[i].mag;
        }
    }

    if (harmonic_power + noise_power < EPSILON) return 0.0f;
    float ratio = safe_div(harmonic_power, noise_power);
    return ratio > 0 ? 10 * log10f(ratio) : 0.0f;
}

freq_mag_pair *phaser_gen(float *mags, float *frequencies, size_t offset, size_t fmin, size_t range, size_t *count) {
    *count = 0;  
    size_t c=0;
    #pragma omp parallel for reduction(+:c)
    for (size_t i = 0; i < range; i++) {
        if (mags[i + offset + fmin] > 0.0f) {
            c++;
        }
    }

    *count = c;

    freq_mag_pair *phasers = calloc(*count, sizeof(freq_mag_pair));
    if (!phasers) {
        fprintf(stderr, "Memory allocation failed in phaser_gen\n");
        return NULL;
    }

    size_t idx = 0;
    #pragma omp parallel for
    for (size_t i = 0; i < range; i++) {
        if (mags[i + offset + fmin] > 0.0f) {
            size_t pos;
            #pragma omp atomic capture
            pos = idx++;

            phasers[pos].mag = mags[i + offset + fmin];
            phasers[pos].freq = frequencies[i + fmin];
        }
    }

    return phasers;
}


freq_mag_pair *find_peaks(freq_mag_pair *phasers, size_t range, size_t *count) {
    *count = 0;
    if (range < 3 || !phasers) return NULL;

    size_t *peak_indices = malloc((range / 2) * sizeof(size_t));  // Preallocate assuming ~50% peaks
    if (!peak_indices) return NULL;

    size_t local_counts[omp_get_max_threads()];  // Thread-local counters
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        local_counts[tid] = 0;
        
        size_t local_indices[range / 2];  // Local array to avoid conflicts

        #pragma omp for nowait
        for (size_t i = 1; i < range - 1; i++) {
            if (phasers[i].mag > phasers[i-1].mag && phasers[i].mag > phasers[i+1].mag) {
                local_indices[local_counts[tid]++] = i;
            }
        }

        size_t offset = 0;
        #pragma omp critical
        {
            offset = *count;
            *count += local_counts[tid];
            for (size_t j = 0; j < local_counts[tid]; j++) {
                peak_indices[offset + j] = local_indices[j];
            }
        }
    }

    if (*count == 0) {
        free(peak_indices);
        return NULL;
    }

    freq_mag_pair *peaks = malloc(*count * sizeof(freq_mag_pair));
    if (!peaks) {
        free(peak_indices);
        return NULL;
    }

    #pragma omp parallel for
    for (size_t i = 0; i < *count; i++) {
        peaks[i] = phasers[peak_indices[i]];
    }

    free(peak_indices);
    return peaks;
}



freq_feats calc_freq_feats(float* mags, float* freqs, size_t offset, size_t fmin, size_t range) {
    freq_feats feats       = {0};
    size_t non_zero_count  = 0,peak_count= 0;

    freq_mag_pair *phasers = phaser_gen(mags, freqs, offset, fmin, range,&non_zero_count);
    freq_mag_pair *peaks   = find_peaks(phasers,non_zero_count,&peak_count);

    if (peaks != NULL) {
        #pragma omp parallel sections
        {
            #pragma omp section
            feats.spectral_centroid = spectral_centroid(phasers, non_zero_count);

            #pragma omp section
            feats.spectral_entropy = spectral_entropy(phasers, non_zero_count);

            #pragma omp section
            feats.spectral_flatness = spectral_flatness(phasers,non_zero_count);

            #pragma omp section
            feats.harmonic_noise_ratio = harmonic_noise_ratio(peaks,peak_count);
        }
        free(peaks);
    }

    free(phasers);

    return feats;

}


void norm_feat(float value, float mi, float mx, float *normalized) {
    *normalized = (value - mi) / (mx - mi);
    *normalized = CLIP(*normalized, 0.0f, 1.0f);
}

void normalize_feats(freq_feats_stats *feats) {
    norm_feat(feats->spectral_centroid,       SPECTRAL_CENTROID_MIN,       SPECTRAL_CENTROID_MAX,      &(feats->spectral_centroid));
    norm_feat(feats->spectral_entropy,        SPECTRAL_ENTROPY_MIN,        SPECTRAL_ENTROPY_MAX,        &(feats->spectral_entropy));
    norm_feat(feats->spectral_flatness,       SPECTRAL_FLATNESS_MIN,       SPECTRAL_FLATNESS_MAX,       &(feats->spectral_flatness));
    norm_feat(feats->harmonic_noise_ratio,    HARMONIC_NOISE_RATIO_MIN,    HARMONIC_NOISE_RATIO_MAX,    &(feats->harmonic_noise_ratio));
    norm_feat(feats->spectral_centroid_sd,    SPECTRAL_CENTROID_SD_MIN,    SPECTRAL_CENTROID_SD_MAX,    &(feats->spectral_centroid_sd));
    norm_feat(feats->spectral_entropy_sd,     SPECTRAL_ENTROPY_SD_MIN,     SPECTRAL_ENTROPY_SD_MAX,     &(feats->spectral_entropy_sd));
    norm_feat(feats->spectral_flatness_sd,    SPECTRAL_FLATNESS_SD_MIN,    SPECTRAL_FLATNESS_SD_MAX,    &(feats->spectral_flatness_sd));
    norm_feat(feats->harmonic_noise_ratio_sd, HARMONIC_NOISE_RATIO_SD_MIN, HARMONIC_NOISE_RATIO_SD_MAX, &(feats->harmonic_noise_ratio_sd));
}


freq_feats_stats freq_feats_time_avg(filter_param *param, stft_d *result, size_t curr) {
    const size_t start = curr       * param->seg_length_index;
    size_t         end = (curr + 1) * param->seg_length_index;

                   end = (end > result->output_size) ? result->output_size : end;

    freq_feats_stats feats_avg = {0};
    freq_feats feats;
    size_t offset;
    const size_t freq_range = param->max_freq - param->min_freq;
    const size_t num = end - start;

    if (num == 0) return feats_avg;
  
    float spectral_centroid=0.0f, spectral_entropy=0.0f, spectral_flatness=0.0f, harmonic_noise_ratio=0.0f;
    #pragma omp parallel for reduction(+:spectral_centroid, spectral_entropy, spectral_flatness, harmonic_noise_ratio)
    for (size_t t = start; t < end; t++) {
        offset = t * param->abs_max;
        feats = calc_freq_feats(result->magnitudes, result->frequencies, offset, param->min_freq, freq_range);
        
        spectral_centroid    += feats.spectral_centroid;
        spectral_entropy     += feats.spectral_entropy;
        spectral_flatness    += feats.spectral_flatness;
        harmonic_noise_ratio += feats.harmonic_noise_ratio;
    }

    feats_avg.spectral_centroid    = spectral_centroid / num;
    feats_avg.spectral_entropy     = spectral_entropy / num;
    feats_avg.spectral_flatness    = spectral_flatness / num;
    feats_avg.harmonic_noise_ratio = harmonic_noise_ratio / num;


    float centroid_var = 0.0f, entropy_var = 0.0f, flatness_var = 0.0f, hnr_var = 0.0f;

    #pragma omp parallel for reduction(+: centroid_var, entropy_var, flatness_var, hnr_var)
    for (size_t t = start; t < end; t++) {
        offset = t * param->abs_max;
        feats = calc_freq_feats(result->magnitudes, result->frequencies, offset, param->min_freq, freq_range);

        centroid_var += powf(feats.spectral_centroid - feats_avg.spectral_centroid, 2);
        entropy_var  += powf(feats.spectral_entropy - feats_avg.spectral_entropy, 2);
        flatness_var += powf(feats.spectral_flatness - feats_avg.spectral_flatness, 2);
        hnr_var      += powf(feats.harmonic_noise_ratio - feats_avg.harmonic_noise_ratio, 2);
    }

    feats_avg.spectral_centroid_sd    = sqrtf(centroid_var / num);
    feats_avg.spectral_entropy_sd     = sqrtf(entropy_var  / num);
    feats_avg.spectral_flatness_sd    = sqrtf(flatness_var / num);
    feats_avg.harmonic_noise_ratio_sd = sqrtf(hnr_var      / num);
     
    normalize_feats(&feats_avg);

    return feats_avg;
}


res *stft_pred(stft_d *result, filter_param *param,float th) {    
    
    START_TIMING();
    norm(result->magnitudes, result->output_size * result->num_frequencies,th);
    END_TIMING("norm_all");
    
    res *preds      = (res*) malloc(param->output_size * sizeof(res));

    if (!preds) {
        fprintf(stderr, "Memory allocation failed for detections\n");
        exit(EXIT_FAILURE);
    }
    


    for(size_t t=0;t<param->output_size;t++){
        preds[t].vals_str = malloc(BUFFER_SIZE * sizeof(char));
    }

    float input[param->output_size][8];
    
    START_TIMING(); 
  
    #pragma omp parallel for   
    for (size_t t = 0; t < param->output_size; t++) {
        preds[t].stats    = freq_feats_time_avg(param,result,t);
        
        input[t][0] = preds[t].stats.spectral_centroid;
        input[t][1] = preds[t].stats.spectral_entropy;
        input[t][2] = preds[t].stats.spectral_flatness;
        input[t][3] = preds[t].stats.harmonic_noise_ratio;
        input[t][4] = preds[t].stats.spectral_centroid_sd;
        input[t][5] = preds[t].stats.spectral_entropy_sd;
        input[t][6] = preds[t].stats.spectral_flatness_sd;
        input[t][7] = preds[t].stats.harmonic_noise_ratio_sd;
        
        preds[t].all_zero = is_freq_feats_stats_zero(&preds[t].stats);
        //print_spectral_features(&preds[t].stats, t,param->seg_length);
        accumulate_spectral_features(preds[t].vals_str, &preds[t].stats, t,param->seg_length);
    }
    END_TIMING("feat_extraction");
    
    START_TIMING();
    for (size_t t = 0; t < param->output_size; t++) {
        preds[t].preds = preds[t].all_zero ? 
            (pred_t){.val=0, .pred=false} : model(input[t]);
    }
    END_TIMING("pred");


    return preds;
}
