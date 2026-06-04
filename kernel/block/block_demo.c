#include "mcsos/block.h"

/* M14: block layer demo initializer.
 * Storage bersifat static agar lifetime lebih panjang dari registry.
 * Dipanggil dari kmain setelah .bss valid dan sebelum filesystem memakai block layer.
 */

static unsigned char g_m14_ramdisk_storage[512u * 64u];
static mcsos_blk_device_t g_m14_ramdisk_dev;
static mcsos_ramblk_t g_m14_ramdisk;

void m14_block_demo_init(void) {
    mcsos_blk_registry_reset();
    if (mcsos_ramblk_init(&g_m14_ramdisk_dev,
                          &g_m14_ramdisk,
                          "ram0",
                          g_m14_ramdisk_storage,
                          sizeof(g_m14_ramdisk_storage),
                          512u) != MCSOS_BLK_OK) {
        return;
    }
    (void)mcsos_blk_register(&g_m14_ramdisk_dev);
}
