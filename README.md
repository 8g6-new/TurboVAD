# TurboVAD

**TurboVAD** is a **lightweight, high-speed** Voice Activity Detection (VAD) system that utilizes **self-attention** and **spectral features** for accurate voice detection. Designed for real-time applications, TurboVAD achieves **high efficiency and accuracy**, making it ideal for **low-latency** environments.

> **Note:** This project is currently **under testing**. The initial benchmark results are based on a **small sample size** and may vary as further testing and fine-tuning are performed.

## 🚀 Features
* **Self-Attention Mechanism** for efficient feature weighting
* **Spectral Feature Analysis** including spectral centroid, entropy, flatness, and harmonic noise ratio
* **Optimized C Implementation** for performance and efficiency
* **Model Size < 1KB**, making it ideal for memory-constrained applications
* **Fine-tuned for Bird Activity Detection** to enhance avian vocalization analysis
* **Highly optimized build system** with extensive compiler optimizations and SIMD support
* **Ultra-fast processing** at ~1198x faster than real-time

## 📌 Technical Overview
TurboVAD processes **8 spectral features** extracted from audio signals:
1. Spectral Centroid
2. Spectral Entropy
3. Spectral Flatness
4. Harmonic Noise Ratio
5. Spectral Centroid SD
6. Spectral Entropy SD
7. Spectral Flatness SD
8. Harmonic Noise Ratio SD

These features represent **mean (μ) and standard deviation (σ)** over a time frame, providing robust temporal characteristics for accurate classification.

These features are processed using a **self-attention mechanism**, which assigns dynamic importance to each feature and refines classification using a softmax-based scoring system. The model is **specifically optimized for bird activity detection**, making it ideal for wildlife monitoring and ornithological research.

## ⚡ Model Architecture
* **Feature Extraction:** Precomputed spectral statistics
* **Attention Mechanism:** Weights important features using learned attention matrices
* **Classification:** A lightweight decision function with a sigmoid activation to determine voice presence

## 📊 Benchmark Results


> **Note:** The benchmark results presented here are from initial tests using a small sample size. Further testing and benchmarking are required to refine the performance metrics.


### Performance Metrics
| Metric | Value |
|--------|-------|
| Accuracy | 79% |
| Model Size | < 1KB |
| Optimal Threshold | 0.5002 |
| Processing Speed | ~1198x faster than real-time |
| Total Processing Time | 0.834 ms per second of audio |
| Model Inference Time | 87.686 ns per second of audio |

### Processing Pipeline Breakdown
| Component | Mean Time | Description |
|-----------|-----------|-------------|
| STFT | 0.392 ms | Short-Time Fourier Transform (spectral analysis) |
| Normalization | 69.408 µs | Data normalization |
| Feature Extraction | 372.793 µs | Feature extraction from spectral data |
| Inference | 87.686 ns | Model prediction time |

### System-Level Performance
| Metric | Value | Description |
|--------|-------|-------------|
| IPC | 0.229 | Instructions per cycle |
| Branch Miss | 1.49% | Percentage of branch mispredictions |
| Parallel Speedup | 7.96x | Speedup from parallel execution |
| CPUs Utilized | 8.31 | Average number of CPUs used |

The benchmarks were generated using random sampling across diverse bird species, with results averaged over multiple samples to ensure representative performance metrics. All timings are normalized per second of audio for consistent comparison.

## 🛠️ Build System
TurboVAD employs a comprehensive build system with multiple optimization levels:

```bash
# Build with built-in color scheme (default)
make builtin

# Build with OpenCV-like color scheme
make opencv_like

# Build in debug mode 
make debug

# Clean build artifacts
make clean

# Run the program
make run

# Run with memory checking
make run_debug
```

### Build Features
* **Advanced Optimization** with `-Ofast` for maximum performance
* **SIMD Acceleration** with AVX, AVX2, FMA, and SSE4.2 instruction sets
* **Parallel Processing** via OpenMP (achieving ~8x parallel speedup)
* **Link-Time Optimization** for cross-file optimizations
* **Vectorization** of loops for improved performance
* **Aggressive loop optimizations** including unrolling, peeling, and interchange

## 🛠️ Input Parameters
TurboVAD accepts the following parameters:
* **filename:** Path to the input audio file
* **window_size_det:** Window size for detection
* **window_size_img:** Window size for output stft/mel spectrogram 
* **hop_size_det:** Hop size for detection
* **hop_size_img:** Hop size for output stft/mel spectrogram 
* **window_type_det:** Window function type for detection
* **window_type_img:** Window function type for output stft/mel spectrogram 
* **seg_len:** Segment length
* **num_mel:** Number of Mel filterbanks
* **mel_min:** Minimum Mel frequency
* **mel_max:** Maximum Mel frequency
* **wav_fol:** Path to the waveform folder
* **stft_fol:** Path to the STFT cache folder
* **mel_fol:** Path to the Mel spectrogram cache folder
* **cache_fol:** Path to the FFT cache folder
* **cs_stft:** Color scheme ID for visualization output STFT
* **cs_mel:** Color scheme ID for visualization output Mel Spectrogram
* **th:** Audio filtration threshold. If set to `0`, the STFT is filtered dynamically to remove noise. It is recommended that the default value for noisy data be 0.01

## 🛠️ Future Improvements
* Further fine-tuning for different bird species
* Expansion to support multi-environment bird call detection
* Optimizations for low-power embedded devices
* Benchmarking against traditional VAD systems

## 🤝 Contributing
Contributions are welcome! Feel free to open issues, submit PRs, or discuss improvements in the repo.

## 💜 License
**TurboVAD** is open-source under the **MIT License**.

## About
Spectral Feature Based Voice Activity Detection using Self Attention

## Topics
c, multithreading, fft, spectral-analysis, omp-parallel, voice-activity-detection, self-attention, bird-activity-detection