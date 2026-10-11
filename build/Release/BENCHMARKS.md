# Benchmark Results

Benchmarked over 300 frames per entry (fixed timestep dt = 1/60 s).

| n | Method | Build/Refit (us) | Query Time (us) | Pairs Tested | Total Step (us) |
|------|--------------|------------------|-----------------|--------------|-----------------|
| 100 | Median Split | 10.94 | 6.90 | 275 | 17.83 |
| 100 | SAH Split | 35.19 | 6.09 | 228 | 41.28 |
| 100 | Refit (Fat) | 1.02 | 25.62 | 703 | 26.63 |
| 500 | Median Split | 105.86 | 78.70 | 2132 | 184.56 |
| 500 | SAH Split | 277.85 | 76.91 | 1920 | 354.76 |
| 500 | Refit (Fat) | 7.98 | 1083.08 | 14065 | 1091.05 |
| 1000 | Median Split | 324.67 | 281.82 | 5595 | 606.49 |
| 1000 | SAH Split | 466.01 | 178.45 | 5523 | 644.46 |
| 1000 | Refit (Fat) | 16.10 | 4181.64 | 53421 | 4197.74 |
| 5000 | Refit (Fat) | 81.40 | 107119.69 | 1418003 | 107201.09 |

_Measured with <chrono::high_resolution_clock> on MSVC /O2 /W4._
