# Computer Architecture M2 Profiling

A microarchitectural profiling and analysis project on the **Apple M2 processor**, developed using **C** for microbenchmark execution and **Xcode Instruments** for hardware performance counter measurements.

This project was completed as part of the Computer Architecture course at **Sharif University of Technology** under the supervision of **Dr. Jahangir**.

---

## 🛠️ Implementation Details & Tools

- **Programming Language:** Written in **C** for low-level memory control, pointer chasing, and cache layout analysis.
- **Profiling & Tracing:** Utilized **Xcode Instruments** (Time Profiler & Custom Hardware Performance Counters) to monitor internal CPU hardware events.
- **Hardware Target:** Apple Silicon M2 processor running on macOS.
- **Report Typesetting:** LaTeX (XeLaTeX compiler).

---

## 📌 Executive Summary & Key Findings

### 1. Microarchitectural Fingerprinting
- **L1 Data Cache Size:** 128 KB (Data latency: ~1.5 ns)
- **L2 Cache Size:** 16 MB (Latency: 9.5–12 ns)
- **Cache Line Size:** 128 Bytes
- **Set Associativity:** 8-way set-associative (L1 Data Cache)

### 2. Hardware Performance Counters (Xcode Instruments)
- **IPC (Instructions Per Cycle):** ~0.2525
- **Branch Misprediction Rate:** Extremely low (~0.0010%)
- **TLB Miss Rate:** ~4.64%

### 3. Performance Optimization (Cache Tiling)
- Evaluated **Row-Major vs. Column-Major** matrix traversal ($4096 \times 4096$).
- Applied **Cache Tiling ($32 \times 32$)** in C to eliminate cache thrashing.
- **Results:**
  - **96.5% reduction** in L1D TLB misses.
  - **20% speedup** in overall execution time.

---

## 📁 Repository Structure

```text
.
├── benchmark/                   # C source code for cache microbenchmarks
├── report/                      # Project reports in English and Persian
│   ├── English/                 # English LaTeX source files & compiled PDF
│   └── Persian/                 # Persian LaTeX source files & compiled PDF
├── output.zip                   # Raw Xcode Instruments trace & profiling data
├── Project Specification.pdf    # Original course project assignment description
└── README.md                    # Project documentation
```

## Full Report
For complete methodological details, hardware event breakdowns, and detailed timeline figures, please refer to the compiled PDF reports in either the report/English/ or report/Persian/ directory.
