# Programming

## USB/UART

* Use 3.3V UART!
* Hold "BOOT" then plug in UART.
* `pio run -t upload`

## Over-the-Air (OTA)

1. Hold both buttons (GPIO 34 + 35) for 3 seconds. The display will show **99:99** to confirm OTA mode is active.
2. The clock will connect to WiFi and print its IP address to serial.
3. Upload: `pio run -t upload --upload-port <IP_ADDRESS>`
4. The clock restarts automatically after a successful update.
5. To exit OTA mode without updating, press the reset button or power-cycle.

Note: The first upload after enabling OTA support must be done over USB, since the partition table changes.
