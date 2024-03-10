# Webserver-monitoring-system

## How to make Spiffs work

1. Configure in Platform.ini new Option: 
```
board_build.partitions = partitions.csv
```
2. Open Menu Config and go to **Partition Table**, change it to custom partition
3. To build the Spiffs partition binary from certain folder (here data) add following to the CmakeList.txt in your main folder:
```
spiffs_create_partition_image(spiffs data)
```
4. Use **Build Filesystem Image** and **Upload Filesystem Image** to upload

## How to make CallMeBot API work

1. Open menuconfig go to **Component config**
2. Open **ESP-TLS** and check **Allow potentially insecure options** and **Skip server certificate verification by default**

## How to use the project
We used Platformio and ESP-IDF Framework for our Project. Make sure that the Kconfig values are set. <br>
You can find them under the menuconfig and **Component config** <br>
All custom Kconfigs are listed at the bottom:
1. Mqtt Configurations
2. Whatsapp Messaging Configurations
3. Wifi Configurations

Dont forget to set the delay between the notifcations in the Kconfig for the Project <br>
You can find the settings under the menuconfig **Monitoring Project Configuration**

## Sources

Sources can be found in header files or on top of main.c file.There are also listed here:

- Spiffs: https://esp32tutorials.com/esp32-esp-idf-spiffs-web-server/
- Whatsapp Notification: https://esp32tutorials.com/esp32-esp-idf-send-messages-whatsapp/
- Wifi: https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c#L60
- Mqtt: https://github.com/espressif/esp-idf/blob/v5.0/examples/protocols/mqtt/tcp/main/app_main.c
- Callmebot-Api: https://www.callmebot.com/blog/free-api-whatsapp-messages/