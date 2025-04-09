#include "../../headers/audio_tools/wav.h"


void init_wav_header(wav_header* header, int format_tag, int channels, int sample_rate, int bits_per_sample, uint32_t data_length) {
    memcpy(header->riff, "RIFF", 4);
    memcpy(header->wave, "WAVE", 4);
    memcpy(header->fmt,  "fmt ", 4);
    memcpy(header->data, "data", 4);

    header->chunk_size      = 16; 
    header->format_tag      = format_tag;
    header->num_channels    = channels;
    header->sample_rate     = sample_rate;
    header->bits_per_sample = bits_per_sample;
    header->block_align     = channels * (bits_per_sample / 8);
    header->bytes_per_sec   = sample_rate * header->block_align;
    header->data_length     = data_length;
    header->file_length     = data_length + sizeof(wav_header) - 8;  
}

int write_pcm_wav(const char *filename, const int16_t *pcm, uint32_t sample_count, int channels, int sample_rate) {
    
    wav_header header;
    uint32_t data_length = sample_count * channels * sizeof(int16_t);
    
    init_wav_header(&header, WAV_FORMAT_PCM, channels, sample_rate, 16, data_length);

    FILE *fout = fopen(filename, "wb");
    if (!fout) {
        perror("Error opening file for writing");
        return -1;
    }

    if (fwrite(&header, sizeof(header), 1, fout) != 1) {
        perror("Error writing WAV header");
        fclose(fout);
        return -1;
    }

    if (fwrite(pcm, 1, data_length, fout) != data_length) {
        perror("Error writing WAV data");
        fclose(fout);
        return -1;
    }
    else{
        // printf("%s PCM 16bit WAV file written successfully.\n",filename);
    }


    fclose(fout);
    return 0;
}

int write_float_wav(const char *filename, const float *pcm, uint32_t sample_count,int channels, int sample_rate) {
    
    wav_header header;
    uint32_t data_length = sample_count * channels * sizeof(float);

    init_wav_header(&header,WAV_FORMAT_FLOAT, channels, sample_rate, 32, data_length);
    
    FILE *fout = fopen(filename, "wb");
    if (!fout) {
        perror("Error opening file for writing");
        return -1;
    }

    if (fwrite(&header, sizeof(header), 1, fout) != 1) {
        perror("Error writing WAV header");
        fclose(fout);
        return -1;
    }

    if (fwrite(pcm, 1, data_length, fout) != data_length) {
        perror("Error writing WAV data");
        fclose(fout);
        return -1;
    }
    else{
        // printf("%s Float 32 bit WAV file written successfully.\n",filename);
    }

    fclose(fout);
    return 0;
}


audio_data sliced_write_wave(audio_data *audio,const char *fn,size_t t,float seg_len) {

    audio_data audio_out={0};
    
    float start = t * seg_len;
    float end   = (t+1) * seg_len;
    
    uint64_t start_sample  = (uint64_t)(start * audio->sample_rate * audio->channels);
    uint64_t end_sample    = (uint64_t)(end   * audio->sample_rate * audio->channels);
    uint64_t slice_samples = end_sample - start_sample;

    audio_out.samples      = malloc(slice_samples * DATA_SIZE);
    audio_out.channels     = audio->channels;
    audio_out.sample_rate  = audio_out.sample_rate;
    audio_out.num_samples  = slice_samples;

    memcpy(audio_out.samples, (W_D_TYPE *)audio->samples + start_sample, slice_samples * DATA_SIZE);

    write_wave(fn,audio_out.samples,slice_samples / audio->channels, audio->channels, audio->sample_rate);

    return audio_out;
}

