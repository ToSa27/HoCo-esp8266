# HoCo-esp8266
Home Control Framework - ESP8266 implementation

See subdirectories for additional details and/or licenses for include components.

Installation:
- copy "config_template.mk" to "config.mk" and adjust based on your needs
- make "tools" once to build the tools (for now only esptool2)
- connect ESP8266 via serial port
- make "flash"

Right now building and flashing bootloader only - as serial is meant for debugging only, I kept it at 76800 baud. Start your favorite serial monitor or use coolterm included in the tools directory to see the bootloader in action. Result should be similar to this for now:

```
 ets Jan  8 2013,rst cause:2, boot mode:(3,7)

load 0x40100000, len 1856, room 16 
tail 0
chksum 0x29
load 0x3ffe8000, len 692, room 8 
tail 12
chksum 0x1e
csum 0x1e

HoCo bootloader v1.1-7
Writing default bootloader config.
latest factory/firmware: 0/0
No valid config - expecting factory at slot 1
Checking factory rom in slot 1
Rom in slot 1 is bad.
No valid rom found.
user code done
```
