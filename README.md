# ESP32 Model & Info Checker — README

A tiny, reliable sketch to print *everything useful* about the ESP32 it runs on, plus a readable explanation of each field and how to interpret the results. Use this to identify chip family (ESP32, ESP32-S2, S3, C3, H2, …), silicon revision, number of cores, flash size and more.

> ✅ Includes:
>
> * Ready-to-flash Arduino sketch (prints human-friendly info)
> * Decoding of esp_chip_info() model + feature flags
> * A list of known `esp_chip_model` values you may see (and other model names that can appear in newer SDKs)
> * Plain-language explanations of every parameter the sketch prints
> * Sample output and troubleshooting tips

---

## Quick start — the sketch

Upload this to your board (Arduino/PlatformIO with ESP32 board support) and open serial monitor at **115200**:

```cpp
// ESP32_info_printer.ino
#include <Arduino.h>
#include "esp_chip_info.h"
#include "esp_system.h"

String chipModelToString(esp_chip_model_t m) {
  switch (m) {
    case CHIP_ESP32:   return "ESP32 (Xtensa dual-core family)";
    case CHIP_ESP32S2: return "ESP32-S2 (single-core, USB-capable)";
    case CHIP_ESP32S3: return "ESP32-S3 (dual-core, vector/AI instr.)";
    case CHIP_ESP32C3: return "ESP32-C3 (RISC-V, low-power)";
    case CHIP_ESP32H2: return "ESP32-H2 (802.15.4 + BLE)";
    // Newer models may exist in newer ESP-IDF versions — list them if present:
#ifdef CHIP_ESP32C2
    case CHIP_ESP32C2: return "ESP32-C2";
#endif
#ifdef CHIP_ESP32C6
    case CHIP_ESP32C6: return "ESP32-C6";
#endif
#ifdef CHIP_ESP32C5
    case CHIP_ESP32C5: return "ESP32-C5";
#endif
#ifdef CHIP_ESP32P4
    case CHIP_ESP32P4: return "ESP32-P4";
#endif
    default: return String("Unknown model (enum=") + String((int)m) + ")";
  }
}

void printFeature(const char *name, bool enabled) {
  Serial.printf("  %-22s : %s\n", name, enabled ? "YES" : "no");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== ESP32 DETAILED INFO ===");
  Serial.println();

  // SDK / build info
  Serial.printf("ESP-IDF / SDK version : %s\n", esp_get_idf_version());

  // Chip info from esp_chip_info()
  esp_chip_info_t info;
  esp_chip_info(&info);

  Serial.printf("Chip model            : %s\n", chipModelToString(info.model).c_str());
  Serial.printf("Chip enum value       : %u\n", (unsigned)info.model);

  // features are a bitmask of CHIP_FEATURE_*
  Serial.printf("Feature flags (bits)  : 0x%08X\n", info.features);
  printFeature("Embedded flash",   info.features & CHIP_FEATURE_EMB_FLASH);
  printFeature("2.4GHz WiFi",     info.features & CHIP_FEATURE_WIFI_BGN);
  printFeature("Bluetooth LE",    info.features & CHIP_FEATURE_BLE);
  printFeature("Bluetooth Classic", info.features & CHIP_FEATURE_BT);
  printFeature("IEEE802.15.4 (Thread/Zigbee)", info.features & CHIP_FEATURE_IEEE802154);

  Serial.printf("CPU cores             : %u\n", info.cores);
  // NOTE: esp_chip_info_t has revision or full_revision depending on IDF
#ifdef CONFIG_IDF_TARGET_ESP32
  Serial.printf("Chip revision         : %u\n", info.revision); // simpler field name on older IDF
#else
  // Newer IDF may expose full_revision
  Serial.printf("Chip revision (full)  : %u\n", info.revision);
#endif

  // More common Arduino helpers
  Serial.printf("CPU frequency (MHz)   : %u\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash size (bytes)    : %u\n", ESP.getFlashChipSize());
  Serial.printf("Flash size (MB)       : %u\n", ESP.getFlashChipSize() / (1024UL*1024UL));

  // Other useful info
  Serial.printf("Reset reason          : %d\n", esp_reset_reason());
  Serial.printf("SDK build config IDF  : %s\n", esp_get_idf_version());

  // Print chip model helper from Arduino if available
#ifdef ESP_getChipModel
  Serial.printf("Arduino reported chip : %s\n", ESP.getChipModel());
#endif

  Serial.println();
  Serial.println("=== END ===");
}

void loop() {
  delay(10000);
}
```

---

## What the sketch prints (field-by-field explanation)

* **ESP-IDF / SDK version**
  The version string of the SDK used to build the firmware (e.g. `v5.1.4-...`). This tells you which Espressif SDK produced the binary. Useful to debug build/feature mismatches.

* **Chip model**
  Human-readable name mapped from the `esp_chip_info_t.model` enum (e.g. `ESP32`, `ESP32-S2`, `ESP32-S3`, `ESP32-C3`, `ESP32-H2`, etc). This is the *SoC family* — it tells you CPU architecture (Xtensa vs RISC-V), peripheral capabilities (Wi-Fi/Bluetooth/IEEE802.15.4), and feature expectations.

* **Chip enum value**
  The numeric enum value from the SDK (useful when cross-checking with a particular ESP-IDF version).

* **Feature flags (bitmask)**
  The raw bitmask showing built-in chip features. Typical flags (macros in `esp_chip_info.h`) you may see:

  * `CHIP_FEATURE_EMB_FLASH` — chip has embedded (on-chip) flash.
  * `CHIP_FEATURE_WIFI_BGN` — 2.4 GHz Wi-Fi support.
  * `CHIP_FEATURE_BLE` — Bluetooth Low Energy available.
  * `CHIP_FEATURE_BT` — Bluetooth Classic available.
  * `CHIP_FEATURE_IEEE802154` — IEEE 802.15.4 (Thread/Zigbee) radio present.
    The sketch decodes and prints these as YES/no.

* **CPU cores**
  Number of CPU cores available (usually 1 or 2). Example: classic ESP32 = 2 cores; ESP32-S2 = 1 core.

* **Chip revision**
  A silicon revision identifier (often numeric like `301` or `1`). This is a manufacturing revision; usually not critical for end-users but useful for low-level compatibility notes or bug reports.

* **CPU frequency (MHz)**
  Clock speed of the CPU (80 / 160 / 240 MHz etc). Printed from `ESP.getCpuFreqMHz()`.

* **Flash size (bytes / MB)**
  Size of external SPI flash chip as detected by the SDK. Important when selecting build/partition scheme — e.g. 4 MB is common.

* **Reset reason**
  Low-level numeric code returned by `esp_reset_reason()` (e.g., power-on, software reset, watchdog reset). Not always needed but useful when debugging boot loops.

* **Arduino reported chip** (if available)
  A convenience string from Arduino core functions, e.g. `ESP32` or `ESP32-D0WD-V3`.

---

## Known/expected `esp_chip_model_t` values

> NOTE: the exact set of enum values depends on the ESP-IDF version used to build firmware. New chip families are added to ESP-IDF over time. Below are the common values you will see across modern SDKs:

Core set (common across many IDF versions)

* `CHIP_ESP32` — Classic ESP32 (Xtensa) family (WROOM/WROVER modules).
* `CHIP_ESP32S2` — ESP32-S2 (single-core Xtensa; USB features).
* `CHIP_ESP32S3` — ESP32-S3 (Xtensa with vector extensions for AI).
* `CHIP_ESP32C3` — ESP32-C3 (RISC-V core; low-power).
* `CHIP_ESP32H2` — ESP32-H2 (supports IEEE 802.15.4 and BLE).

Other model names you may encounter in newer or specialized IDF releases (may appear as enums or via other macros):

* `ESP32-C2`, `ESP32-C5`, `ESP32-C6` — additional C-series variants (check your IDF for exact enum values).
* `ESP32-P4` / `ESP32-P6` / `ESP32-P` family — newer high-performance chips (if supported by your IDF).
* `ESP32-D0WD-V3` — lower-level chip marking; reported by some helpers as the chip part number (seen in `ESP.getChipModel()` output or log lines).

If your sketch reports `Unknown model (enum=X)`, it usually means your firmware was built with an SDK that has a model enum the sketch doesn't explicitly map — update the sketch mapping (or use the enum number) and check the ESP-IDF header `esp_chip_info.h` for the current enum list.

(References: esp_chip_info() docs and esp_chip_info.h in ESP-IDF.)

---

## Example outputs and what they mean

**Example A**

```
ESP-IDF / SDK version : v5.1.4-828-g33fbade6b8-dirty
Chip model            : ESP32 (Xtensa dual-core family)
Chip enum value       : 1
Feature flags (bits)  : 0x00000012
  Embedded flash      : no
  2.4GHz WiFi         : YES
  Bluetooth LE        : YES
  Bluetooth Classic   : YES
  IEEE802.15.4        : no
CPU cores             : 2
Chip revision         : 301
CPU frequency (MHz)   : 240
Flash size (MB)       : 4
Reset reason          : 1
Arduino reported chip : ESP32-D0WD-V3
```

Interpretation: Classic dual-core ESP32 with Wi-Fi and Bluetooth, 4 MB flash. Good for nRFBox.

**Example B**

```
Chip model: ESP32-C3 (RISC-V, low-power)
Feature flags: 0x00000010
  Embedded flash : no
  2.4GHz WiFi    : YES
  Bluetooth LE   : YES
  ...
CPU cores: 1
Flash size: 2 MB
```

Interpretation: ESP32-C3 (RISC-V, single core). Different peripheral/cross-compatibilities (e.g., different USB/ADC/RTC behavior vs classic ESP32).

---

## All ways to identify the module (short list)

1. **Run this sketch** — best, full software-detected info (chip model, revision, features).
2. **Look at the metal can on the module** — printed label often contains `ESP32-WROOM-32`, `ESP32-WROVER`, etc.
3. **Board docs / silkscreen** — devkit boards often have the model printed on the PCB.
4. **Arduino/PlatformIO Board selection** — the board selection gives flash size and chip family hints.
5. **Compare features** — missing Bluetooth, or single-core vs dual-core, quickly narrows the family.

---

## Troubleshooting & tips

* If flash size readout is wrong or 0: check wiring/power or that Arduino core is configured for the correct flash size in Tools / Board settings.
* If the sketch shows `Unknown model (enum=XX)`: your ESP-IDF used to compile the board supports a newer chip enum — update the mapping in the sketch or check `esp_chip_info.h` in your IDF version.
* Boot pins: some GPIOs (GPIO0, 2, 15, 12,  strapping pins) affect boot mode — avoid these for CE/CSN or buttons unless you know their effects.
* If `esp_get_idf_version()` contains `-dirty`: it means the SDK used to compile this firmware had local modifications at build time — nothing inherently wrong, but useful to know when debugging.

---

## Where the sketch reads things from (technical)

* `esp_chip_info(&info)` — low-level IDF API returning `esp_chip_info_t` (fields: `model`, `features`, `cores`, `revision`).
* `esp_get_idf_version()` — SDK build descriptor string.
* `ESP.getCpuFreqMHz()`, `ESP.getFlashChipSize()` — Arduino wrapper helpers for CPU freq & flash size.
* `esp_reset_reason()` — reset reason enum from IDF.

---

## Legal / Safety note (for nRFBox context)

Since you earlier mentioned building **nRFBox**, remember that *jamming*, spoofing or interfering with others’ RF communications may be illegal in many jurisdictions. Use hardware/software only on your own devices, in controlled lab environments, or with explicit permission.

