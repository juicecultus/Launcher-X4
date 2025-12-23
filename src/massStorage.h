#pragma once

#include "SD.h"
#include <Arduino.h>
#include <globals.h>

#define USB_VID 0x303a
#define USB_PID 0x1001

void drawUSBStickIcon(bool plugged);

// For XTEINK_X4 we stub out MassStorage (no native USB MSC)
#if defined(XTEINK_X4)
class MassStorage {
public:
    static inline bool shouldStop = false;
    MassStorage() = default;
    ~MassStorage() = default;
    void setup() {}
    void loop() {}
    static void setShouldStop(bool) {}
    static bool getShouldStop() { return false; }
};

inline int32_t usbWriteCallback(uint32_t, uint32_t, uint8_t *, uint32_t) { return -1; }
inline int32_t usbReadCallback(uint32_t, uint32_t, void *, uint32_t) { return -1; }
inline bool usbStartStopCallback(uint8_t, bool, bool) { return false; }
inline void MassStorage(void) {}

#elif defined(ARDUINO_USB_MODE)
#include <USB.h>

class MassStorage {
public:
    static bool shouldStop;

    MassStorage();
    ~MassStorage();
    void setup();
    void loop();
    static void setShouldStop(bool s) { shouldStop = s; }
    static bool getShouldStop() { return shouldStop; }

public:
    USBMSC msc;
    FATFS fatfs;
    uint8_t work[FF_MAX_SS];
    uint32_t pm_sector_size;
    uint8_t pm_part_type;
    uint8_t pm_part_subtype;
    uint32_t pm_abs_sector_offset;
    uint32_t pm_tot_sectors;
    uint32_t pm_reserved_sectors;
    uint32_t pm_table_sectors;
    uint32_t pm_fat_sectors;
    uint32_t pm_first_fat_sector;
    uint32_t pm_first_data_sector;
    uint32_t pm_root_dir_sectors;
    FATFS fs;
};

int32_t usbWriteCallback(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
int32_t usbReadCallback(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject);
void MassStorage(void);

#endif // ARDUINO_USB_MODE
