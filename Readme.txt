1. cmake -S . -B build
2. cmake --build build --config Debug
3. Run from visual studio or PowerShell/Git Bash


# Controls

## Normal Mode Selection
* Key 5 (Mode 0): Summed Approximation - Lightweight approximation based on wave derivatives.
* Key 6 (Mode 1): Exact Analytical - Precise cross-product derivation using exact partial derivatives.
* Key 7 (Mode 2): Finite Difference - Estimates surface slope by multi-sampling wave positions per vertex.

## Mesh Resolution Scaling
* **Keys 1–4:** Scales grid density from low-poly (~1,250 triangles) up to high-density stress testing (~2,000,000 triangles).

## Performance Monitoring
Real-time frame execution times (in milliseconds) and FPS are displayed in the application window's title bar. Toggle between normal modes at high triangle counts to observe execution time shifts.