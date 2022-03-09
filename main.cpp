/* mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed.h"
#include <stdio.h>
#include <errno.h>

#include "SPIFBlockDevice.h"

// Maximum number of elements in buffer
#define BUFFER_MAX_LEN 10

/* Disable write-protect (/WP) and hold (/HOLD) functions
 *
 * Excerpt on QE bit of Winbond SPI Flash:
 *
 * The Quad Enable (QE) bit is a non-volatile read/write bit in the status
 * register (S9) that enables Quad SPI operation. When the QE bit is set to
 * a 0 state (factory default for part numbers with ordering options “IM”),
 * the /HOLD are enabled, the device operates in Standard/Dual SPI modes.
 * When the QE bit is set to a 1 (factory fixed default for part numbers with
 * ordering options “IQ”), the Quad IO2 and IO3 pins are enabled, and /HOLD
 * function is disabled, the device operates in Standard/Dual/Quad SPI modes.
 *
 * So that we need to disable write-protect and hold functions by driving /WP
 * and /HOLD pins to high if QE bit is not set.
 */
#if defined(TARGET_NUMAKER_IOT_M467)
/* Can comment out the below lines if QE bit is set, e.g. W25Q32JVSSIQ, or keep them for safe */
DigitalOut onboard_spi_wp(PI_13, 1);
DigitalOut onboard_spi_hold(PI_12, 1);
#elif defined(TARGET_NUMAKER_PFM_M487) || defined(TARGET_NUMAKER_IOT_M487)
DigitalOut onboard_spi_wp(PC_5, 1);
DigitalOut onboard_spi_hold(PC_4, 1);
#endif

BlockDevice *bd = new SPIFBlockDevice(MBED_CONF_SPIF_DRIVER_SPI_MOSI,
                                      MBED_CONF_SPIF_DRIVER_SPI_MISO,
                                      MBED_CONF_SPIF_DRIVER_SPI_CLK,
                                      MBED_CONF_SPIF_DRIVER_SPI_CS);

// This example uses LittleFileSystem as the default file system
#include "LittleFileSystem.h"
LittleFileSystem fs("fs");

// Uncomment the following two lines and comment the previous two to use FAT file system.
// #include "FATFileSystem.h"
// FATFileSystem fs("fs");

// Support bare-metal build in which RTOS will be absent
#if MBED_CONF_RTOS_PRESENT
// Set up the button to trigger an erase
InterruptIn irq(BUTTON1);
void erase() {
    printf("Initializing the block device... ");
    fflush(stdout);
    int err = bd->init();
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Erasing the block device... ");
    fflush(stdout);
    err = bd->erase(0, bd->size());
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Deinitializing the block device... ");
    fflush(stdout);
    err = bd->deinit();
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
}
#endif

#if MBED_CONF_RTOS_PRESENT
#if MBED_MAJOR_VERSION >= 6
static auto erase_event = mbed_event_queue()->make_user_allocated_event(erase);
#endif
#endif

// Entry point for the example
int main() {
    printf("--- Mbed OS filesystem example ---\n");

#if MBED_CONF_RTOS_PRESENT
    // Setup the erase event on button press, use the event queue
    // to avoid running in interrupt context
#if MBED_MAJOR_VERSION >= 6
    irq.fall(std::ref(erase_event));
#else
    irq.fall(mbed_event_queue()->event(erase));
#endif
#endif

    // Try to mount the filesystem
    printf("Mounting the filesystem... ");
    fflush(stdout);
    int err = fs.mount(bd);
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        // Reformat if we can't mount the filesystem
        // this should only happen on the first boot
        printf("No filesystem found, formatting... ");
        fflush(stdout);
        err = fs.reformat(bd);
        printf("%s\n", (err ? "Fail :(" : "OK"));
        if (err) {
            error("error: %s (%d)\n", strerror(-err), err);
        }
    }

    // Open the numbers file
    printf("Opening \"/fs/numbers.txt\"... ");
    fflush(stdout);
    FILE *f = fopen("/fs/numbers.txt", "r+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
        // Create the numbers file if it doesn't exist
        printf("No file found, creating a new file... ");
        fflush(stdout);
        f = fopen("/fs/numbers.txt", "w+");
        printf("%s\n", (!f ? "Fail :(" : "OK"));
        if (!f) {
            error("error: %s (%d)\n", strerror(errno), -errno);
        }

        for (int i = 0; i < 10; i++) {
            printf("\rWriting numbers (%d/%d)... ", i, 10);
            fflush(stdout);
            err = fprintf(f, "    %d\n", i);
            if (err < 0) {
                printf("Fail :(\n");
                error("error: %s (%d)\n", strerror(errno), -errno);
            }
        }
        printf("\rWriting numbers (%d/%d)... OK\n", 10, 10);

        printf("Seeking file... ");
        fflush(stdout);
        err = fseek(f, 0, SEEK_SET);
        printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
        if (err < 0) {
            error("error: %s (%d)\n", strerror(errno), -errno);
        }
    }

    // Go through and increment the numbers
    for (int i = 0; i < 10; i++) {
        printf("\rIncrementing numbers (%d/%d)... ", i, 10);
        fflush(stdout);

        // Get current stream position
        long pos = ftell(f);

        // Parse out the number and increment
        char buf[BUFFER_MAX_LEN];
        if (!fgets(buf, BUFFER_MAX_LEN, f)) {
            error("error: %s (%d)\n", strerror(errno), -errno);
        }
        char *endptr;
        int32_t number = strtol(buf, &endptr, 10);
        if (
            (errno == ERANGE) || // The number is too small/large
            (endptr == buf) ||   // No character was read
            (*endptr && *endptr != '\n') // The whole input was not converted
        ) {
            continue;
        }
        number += 1;

        // Seek to beginning of number
        fseek(f, pos, SEEK_SET);
    
        // Store number
        fprintf(f, "    %d\n", number);

        // Flush between write and read on same file
        fflush(f);
    }
    printf("\rIncrementing numbers (%d/%d)... OK\n", 10, 10);

    // Close the file which also flushes any cached writes
    printf("Closing \"/fs/numbers.txt\"... ");
    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }
    
    // Display the root directory
    printf("Opening the root directory... ");
    fflush(stdout);
    DIR *d = opendir("/fs/");
    printf("%s\n", (!d ? "Fail :(" : "OK"));
    if (!d) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }

    printf("root directory:\n");
    while (true) {
        struct dirent *e = readdir(d);
        if (!e) {
            break;
        }

        printf("    %s\n", e->d_name);
    }

    printf("Closing the root directory... ");
    fflush(stdout);
    err = closedir(d);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }

    // Display the numbers file
    printf("Opening \"/fs/numbers.txt\"... ");
    fflush(stdout);
    f = fopen("/fs/numbers.txt", "r");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }

    printf("numbers:\n");
    while (!feof(f)) {
        int c = fgetc(f);
        printf("%c", c);
    }

    printf("\rClosing \"/fs/numbers.txt\"... ");
    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }

    // Tidy up
    printf("Unmounting... ");
    fflush(stdout);
    err = fs.unmount();
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
        
    printf("Mbed OS filesystem example done!\n");
}

