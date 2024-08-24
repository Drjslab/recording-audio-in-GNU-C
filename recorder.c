#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "default"

typedef struct WAV_HEADER {
    char riff[4];        // RIFF string
    unsigned int overall_size;  // overall size of file in bytes
    char wave[4];        // WAVE string
    char fmt_chunk_marker[4];  // fmt string with trailing null char
    unsigned int length_of_fmt; // length of the format data
    unsigned short format_type; // format type
    unsigned short channels;    // number of channels
    unsigned int sample_rate;   // sampling rate (blocks per second)
    unsigned int byterate;      // SampleRate * NumChannels * BitsPerSample/8
    unsigned short block_align; // NumChannels * BitsPerSample/8
    unsigned short bits_per_sample; // bits per sample, 8- 8bits, 16- 16 bits etc
    char data_chunk_header[4];  // DATA string or FLLR string
    unsigned int data_size;     // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} wav_header;

void write_wav_header(FILE *file, wav_header *header) {
    fwrite(header, sizeof(wav_header), 1, file);
}

int main(int argc, char *argv[]) {
    int err;
    unsigned int rate = 44100;
    unsigned int channels = 2;
    unsigned int seconds = 5; // Record for 5 seconds
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    char *buffer;
    unsigned int buffer_frames = 128;
    int pcm;
    FILE *wav_file;
    wav_header header;

    // Initialize the header
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt_chunk_marker, "fmt ", 4);
    memcpy(header.data_chunk_header, "data", 4);
    header.length_of_fmt = 16;
    header.format_type = 1;
    header.channels = channels;
    header.sample_rate = rate;
    header.bits_per_sample = 16;
    header.byterate = rate * channels * header.bits_per_sample / 8;
    header.block_align = channels * header.bits_per_sample / 8;
    header.data_size = seconds * rate * channels * header.bits_per_sample / 8;
    header.overall_size = header.data_size + sizeof(wav_header) - 8;

    // Open the WAV file
    wav_file = fopen("output.wav", "wb");
    if (!wav_file) {
        fprintf(stderr, "Error: Could not open output.wav for writing\n");
        return -1;
    }
    write_wav_header(wav_file, &header);

    // Open PCM device for recording (capture)
    if ((err = snd_pcm_open(&capture_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "cannot open audio device %s (%s)\n", PCM_DEVICE, snd_strerror(err));
        return 1;
    }

    // Allocate parameters object and fill it with default values
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(capture_handle, hw_params);

    // Set parameters
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels);

    // Apply the HW parameters to the PCM device
    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
        fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
        return 1;
    }

    buffer = (char *) malloc(buffer_frames * header.block_align);

    for (int i = 0; i < (int) (seconds * rate / buffer_frames); ++i) {
        if ((pcm = snd_pcm_readi(capture_handle, buffer, buffer_frames)) == -EPIPE) {
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(capture_handle);
        } else if (pcm < 0) {
            fprintf(stderr, "error reading from PCM device: %s\n", snd_strerror(pcm));
        } else if (pcm != (int) buffer_frames) {
            fprintf(stderr, "short read, read %d frames\n", pcm);
        }

        fwrite(buffer, header.block_align, buffer_frames, wav_file);
    }

    snd_pcm_close(capture_handle);
    free(buffer);
    fclose(wav_file);

    printf("Recording complete. Saved to output.wav\n");

    return 0;
}
