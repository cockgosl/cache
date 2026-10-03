# Cache Algorithms Benchmark Results

### Workload: LOOP
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 4235 (Hit Ratio: 84.70%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 0 | 0.00 | 4235 | 0.00 |
| 2Q | 0 | 0.00 | 4235 | 0.00 |
| LFU | 0 | 0.00 | 4235 | 0.00 |
| LIRS | 4118 | 82.36 | 4235 | 97.24 |
| ARC | 17 | 0.34 | 4235 | 0.40 |

#### Multi-Level Cache (LRU+LRU) | LOOP
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 0
- Hit Ratio: 0.00%
- Efficiency: 0.00%
- Memory Accesses: 5000

**[EXCLUSIVE]**
- Total Hits: 0
- Hit Ratio: 0.00%
- Efficiency: 0.00%
- Memory Accesses: 5000

---

### Workload: SCAN
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 0 (Hit Ratio: 0.00%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 0 | 0.00 | 0 | 0.00 |
| 2Q | 0 | 0.00 | 0 | 0.00 |
| LFU | 0 | 0.00 | 0 | 0.00 |
| LIRS | 0 | 0.00 | 0 | 0.00 |
| ARC | 0 | 0.00 | 0 | 0.00 |

#### Multi-Level Cache (LRU+LRU) | SCAN
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 0
- Hit Ratio: 0.00%
- Efficiency: 0.00%
- Memory Accesses: 5000

**[EXCLUSIVE]**
- Total Hits: 0
- Hit Ratio: 0.00%
- Efficiency: 0.00%
- Memory Accesses: 5000

---

### Workload: HOT/COLD
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 4392 (Hit Ratio: 87.84%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 4261 | 85.22 | 4392 | 97.02 |
| 2Q | 4276 | 85.52 | 4392 | 97.36 |
| LFU | 4275 | 85.50 | 4392 | 97.34 |
| LIRS | 4277 | 85.54 | 4392 | 97.38 |
| ARC | 4271 | 85.42 | 4392 | 97.24 |

#### Multi-Level Cache (LRU+LRU) | HOT/COLD
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 3781
- Hit Ratio: 75.62%
- Efficiency: 86.09%
- Memory Accesses: 1219

**[EXCLUSIVE]**
- Total Hits: 4261
- Hit Ratio: 85.22%
- Efficiency: 97.02%
- Memory Accesses: 739

---

### Workload: WORKING_SET
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 4747 (Hit Ratio: 94.94%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 4702 | 94.04 | 4747 | 99.05 |
| 2Q | 4692 | 93.84 | 4747 | 98.84 |
| LFU | 641 | 12.82 | 4747 | 13.50 |
| LIRS | 4531 | 90.62 | 4747 | 95.45 |
| ARC | 4697 | 93.94 | 4747 | 98.95 |

#### Multi-Level Cache (LRU+LRU) | WORKING_SET
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 3863
- Hit Ratio: 77.26%
- Efficiency: 81.38%
- Memory Accesses: 1137

**[EXCLUSIVE]**
- Total Hits: 4702
- Hit Ratio: 94.04%
- Efficiency: 99.05%
- Memory Accesses: 298

---

### Workload: MIXED
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 3444 (Hit Ratio: 68.88%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 2772 | 55.44 | 3444 | 80.49 |
| 2Q | 2880 | 57.60 | 3444 | 83.62 |
| LFU | 2883 | 57.66 | 3444 | 83.71 |
| LIRS | 2732 | 54.64 | 3444 | 79.33 |
| ARC | 2810 | 56.20 | 3444 | 81.59 |

#### Multi-Level Cache (LRU+LRU) | MIXED
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 2631
- Hit Ratio: 52.62%
- Efficiency: 76.39%
- Memory Accesses: 2369

**[EXCLUSIVE]**
- Total Hits: 2772
- Hit Ratio: 55.44%
- Efficiency: 80.49%
- Memory Accesses: 2228

---

### Workload: RANDOM
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 2619 (Hit Ratio: 52.38%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 1010 | 20.20 | 2619 | 38.56 |
| 2Q | 1028 | 20.56 | 2619 | 39.25 |
| LFU | 990 | 19.80 | 2619 | 37.80 |
| LIRS | 1222 | 24.44 | 2619 | 46.66 |
| ARC | 1005 | 20.10 | 2619 | 38.37 |

#### Multi-Level Cache (LRU+LRU) | RANDOM
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 690
- Hit Ratio: 13.80%
- Efficiency: 26.35%
- Memory Accesses: 4310

**[EXCLUSIVE]**
- Total Hits: 1010
- Hit Ratio: 20.20%
- Efficiency: 38.56%
- Memory Accesses: 3990

---

