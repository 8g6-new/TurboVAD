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

TurboVAD processes **8 spectral-temporal features**:
1. Spectral Centroid (μ + σ)
2. Spectral Entropy (μ + σ)
3. Spectral Flatness (μ + σ)
4. Harmonic Noise Ratio (μ + σ)

These features represent **mean (μ) and standard deviation (σ)** over a time frame, providing robust temporal characteristics for accurate classification.

These features are processed using a **self-attention mechanism**, which assigns dynamic importance to each feature and refines classification using a softmax-based scoring system. The model is **specifically optimized for bird activity detection**, making it ideal for wildlife monitoring and ornithological research.


## ⚡ Model Architecture and Data Pipeline

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'fontSize': '12px'}}}%%
graph TD
    %% Full Data Pipeline
    A[Audio File] --> B[Audio MIME Detection]
    B --> C{Format?}
    C -->|MP3| D[MP3 Decoding]
    C -->|WAV| E[WAV Decoding]
    D --> F[STFT Analysis]
    E --> F
    F --> G[Feature Extraction]
    G --> H[Attention Mechanism]
    H --> I[Prediction]
    I --> J{Activity Detected?}
    J -->|Yes| K[Save Segment to WAV]
    J -->|Yes| L[Save STFT Spectrogram]
    J -->|No| M[Discard]

    %% Attention Mechanism Details
    subgraph Attention Mechanism
        H --> N[Input Features<br/>μ1-4, σ1-4]
        N --> O1[Mu Branch]
        N --> O2[Sigma Branch]
        
        O1 --> P1[Dot Product<br/>W_mu]
        O2 --> P2[Dot Product<br/>W_sigma]
        
        P1 --> Q1[Softmax]
        P2 --> Q2[Softmax]
        
        Q1 --> R1[Linear Combination<br/>Classifier Weights]
        Q2 --> R2[Linear Combination<br/>Classifier Weights]
        
        R1 --> S[Mu Score]
        R2 --> T[Sigma Score]
        
        S --> U[Weighted Sum]
        T --> U
        U --> V[Sigmoid Activation]
        V --> W[Prediction Threshold]
    end

    classDef process fill:#e1f5fe,stroke:#039be5;
    classDef decision fill:#f0f4c3,stroke:#c0ca33;
    classDef storage fill:#dcedc8,stroke:#689f38;
    classDef attention fill:#f8bbd0,stroke:#c2185b;
    
    class A,B,C,D,E,F,G,H,J,M process;
    class K,L storage;
    class N,O1,O2,P1,P2,Q1,Q2,R1,R2,S,T,U,V,W attention;
    class I,J decision;

## 📊 Benchmark Results


> **Note:** The benchmark results presented here are from initial tests using a small sample size. Further testing and benchmarking are required to refine the performance metrics.



### Processing Pipeline Breakdown

### Performance Comparison: Segment Size Optimization
| **Metric**               | **0.5s Segments**        | **50ms Segments** 🚀      | **Improvement**        |
|--------------------------|--------------------------|---------------------------|------------------------|
| **Processing Speed**     | ~1,198× real-time        | ~13,889× real-time        | **11.6× faster**       |
| **Total Processing Time**| 0.834 ms/audio-sec       | 0.072 ms/audio-sec        | **11.6× reduction**    |
| **Model Inference Time** | 87.686 ns/audio-sec      | 65.711 ns/audio-sec       | **25.1% faster**       |
| **STFT**                 | 0.392 ms                 | 27 µs (0.027 ms)          | **14.5× faster**       |
| **Normalization**        | 69.408 µs                | 8.177 µs                  | **8.5× faster**        |
| **Feature Extraction**   | 372.793 µs               | 36.843 µs                 | **10.1× faster**       |
| **System IPC**           | 0.229                    | 0.759                     | **3.3× better**        |
| **Branch Miss Rate**     | 1.49%                    | 1.99%                     | 33.6% increase 📉      |
| **Parallel Speedup**     | 7.96×                    | 3.31×                     | 58.4% reduction 📉     |
| **CPUs Utilized**        | 8.31                     | 4.30                      | 48.3% reduction 📉     |

**Key Observations**:
- 🚀 **14.5× faster STFT** from improved cache locality
- 📉 **Parallel efficiency tradeoff** due to increased I/O-bound workload
- ⚡ **3.3× better IPC** shows improved instruction throughput
- 🔄 **Branch prediction** slightly impacted by finer segmentation

> **Note:** The reduced parallel speedup and CPU utilization reflect increased I/O dominance at smaller segment sizes - a common tradeoff in real-time systems.

### Current Performance Metrics (50ms Segments)
| Metric | Value |
|--------|-------|
| Accuracy | 79% |
| Model Size | < 1KB |
| Optimal Threshold | 0.5002 | 
| Processing Speed | ~13,889× real-time |
| Total Processing Time | 0.072 ms/audio-sec |

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
