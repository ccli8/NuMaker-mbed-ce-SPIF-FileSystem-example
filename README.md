# Example for SPI flash-backed Mbed OS file system 

This example is a clone of [mbed-os-example-filesystem](https://github.com/ARMmbed/mbed-os-example-filesystem)
and is slightly modified to demonstrate SPI flash-backed Mbed OS file system on Nuvoton Mbed-Enabled targets.

## Supported platforms
- [NuMaker-PFM-NUC472](https://developer.mbed.org/platforms/Nuvoton-NUC472/)
- [NuMaker-PFM-M453](https://developer.mbed.org/platforms/Nuvoton-M453/)
- [NuMaker-PFM-M487](https://developer.mbed.org/platforms/NUMAKER-PFM-M487/)
- [NuMaker-PFM-NANO130](https://os.mbed.com/platforms/NUMAKER-PFM-NANO130/)

## Configure SPI pins
You may choose on-board (if supportyed) or externally attached SPI flash
to back Mbed OS file system. To do that, we need to configure SPI pins in `mbed_app.json`.
If the default setting in `mbed_app.json` doesn't match, change it there.

In the following, we take **NuMaker-PFM-M487** board as an example for explanation:

### On-board SPI flash
If you want to try its on-board SPI flash, set `ONBOARD_SPIF` to true.
- The pins `ONBOARD_SPI_MOSI`/`ONBOARD_SPI_MISO`/`ONBOARD_SPI_CLK`/`ONBOARD_SPI_CS` are for on-board SPI flash.
  Don't change them because they are fixed on-board.
- The pins `SPI_MOSI`/`SPI_MISO`/`SPI_CLK`/`SPI_CS` are for externally SPI flash and won't be used.

<pre>
"NUMAKER_PFM_M487": {
    "target.macros_add": [
        "BUTTON1=SW2"
    ],
    <b>
    "ONBOARD_SPIF": true,
    "ONBOARD_SPI_MOSI": "PC_0",
    "ONBOARD_SPI_MISO": "PC_1",
    "ONBOARD_SPI_CLK":  "PC_2",
    "ONBOARD_SPI_CS":   "PC_3",
    </b>
    "SPI_MOSI": "D11",
    "SPI_MISO": "D12",
    "SPI_CLK":  "D13",
    "SPI_CS":   "D10"
},
</pre>

### Externally attached SPI flash
If you want to try externally attached SPI flash, unset `ONBOARD_SPIF` or set `ONBOARD_SPIF` to false.
- The pins `ONBOARD_SPI_MOSI`/`ONBOARD_SPI_MISO`/`ONBOARD_SPI_CLK`/`ONBOARD_SPI_CS` are for on-board SPI flash and won't be used.
- The pins `SPI_MOSI`/`SPI_MISO`/`SPI_CLK`/`SPI_CS` are for externally attached SPI flash.
  Change them if they don't match.

<pre>
"NUMAKER_PFM_M487": {
    "target.macros_add": [
        "BUTTON1=SW2"
    ],
    "ONBOARD_SPIF": false,
    "ONBOARD_SPI_MOSI": "PC_0",
    "ONBOARD_SPI_MISO": "PC_1",
    "ONBOARD_SPI_CLK":  "PC_2",
    "ONBOARD_SPI_CS":   "PC_3",
    <b>
    "SPI_MOSI": "D11",
    "SPI_MISO": "D12",
    "SPI_CLK":  "D13",
    "SPI_CS":   "D10"
    </b>
},
</pre>

## Trouble-shooting
- As of this writing, Arm's [spif-driver](https://github.com/ARMmbed/spif-driver) uses **Read Data (03h)** command
for read. This command has lower maximum allowed SPI clock rate than **Fast Read (0Bh)**.
Check the specification of your SPI flash of choice for this command's maximum allowed SPI clock rate.

- If you connect your SPI flash through fly-wire, you would meet errors easily with worse signal.
Try lowering the SPI clock rate.
    <pre>
    SPIFBlockDevice bd(MBED_CONF_APP_SPI_MOSI,
                    MBED_CONF_APP_SPI_MISO,
                    MBED_CONF_APP_SPI_CLK,
                    MBED_CONF_APP_SPI_CS,
                    <b>1000000</b>);
    </pre>

- Dependent on your SPI flash of choice and hardware circuit, besides SPI pins MOSI/MISO/CLK/CS,
you may need to configure other pins of your SPI flash. For example, to access **NuMaker-PFM-M487**'s
on-board SPI flash, we need additionally to configure its /WP and /HOLD pins to high because
we needn't its write-protect and hold functions.
    ```
    /* We needn't write-protect and hold functions. Configure /WP and /HOLD pins to high. */
    DigitalOut onboard_spi_wp(PC_5, 1);
    DigitalOut onboard_spi_hold(PC_4, 1);
    ```
