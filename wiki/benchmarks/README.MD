# Voice Acoustic Activity Detection Benchmark Report

This report details the benchmark results for the voice acoustic activity detection model. The analysis excludes file I/O operations (e.g., file reading and the IO loop) and focuses strictly on the essential audio processing and inference steps.

## Overview

The benchmark covers multiple stages of the pipeline:

- **Preprocessing:** Operations such as audio detection, decoding, window initialization, spectral transformation, normalization, feature extraction, and mel filter initialization.
- **Inference:** The actual model prediction step.
- **Overall Pipeline:** The aggregated cost of preprocessing and inference per second of audio processed.

> **Note:** In these benchmarks, the key metric for actual model performance is `pred` (inference time), while `total_pro` represents the complete pipeline cost including preprocessing.

---

## Preprocessing Steps

| Component              | Mean Time       | Std Dev           | Range (min - max)                     | Description                                    |
|------------------------|-----------------|-------------------|---------------------------------------|------------------------------------------------|
| **auto_det**           | 1170.808 ms     | ± 1876.135 ms     | 3.410 ms – 11166.963 ms                | Audio detection with high variance             |
| **dec_mp3**            | 616.970 ms      | ± 229.657 ms      | 223.925 ms – 1324.767 ms               | MP3 decoding                                   |
| **init_window_function** | 526.198 ns  | ± 500.679 ns      | 39.124 ns – 3090.909 ns                | Window function initialization                 |
| **STFT**               | 0.392 ms        | ± 0.235 ms        | 0.124 ms – 1.171 ms                    | Short-Time Fourier Transform (spectral analysis) |
| **norm_all**           | 69.408 µs       | ± 47.394 µs       | 9.706 µs – 281.745 µs                  | Normalization                                  |
| **feat_extraction**    | 372.793 µs      | ± 97.035 µs       | 154.503 µs – 658.061 µs                | Feature extraction                             |
| **initialize_mel_filter** | 8106.987 ns | ± 7559.523 ns     | 1041.322 ns – 44589.744 ns             | Mel filter bank initialization                 |

---

## Inference

| Component | Mean Time  | Std Dev       | Range (min - max)            | Description                       |
|-----------|------------|---------------|------------------------------|-----------------------------------|
| **pred**  | 87.686 ns  | ± 48.251 ns   | 50.562 ns – 442.857 ns       | Model inference (actual prediction) |

---

## Overall Pipeline Performance

| Metric     | Mean (per sec of audio) | Std Dev             | Range (min - max)               | Description                                           |
|------------|-------------------------|---------------------|---------------------------------|-------------------------------------------------------|
| **total_pro** | 0.834 ms/sec          | ± 0.293 ms/sec      | 0.328 ms/sec – 1.612 ms/sec     | Total processing cost including preprocessing and inference |

---

## Summary

- **Preprocessing Efficiency:** Most preprocessing steps—such as spectral analysis, normalization, and feature extraction—operate well within sub-millisecond time ranges, contributing minimally to overall delay.
- **Optimized Inference:** The inference step (`pred`) is optimized to be extremely fast (sub-100 ns on average), ensuring nearly instantaneous predictions after preprocessing.
- **Pipeline Cost:** With an aggregated processing cost of roughly 0.834 ms per second of audio, the model demonstrates high efficiency, making it well-suited for real-time audio processing applications.
- **Observations:** The high variance seen in some preprocessing stages (e.g., `auto_det` and `dec_mp3`) may be attributed to variations in input complexity and system overhead. Future optimizations should target these operations to further streamline the processing pipeline.

---

This report provides a clear and concise benchmark for developers and researchers, illustrating the performance characteristics of the voice acoustic activity detection pipeline and highlighting areas for potential optimization.
