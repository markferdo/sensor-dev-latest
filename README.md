

# ESP-IDF NVS (Non-Volatile Storage) Overview  
Data gathered by Nischal Gautam, summarized by ChatGPT  
References:  
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_flash.html  
- https://github.com/espressif/esp-idf/blob/v5.5.1/examples/storage/nvs/nvs_rw_value/main/nvs_value_example_main.c  

---

# Summary

- NVS is key-value storage designed for many small persistent values.  
- Uses a dedicated flash partition and namespaces for isolation.  
- Writes are atomic.
- Freeing read buffers does not modify stored flash data.  
- Flash endurance limits apply, but NVS includes wear leveling.  
- Encryption is available and integrates with flash encryption.  
- Partition size and structure impact both flash and RAM usage.

---

# Overview of NVS (Non-Volatile Storage)

## What NVS Is
NVS is a key-value storage system located in flash memory on ESP32-series chips. It is designed for storing persistent configuration data.  
Properties:

- Data persists across reboot and power loss.  
- Uses a dedicated flash partition.  
- Intended for small configuration values.  

Supported value types:  
- Integer types (`uint8_t` to `int64_t`)  
- Zero-terminated strings  
- Binary blobs  

Keys must be ASCII and ≤15 characters long.

Key

    Zero-terminated ASCII string containing a key name. Maximum string length is 15 bytes, excluding a zero terminator.
Data

    For integer types, this field contains the value itself. If the value itself is shorter than 8 bytes, it is padded to the right, with unused bytes filled with 0xff.

## What NVS Is Used For

Common use cases include:  
- Wi-Fi credentials  
- Application configuration  
- Calibration data  
- User preferences  
- Device state

---

# Storage Model

## NVS Partition
- Defined in the project partition table.  
- Default size is ~20 KB.  
- Fully separate from your program, filesystem, and OTA partitions.  
- Can be expanded up to megabytes if needed.

## Partition Behavior
- Internally organized into pages containing entries.  
- Automatic compaction occurs when pages fill.  
- Errors like `ESP_ERR_NVS_NO_FREE_PAGES` indicate a full or incompatible partition layout.

---

# Data Model

## Keys and Values
Keys:
- ASCII-only  
- Max length: 15 characters  

Values:  
- Integer types  
- Strings  
- Blobs  

## Large Amounts of Data
NVS can store large data blocks but is not optimized for them.  
Large blobs:  
- Increase RAM usage  
- Increase flash wear  
- Reduce performance

---

# Namespaces

## What Namespaces Are
Namespaces provide isolation between sets of keys.  
Characteristics:

- Logical grouping of related key-value pairs  
- Max name length: 15 characters  
- Up to 254 namespaces per NVS partition  
- Same namespace name in different partitions = separate entities  

## How Namespaces Are Used
- Selected when calling `nvs_open()` or `nvs_open_from_partition()`  
- Returned handle is bound to that namespace  
- All reads/writes under that handle relate only to that namespace  

---

# Memory and RAM Usage

## Flash Space
- Every entry uses space rounded to page metadata.  
- Even small values incur overhead.  
- Multi-page values increase fragmentation.  
- More keys = more frequent compaction.

## RAM Usage
RAM usage scales with NVS partition size and number of keys:  
- ~22 KB of RAM per 1 MB NVS partition  
- ~5.5 KB RAM per 1000 keys  
Used for index structures enabling fast lookups.

---

# Writing and Reading Behavior

## Write Process
1. Call `nvs_open()`.  
2. Write data using `nvs_set_*()`.  
3. Call `nvs_commit()` to persist changes.  

Without `nvs_commit()`, no write occurs.

## Read Process
1. Call `nvs_open()` in R/O or R/W mode.  
2. Read using `nvs_get_*()`.  
3. For strings/blobs, first call to get required size, second call to retrieve data.  

Freeing allocated read buffers does **not** affect stored NVS contents.

## Atomicity
- All writes are atomic.  
- Old values remain valid if a write is interrupted.  
- New data is validated before replacing old data.

---

# Reliability and Endurance Considerations

## Flash Wear
- Flash sectors typically endure ~100k erase cycles.  
- NVS manages wear leveling internally.  
- Avoid using NVS for frequently updated data (logging, counters).  
- Large blobs cause more write amplification.

## Protection Against Mistakes
- You cannot write outside the NVS partition.  
- ESP-IDF enforces partition boundaries.  
- Program code and other partitions are protected unless the user misconfigures the partition table.

---

# Errors and Their Meaning

## `ESP_ERR_NVS_NO_FREE_PAGES`
Occurs when:  
- The partition is full and compaction cannot free space.  
- The layout changed and metadata is incompatible.  

Solution:  
- Erase the NVS partition (`idf.py erase_flash` or programmatic erase).

## `ESP_ERR_NVS_NEW_VERSION_FOUND`
Occurs when:  
- Stored NVS data uses an older internal format.  
- Data cannot be upgraded automatically.  

Solution:  
- Erase only the NVS partition.  
- Application and other partitions remain intact.

---

# Security and Encryption

## NVS Encryption
- Uses XTS-AES encryption.  
- Requires flash encryption (keys stored securely).  
- Prevents unauthorized reading or modification.  
- Erase operations are still allowed.

## Behavior
- Automatically enabled when flash encryption is active.  
- Ensures Wi-Fi credentials and sensitive data are protected.

---

# Data Isolation and Multi-Value Storage

## Handling Multiple Independent Blocks of Data

### 1. Different Keys
Examples:  
`test_config`, `deploy_config`  
Simple and effective.

### 2. Different Namespaces
Examples:  
`test` and `deploy`  
Prevents key collisions and keeps blocks separate.

### 3. Different NVS Partitions
Useful when:  
- Different erase cycles are required  
- Different encryption settings are required

## Preventing Overwrites
Keys are independent.  
Only identical key names in the same namespace can overwrite each other.

---

# Safety and Behavior on Partial/Failed Writes

## Safety Guarantees
- Writes are atomic.  
- Power interruption cannot corrupt existing values.  
- Invalid new entries are never committed.

## What Cannot Happen Accidentally
- You cannot overwrite other partitions.  
- You cannot break the chip or program by writing large amounts of data.  
- NVS remains stable unless flash wear limits are exceeded.
