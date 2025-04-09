# Voice Acoustic Activity Detection Benchmark Report

This report details the benchmark results for the voice acoustic activity detection model. The results focus on the core processing pipeline—from spectral transformation and normalization to feature extraction and inference—while excluding peripheral operations that do not directly contribute to end-to-end processing speed. All metrics are normalized per second of audio, ensuring consistency regardless of individual audio file durations.

> **Key Metrics:**
> - **pred:** The actual model inference (prediction) time per second of audio.
> - **total_pro:** The aggregated processing cost (including preprocessing and inference) per second of audio.
> - **Speedup Factor:** How many times faster than real time the pipeline processes audio.
> - **System-Level Metrics:** Include IPC (instructions per cycle), branch misprediction percentage, parallel speedup, and CPUs utilized.

---

## 1. Benchmark Generation

Benchmarks are generated using a Node.js script that executes the acoustic detection binary (`./builtin`) with performance analysis via `perf stat`. Key aspects include:

- **Random Sampling Across Bird Species:**  
  For each bird species in the dataset, three audio files are randomly selected. The results are averaged over these samples to yield representative performance metrics across a diverse set of audio files.

- **Batch Processing and Aggregation:**  
  Audio files are processed in batches with performance data aggregated across all bird species.

- **Normalization by Audio Length:**  
  All function timings and derived performance metrics are normalized per second of audio, ensuring that comparisons remain consistent regardless of file length.

- **Log Parsing and Data Extraction:**  
  Custom log-parsing functions extract the relevant function timings (e.g., STFT, normalization, feature extraction, inference) as well as system-level statistics, including IPC, branch miss percentages, parallel speedup, and CPUs utilized (derived from the "CPUs utilized" metric reported by `perf stat`).

---

## 2. Core Pipeline Metrics

These metrics represent the key components of the processing pipeline, normalized per second of audio:

### Preprocessing Steps

| Component            | Mean Time          | Std Dev           | Range (min - max)              | Description                                     |
|----------------------|--------------------|-------------------|--------------------------------|-------------------------------------------------|
| **STFT**             | 0.392 ms           | ± 0.235 ms        | 0.124 ms – 1.171 ms            | Short-Time Fourier Transform (spectral analysis)|
| **norm_all**         | 69.408 µs          | ± 47.394 µs       | 9.706 µs – 281.745 µs          | Data normalization                              |
| **feat_extraction**  | 372.793 µs         | ± 97.035 µs       | 154.503 µs – 658.061 µs        | Feature extraction                              |

### Inference

| Component | Mean Time      | Std Dev         | Range (min - max)         | Description                                    |
|-----------|----------------|-----------------|---------------------------|------------------------------------------------|
| **pred**  | 87.686 ns      | ± 48.251 ns     | 50.562 ns – 442.857 ns    | Model inference (prediction) timing            |

### Overall Pipeline Performance

| Metric       | Mean (per sec of audio) | Std Dev             | Range (min - max)              | Description                                                      |
|--------------|-------------------------|---------------------|--------------------------------|------------------------------------------------------------------|
| **total_pro**| 0.834 ms/sec            | ± 0.293 ms/sec      | 0.328 ms/sec – 1.612 ms/sec    | Total processing cost including preprocessing and inference      |

---

## 3. System-Level Metrics

These additional system-level metrics offer insight into the underlying processing efficiency:

| Metric               | Mean          | Std Dev        | Range (min - max)       | Description                                                     |
|----------------------|---------------|----------------|-------------------------|-----------------------------------------------------------------|
| **ipc**              | 0.229         | ± 0.010        | 0.186 – 0.250           | Instructions per cycle                                          |
| **branch_miss_%**    | 1.49 %        | ± 0.32 %       | 0.75 % – 2.23 %         | Percentage of branch mispredictions                             |
| **parallel_speedup** | 7.96x         | ± 1.56x        | 3.24x – 10.13x          | Speedup from parallel execution (system_time / elapsed_time)      |
| **cpu_utilized**     | 8.31          | ± 1.62         | 3.38 – 10.48            | Average number of CPUs utilized                                 |

---

## 4. Speedup Analysis

The overall pipeline processes one second of audio in approximately 0.834 ms. To determine how many times faster than real time the system operates, the following calculation is used:

- **Real-time processing:** 1 second of audio equals 1000 ms  
- **Pipeline processing time:** 0.834 ms per second of audio  
- **Speedup Factor:**

\[
\text{Speedup} = \frac{1000\text{ ms}}{0.834\text{ ms}} \approx 1198 \text{ times faster than real time}
\]

This impressive speedup confirms that the pipeline is exceptionally efficient for real-time audio processing.

---

## 5. Summary

- **Robust Sampling:**  
  The benchmark randomly selects three files for each bird species, and the results are averaged across these diverse samples to provide representative metrics.

- **Normalization:**  
  All metrics are normalized per second of audio, ensuring comparability across files with varying durations.

- **Core Pipeline Efficiency:**  
  Essential processing steps (STFT, normalization, and feature extraction) execute in microsecond to millisecond ranges, while the inference stage is extremely fast (≈87 ns).  

- **System-Level Performance:**  
  Metrics such as IPC, branch miss percentage, parallel speedup, and CPUs utilized provide a deeper view into the system's efficiency.

- **Real-Time Speedup:**  
  With an overall processing time of 0.834 ms per second of audio, the pipeline operates approximately 1198 times faster than real time, making it highly suitable for applications requiring rapid audio processing.

