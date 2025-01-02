# Example for SPI NOR flash backed file system on Nuvoton's Mbed CE enabled boards

This is an example to demo file system backed by SPI NOR flash on Nuvoton's Mbed CE enabled boards.

Check out [Mbed CE](https://github.com/mbed-ce)
for details on Mbed OS community edition.

## Support development tools

Use cmake-based build system.
Check out [hello world example](https://github.com/mbed-ce/mbed-ce-hello-world) for getting started.

> **⚠️ Warning**
>
> Legacy development tools below are not supported anymore.
> - [Arm's Mbed Studio](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-studio.html)
> - [Arm's Mbed CLI 2](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-cli-2.html)
> - [Arm's Mbed CLI 1](https://os.mbed.com/docs/mbed-os/v6.15/tools/developing-mbed-cli.html)

For [VS Code development](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-VS-Code)
or [OpenOCD as upload method](https://github.com/mbed-ce/mbed-os/wiki/Upload-Methods#openocd),
install below additionally:

-   [NuEclipse](https://github.com/OpenNuvoton/Nuvoton_Tools#numicro-software-development-tools): Nuvoton's fork of Eclipse
-   Nuvoton forked OpenOCD: Shipped with NuEclipse installation package above.
    Checking openocd version `openocd --version`, it should fix to `0.10.022`.

## Developer guide

In the following, we take **NuMaker-IoT-M467** board as an example for Mbed CE support.

### Build the example

1.  Clone the example and navigate into it
    ```
    $ git clone https://github.com/mbed-nuvoton/NuMaker-mbed-ce-SPIF-FileSystem-example
    $ cd NuMaker-mbed-ce-SPIF-FileSystem-example
    $ git checkout -f master
    ```

1.  Deploy necessary libraries
    ```
    $ git submodule update --init
    ```
    Or for fast install:
    ```
    $ git submodule update --init --filter=blob:none
    ```

1.  Compile with cmake/ninja
    ```
    $ mkdir build; cd build
    $ cmake .. -GNinja -DCMAKE_BUILD_TYPE=Develop -DMBED_TARGET=NUMAKER_IOT_M467
    $ ninja
    $ cd ..
    ```

### Flash the image

Flash by drag-n-drop built image `NuMaker-mbed-ce-SPIF-FileSystem-example.bin` or `NuMaker-mbed-ce-SPIF-FileSystem-example.hex` onto **NuMaker-IoT-M467** board

### Verify the result

Configure your terminal program with **115200/8-N-1**, and you should see output similar to below:

```
--- Mbed OS filesystem example ---
Mounting the filesystem... lfs error:493: Corrupted dir pair at 0 1
lfs error:2222: Invalid superblock at 0 1
Fail :(
No filesystem found, formatting... OK
Opening "/fs/numbers.txt"... Fail :(
No file found, creating a new file... OK
Writing numbers (10/10)... OK
Seeking file... OK
Incrementing numbers (10/10)... OK
Closing "/fs/numbers.txt"... OK
Opening the root directory... OK
root directory:
    .
    ..
    numbers.txt
Closing the root directory... OK
Opening "/fs/numbers.txt"... OK
numbers:
    1
    2
    3
    4
    5
    6
    7
    8
    9
    10
Closing "/fs/numbers.txt"... OK
Unmounting... OK
Mbed OS filesystem example done!
```
