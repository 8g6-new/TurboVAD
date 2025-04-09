#ifndef WAV_H
#define WAV_H

    #include <stdint.h>
    #include <stdio.h>
    #include <string.h>

    #define WAV_FORMAT_PCM   0x0001
    #define WAV_FORMAT_FLOAT 0x0003

    #include "audio_io.h"

    #pragma pack(push, 1)
    typedef struct {
        char     riff[4];         /* "RIFF"                                  */
        uint32_t file_length;     /* file length in bytes                    */
        char     wave[4];         /* "WAVE"                                  */
        char     fmt[4];          /* "fmt "                                  */
        uint32_t chunk_size;      /* size of FMT chunk in bytes (usually 16) */
        uint16_t format_tag;      /* 1=PCM, 3=IEEE float                    */
        uint16_t num_channels;    /* 1=mono, 2=stereo                       */
        uint32_t sample_rate;     /* Sampling rate in samples per second     */
        uint32_t bytes_per_sec;   /* bytes per second                       */
        uint16_t block_align;     /* bytes per sample * num_channels        */
        uint16_t bits_per_sample; /* Number of bits per sample              */
        char     data[4];         /* "data"                                 */
        uint32_t data_length;     /* data length in bytes                   */
    } wav_header;
    #pragma pack(pop)             /* to restore the shaped ( remove compiler padding */
   
    void init_wav_header(wav_header* header, int format_tag, int channels, int sample_rate, int bits_per_sample, uint32_t data_length);
    int write_pcm_wav(const char *filename, const int16_t *pcm, uint32_t sample_count, int channels, int sample_rate);
    int write_float_wav(const char *filename, const float *pcm, uint32_t sample_count,int channels, int sample_rate);
    audio_data sliced_write_wave(audio_data *audio,const char *fn,size_t t,float seg_len);
    void       free_audio(audio_data *audio);
 
#endif 