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
- **Ideal Belady Hits**: 4407 (Hit Ratio: 88.14%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 4263 | 85.26 | 4407 | 96.73 |
| 2Q | 4285 | 85.70 | 4407 | 97.23 |
| LFU | 4289 | 85.78 | 4407 | 97.32 |
| LIRS | 4289 | 85.78 | 4407 | 97.32 |
| ARC | 4288 | 85.76 | 4407 | 97.30 |

#### Multi-Level Cache (LRU+LRU) | HOT/COLD
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 3798
- Hit Ratio: 75.96%
- Efficiency: 86.18%
- Memory Accesses: 1202

**[EXCLUSIVE]**
- Total Hits: 4263
- Hit Ratio: 85.26%
- Efficiency: 96.73%
- Memory Accesses: 737

---

### Workload: WORKING_SET
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 4747 (Hit Ratio: 94.94%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 4720 | 94.40 | 4747 | 99.43 |
| 2Q | 4710 | 94.20 | 4747 | 99.22 |
| LFU | 645 | 12.90 | 4747 | 13.59 |
| LIRS | 4535 | 90.70 | 4747 | 95.53 |
| ARC | 4713 | 94.26 | 4747 | 99.28 |

#### Multi-Level Cache (LRU+LRU) | WORKING_SET
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 3857
- Hit Ratio: 77.14%
- Efficiency: 81.25%
- Memory Accesses: 1143

**[EXCLUSIVE]**
- Total Hits: 4720
- Hit Ratio: 94.40%
- Efficiency: 99.43%
- Memory Accesses: 280

---

### Workload: MIXED
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 3853 (Hit Ratio: 77.06%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 3342 | 66.84 | 3853 | 86.74 |
| 2Q | 3453 | 69.06 | 3853 | 89.62 |
| LFU | 3453 | 69.06 | 3853 | 89.62 |
| LIRS | 3340 | 66.80 | 3853 | 86.69 |
| ARC | 3388 | 67.76 | 3853 | 87.93 |

#### Multi-Level Cache (LRU+LRU) | MIXED
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 3154
- Hit Ratio: 63.08%
- Efficiency: 81.86%
- Memory Accesses: 1846

**[EXCLUSIVE]**
- Total Hits: 3342
- Hit Ratio: 66.84%
- Efficiency: 86.74%
- Memory Accesses: 1658

---

### Workload: RANDOM
- **Requests**: 5000
- **Capacity**: 30
- **Ideal Belady Hits**: 2598 (Hit Ratio: 51.96%)

| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |
| :--- | :--- | :--- | :--- | :--- |
| LRU | 987 | 19.74 | 2598 | 37.99 |
| 2Q | 989 | 19.78 | 2598 | 38.07 |
| LFU | 941 | 18.82 | 2598 | 36.22 |
| LIRS | 1202 | 24.04 | 2598 | 46.27 |
| ARC | 1018 | 20.36 | 2598 | 39.18 |

#### Multi-Level Cache (LRU+LRU) | RANDOM
- **L1 Capacity**: 10, **L2 Capacity**: 20 (Total: 30)

**[INCLUSIVE]**
- Total Hits: 654
- Hit Ratio: 13.08%
- Efficiency: 25.17%
- Memory Accesses: 4346

**[EXCLUSIVE]**
- Total Hits: 987
- Hit Ratio: 19.74%
- Efficiency: 37.99%
- Memory Accesses: 4013

---

