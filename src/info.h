#include <Arduino.h>

// DEVICE DEFINITION
String DEVICE_ID = "d1ab20d6-aebc-443e-9631-fc5ca9fabb94";
String DEVICE_NAME = "IOT TEST DEVICE";
String DEVICE_TYPE = "switch";
String DEVICE_DESCRIPTION = "Test switch.";
String ASCII = "\r\n\r\n"
" ___   _     _____     _______   ____   ___  ____   ___ _____ ____  _\n"
"|_ _| | |   / _ \\ \\   / / ____| |  _ \\ / _ \\| __ ) / _ \\_   _/ ___|| |\n"
" | |  | |  | | | \\ \\ / /|  _|   | |_) | | | |  _ \\| | | || | \\___ \\| |\n"
" | |  | |__| |_| |\\ V / | |___  |  _ <| |_| | |_) | |_| || |  ___) |_|\n"
"|___| |_____\\___/  \\_/  |_____| |_| \\_\\\\___/|____/ \\___/ |_| |____/(_)\n";

String INFO() {
  String info = ""
    "id=" + DEVICE_ID + "&"
    "name=" + DEVICE_NAME + "&"
    "type=" + DEVICE_TYPE + "&"
    "description=" + DEVICE_DESCRIPTION;

  return info;
}
