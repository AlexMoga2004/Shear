# Shear

**Shear** is a high-performance desktop video clipping and trimming application built in C++ and Qt. Designed for power users who need quick, precise video editing without the bloat of traditional NLEs, Shear leverages an optimised FFmpeg backend pipeline for rapid transcoding, thumbnail generation, and clip export.



---

## Key Features

* **Lightweight & High-Performance:** Built natively in C++ using Qt 6 for minimal memory overhead and fast startup.
* **FFmpeg Pipeline Integration:** Multi-threaded video transcoding, real-time frame extraction, and optimised clip export using direct FFmpeg execution.
* **Target-Size Encoding:** Automated 2-pass encoding with specific file-size targeting (e.g., optimised bitrates for messaging platforms and social uploads).
* **Power-User UI/UX:** Custom timeline and range-slider widgets with modal keyboard navigation for ultra-fast editing workflows.
* **Configurable Hotkeys:** Fully customizable keybindings for trimming, scrubbing, and rendering commands.

---

## Tech Stack

* **Language:** C++17 / C++20
* **GUI Framework:** Qt 6 (Qt Visual Studio Tools)
* **Video Engine:** FFmpeg (Transcoding, clip slicing, thumbnail generation)
* **IDE / Build System:** Visual Studio (MSVC)

---

## Architecture Overview

* **UI Layer:** Qt Widgets & Custom Sliders
* **Core Logic:** Controller & Keybind Engine
* **Processing:** FFmpeg Command Pipeline & Bitrate Calculator
* **Output Engine:** System Process / Transcoded Video Output

---

## Getting Started

### Prerequisites

* **IDE:** Visual Studio 2022 (with "Desktop development with C++" workload installed)
* **Extension:** Qt Visual Studio Tools
* **Framework:** Qt 6.x (configured within Visual Studio Qt Options)
* **System Dependency:** `ffmpeg` installed and available in your system PATH

### Building & Running

1. **Clone the Repository:**
   ```bash
   git clone [https://github.com/AlexMoga2004/Shear.git](https://github.com/AlexMoga2004/Shear.git)
