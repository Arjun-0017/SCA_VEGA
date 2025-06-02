// Created by Team "RTL Rangers"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "aes.h"

#define NUM_SAMPLES 3500
#define TRACE_LENGTH 1024

// Fixed-point config
#define FIXED_TOTAL_BITS 12
#define FIXED_M 2
#define FIXED_N 10

#define HAMMING_TOTAL_BITS 8
#define HAMMING_M 8
#define HAMMING_N 0

// Fixed-point conversion (unsigned Qm.n format)
char* float_to_fixed_bin(float value, int total_bits, int m, int n) {
    if (value < 0.0f) {
        printf("Error: Negative value in unsigned fixed-point converter.\n");
        return NULL;
    }

    uint32_t fixed_val = (uint32_t)roundf(value * (1 << n));

    if (fixed_val >= (1U << (m + n))) {
        printf("Warning: Value %.6f overflows Q%d.%d range.\n", value, m, n);
        fixed_val = (1U << (m + n)) - 1;
    }

    char* bin_str = (char*)malloc(total_bits + 1);
    if (!bin_str) return NULL;

    for (int i = total_bits - 1; i >= 0; i--) {
        bin_str[total_bits - 1 - i] = ((fixed_val >> i) & 1) ? '1' : '0';
    }

    bin_str[total_bits] = '\0';
    return bin_str;
}

// Hamming Distance
int hamming_distance(uint8_t *a, uint8_t *b) {
    int dist = 0;
    for (int i = 0; i < 16; i++) {
        uint8_t xor_val = a[i] ^ b[i];
        dist += __builtin_popcount(xor_val);
    }
    return dist;
}

// Power trace feature extraction
void extract_features(float *trace, float *mean, float *peak, float *energy) {
    float sum = 0.0f, max_val = 0.0f, energy_val = 0.0f;
    for (int i = 0; i < TRACE_LENGTH; i++) {
        float val = trace[i];
        sum += val;
        energy_val += val * val;
        if (val > max_val) max_val = val;
    }
    *mean = sum / TRACE_LENGTH;
    *peak = max_val;
    *energy = energy_val;
}

// Structure to store extracted features
typedef struct {
    float mean_power;
    float peak_power;
    float energy;
    int hamming_dist;
} TraceFeature;

TraceFeature features[NUM_SAMPLES];

// Input data
uint8_t plaintexts[NUM_SAMPLES][16];
uint8_t keys[NUM_SAMPLES][16];
uint8_t ciphertexts[NUM_SAMPLES][16];
float power_traces[NUM_SAMPLES][TRACE_LENGTH];

// Load from CSV
void load_data_from_csv(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }

    char line[32768];
    int sample_idx = 0;

    fgets(line, sizeof(line), file); // Skip header

    while (fgets(line, sizeof(line), file) && sample_idx < NUM_SAMPLES) {
        char *token = strtok(line, ",");

        for (int i = 0; i < 16; i++) {
            sscanf(token, "%2hhx", &plaintexts[sample_idx][i]);
            token = strtok(NULL, ",");
        }

        for (int i = 0; i < 16; i++) {
            sscanf(token, "%2hhx", &ciphertexts[sample_idx][i]);
            token = strtok(NULL, ",");
        }

        for (int i = 0; i < 16; i++) {
            sscanf(token, "%2hhx", &keys[sample_idx][i]);
            token = strtok(NULL, ",");
        }

        for (int i = 0; i < TRACE_LENGTH; i++) {
            if (token != NULL) {
                sscanf(token, "%f", &power_traces[sample_idx][i]);
                token = strtok(NULL, ",");
            } else {
                power_traces[sample_idx][i] = 0.0f;
            }
        }

        sample_idx++;
    }

    fclose(file);
}

int main() {
    load_data_from_csv("Power_Trace_Train.csv");

    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint8_t computed_ct[16];
        struct AES_ctx ctx;
        AES_init_ctx(&ctx, keys[i]);

        uint8_t input_block[16];
        memcpy(input_block, plaintexts[i], 16);
        AES_ECB_encrypt(&ctx, input_block);

        memcpy(computed_ct, input_block, 16);

        int h_dist = hamming_distance(computed_ct, ciphertexts[i]);

        float mean, peak, energy;
        extract_features(power_traces[i], &mean, &peak, &energy);

        features[i].mean_power = mean;
        features[i].peak_power = peak;
        features[i].energy = energy;
        features[i].hamming_dist = h_dist;

        char* mean_bin = float_to_fixed_bin(mean, FIXED_TOTAL_BITS, FIXED_M, FIXED_N);
        char* peak_bin = float_to_fixed_bin(peak, FIXED_TOTAL_BITS, FIXED_M, FIXED_N);
        char* energy_bin = float_to_fixed_bin(energy, FIXED_TOTAL_BITS, FIXED_M, FIXED_N);
        char* hd_bin = float_to_fixed_bin((float)h_dist, HAMMING_TOTAL_BITS, HAMMING_M, HAMMING_N);

        printf("Sample %3d:\n", i);
        printf("  Mean      : %.6f -> %s\n", mean, mean_bin);
        printf("  Peak      : %.6f -> %s\n", peak, peak_bin);
        printf("  Energy    : %.6f -> %s\n", energy, energy_bin);
        printf("  HammingDist: %2d       -> %s (8-bit)\n", h_dist, hd_bin);

        free(mean_bin);
        free(peak_bin);
        free(energy_bin);
        free(hd_bin);
    }

    // === Find maxima and minima hamming distances from features[] ===
    int max_hamming = features[0].hamming_dist;
    int min_hamming = features[0].hamming_dist;
    int max_index = 0;
    int min_index = 0;

    for (int i = 1; i < NUM_SAMPLES; i++) {
        if (features[i].hamming_dist > max_hamming) {
            max_hamming = features[i].hamming_dist;
            max_index = i;
        }
        if (features[i].hamming_dist < min_hamming) {
            min_hamming = features[i].hamming_dist;
            min_index = i;
        }
    }

    printf("\n=== Hamming Distance Summary ===\n");
    printf("Minimum Hamming Distance: %d (Sample %d)\n", min_hamming, min_index);
    printf("Maximum Hamming Distance: %d (Sample %d)\n", max_hamming, max_index);

    return 0;
}
