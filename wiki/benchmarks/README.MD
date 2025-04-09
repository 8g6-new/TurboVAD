# Voice Acoustic Activity Detection Benchmark Report

This report focuses exclusively on the core processing pipeline—from spectral transformation and normalization to feature extraction and inference—while excluding peripheral operations (e.g., file reading and MP3 decoding). All metrics are normalized per second of audio to ensure valid comparisons, regardless of individual file durations.

> **Key Metrics:**
> - **pred:** The actual model inference (prediction) time per second of audio.
> - **total_pro:** The aggregated processing cost (including preprocessing and inference) per second of audio.
> - **Speedup Factor:** How many times faster than real time the pipeline processes audio.
> - **System-Level Metrics:** Include IPC (instructions per cycle), branch misprediction percentage, parallel speedup, and CPUs utilized.

---

## 1. Benchmark Generation

Benchmarks are generated using a Node.js script that executes the acoustic detection binary (`./builtin`) with performance analysis via `perf stat`. Key aspects include:

- **Dataset:**  
  480 audio files sourced from Xeno-canto, totaling **09:13:04 (36,749.466 s)** of audio.

- **Sampling Method:**  
  For each bird species in the dataset, three audio files are randomly selected. Results are averaged over these samples to provide representative performance metrics across a diverse set of audio inputs.

- **Batch Processing and Aggregation:**  
  Audio files are processed in batches, with performance data aggregated across all files.

- **Normalization by Audio Length:**  
  All function timings and derived performance metrics are normalized per second of audio, ensuring consistency regardless of file length.

- **Processing Parameters:**  
  - **FFT Size:** 1024  
  - **Hop Size:** 128  

- **Log Parsing and Data Extraction:**  
  Custom log-parsing functions extract function timings (e.g., STFT, normalization, feature extraction, inference) and system-level statistics (such as IPC, branch misprediction percentage, parallel speedup, and CPUs utilized).

- **Excluded Metrics:**  
  File reading and MP3 decoding metrics are excluded from this analysis.

- **Segment Size:**  
  This benchmark uses **50 ms segments**.

---

## 2. Core Pipeline Metrics (50 ms Segments)

These metrics represent key components of the processing pipeline, normalized per second of audio.

### Preprocessing Steps

| **Component**             | **Mean Time**           | **Std Dev**          | **Range (min - max)**               | **Description**                                    |
|---------------------------|-------------------------|----------------------|-------------------------------------|----------------------------------------------------|
| **auto_det**              | 55.333 ms               | ± 75.854 ms          | 0.102 ms – 542.656 ms               | Automatic file detection                           |
| **init_window_function**  | 46.060 ns               | ± 57.237 ns          | 0.117 ns – 536.585 ns               | Window function initialization                     |
| **STFT**                  | 27 µs                   | ± 18 µs              | 4 µs – 284 µs                       | Short-Time Fourier Transform (spectral analysis)   |
| **norm_all**              | 8.177 µs                | ± 6.702 µs           | 0.328 µs – 68.852 µs                | Data normalization                                 |
| **feat_extraction**       | 36.843 µs               | ± 11.677 µs          | 6.852 µs – 96.915 µs                | Feature extraction                                 |

### Inference

| **Component** | **Mean Time**    | **Std Dev**       | **Range (min - max)**        | **Description**                                 |
|---------------|------------------|-------------------|------------------------------|-------------------------------------------------|
| **pred**      | 65.711 ns        | ± 24.470 ns       | 43.568 ns – 247.842 ns       | Model inference (prediction) timing             |

### Overall Pipeline Performance

| **Metric**    | **Mean (per sec of audio)** | **Std Dev**         | **Range (min - max)**               | **Description**                                                   |
|---------------|-----------------------------|---------------------|-------------------------------------|-------------------------------------------------------------------|
| **total_pro** | 72 µs/sec                   | ± 28 µs/sec         | 14 µs/sec – 353 µs/sec              | Total processing cost (preprocessing and inference) per second of audio |

---

## 3. System-Level Metrics

| **Metric**            | **Mean**            | **Std Dev**         | **Range (min - max)**         | **Description**                                                    |
|-----------------------|---------------------|---------------------|-------------------------------|--------------------------------------------------------------------|
| **ipc**               | 0.759               | ± 0.168             | 0.225 – 1.279                 | Instructions per cycle                                             |
| **branch_miss_%**     | 1.99 %              | ± 0.51 %            | 0.60 % – 4.19 %               | Percentage of branch mispredictions                                |
| **parallel_speedup**  | 3.31x               | ± 0.87x             | 0.76x – 6.74x                 | Speedup from parallel execution (system_time / elapsed_time)       |
| **cpu_utilized**      | 4.30                | ± 1.10              | 1.19 – 8.62                   | Average number of CPUs utilized                                    |

Additional metrics—such as page faults, instructions, cycles, and stalled cycles—are recorded in detailed logs for further insight.

---

## 4. Speedup Analysis

The overall pipeline processes one second of audio in approximately **72 µs**. To compute the real-time speedup:

- **Real-Time Processing:** 1 second of audio = 1,000 ms  
- **Pipeline Processing Time:** 0.072 ms per second of audio (i.e., 72 µs)  
- **Speedup Factor:**  
  \[
  \text{Speedup} = \frac{1000 \, \text{ms}}{0.072 \, \text{ms}} \approx 13,889 \times \, \text{faster than real time}
  \]

This remarkable speedup demonstrates that the pipeline is exceptionally efficient for real-time applications.

---

## 5. Summary

- **Robust Sampling:**  
  The benchmark randomly selects three audio files per bird species from a dataset of 480 files sourced from Xeno-canto (totaling **09:13:04 (36,749.466 s)** of audio), with results averaged to ensure representativeness.

- **Normalization:**  
  All metrics are normalized per second of audio, ensuring valid comparisons across files with varying durations.

- **Core Pipeline Efficiency:**  
  Critical processing steps—including STFT, normalization, and feature extraction—are executed in microsecond to millisecond ranges, while the inference stage remains extremely fast (≈65.7 ns).

- **System-Level Performance:**  
  Metrics such as IPC, branch misprediction percentage, parallel speedup, and CPU utilization indicate robust hardware efficiency.

- **Real-Time Speedup:**  
  With a processing time of 72 µs per second of audio, the pipeline operates approximately **13,889 times faster** than real time, enabling high-throughput, edge-based bioacoustic analysis.

- **Configuration:**  
  - **FFT Size:** 1024  
  - **Hop Size:** 128  

This comprehensive evaluation highlights the performance gains achieved using 50 ms segments and underscores TurboVAD's potential for rapid, real-time audio processing in ecological monitoring and related applications.

---

_Total benchmark run (full dataset): 09:13:04 (36,749.466 s)_