# Benchmark Results

Benchmarked over 300 frames per entry (fixed timestep dt = 1/60 s).

| n | Method | Build/Refit (us) | Query Time (us) | Pairs Tested | Total Step (us) |
|------|--------------|------------------|-----------------|--------------|-----------------|
| 100 | Median Split | 12.05 | 7.38 | 274 | 19.44 |
| 100 | SAH Split | 29.99 | 4.59 | 229 | 34.58 |
| 100 | Refit (Fat) | 0.98 | 24.60 | 770 | 25.58 |
| 500 | Median Split | 97.67 | 67.19 | 2072 | 164.86 |
| 500 | SAH Split | 211.52 | 56.44 | 1873 | 267.96 |
| 500 | Refit (Fat) | 8.47 | 1025.94 | 14735 | 1034.41 |
| 1000 | Median Split | 252.44 | 211.56 | 5650 | 464.00 |
| 1000 | SAH Split | 488.43 | 182.82 | 5452 | 671.26 |
| 1000 | Refit (Fat) | 13.71 | 4042.09 | 56410 | 4055.80 |
| 5000 | Refit (Fat) | 68.08 | 95165.21 | 1445007 | 95233.29 |

_Measured with <chrono::high_resolution_clock> on MSVC /O2 /W4._
