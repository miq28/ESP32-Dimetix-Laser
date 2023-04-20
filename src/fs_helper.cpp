#include "fs_helper.h"
#include "asyncserver.h"

void fs_setup()
{
// -------------------------------------------------------------------
// Mount File system
// -------------------------------------------------------------------
#define FORMAT_LITTLEFS_IF_FAILED false
    DEBUG("Mounting FS...\n");

#ifdef USE_FatFS
    if (MYFS.begin(false, "/ffat", 3)) // limit the RAM usage, bottom line 8kb + 4kb takes per each file, default is 10
#else
    if (MYFS.begin())
#endif
    {
        DEBUG("FS mounted\n");

        DEBUG("totalBytes: %d\r\n", MYFS.totalBytes());
        DEBUG("usedBytes: %d\r\n", MYFS.usedBytes());

        display_srcfile_details();

        save_system_info();

        if (!load_config_network())
        { // Try to load configuration from file system
            // defaultConfig(); // Load defaults if any error

            save_config_network();

            _configAP.APenable = true;
        }
    }
    else
    {
        DEBUG("FS mount failed\n");
    }
}