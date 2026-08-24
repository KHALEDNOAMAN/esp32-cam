# ESP32-CAM Wiring & Setup Guide

## ESP32-CAM Pinout
```
                 ┌─────────────┐
           3V3 ──┤ 1        16 ├── GND
           GND ──┤ 2        15 ├── GPIO 13 (HS2_DATA3)
       GPIO 12 ──┤ 3        14 ├── GPIO 14 (HS2_CLK)
  (SD_DATA2) 4 ──┤ 4        13 ├── GPIO 15 (HS2_CMD)
  (SD_DATA3) 2 ──┤ 5        12 ├── GPIO 2 (SD_DATA0)
       GPIO  4 ──┤ 6 (FLASH) 11├── GPIO 4 (SD_DATA1)
            5V ──┤ 7        10 ├── GPIO 0 (BOOT)
    (U0TXD) 1 ──┤ 8         9 ├── GPIO 16 (U2RXD)
    (U0RXD) 3 ──┤ 9         8 ├── GND
                 └─────────────┘
```

## Wiring to FTDI Programmer
| ESP32-CAM | FTDI |
|-----------|------|
| 5V | VCC (5V) |
| GND | GND |
| U0R (GPIO 3) | TX |
| U0T (GPIO 1) | RX |
| GPIO 0 | GND (for flash mode) |

> **Important**: Remove GPIO 0 → GND connection after flashing!

## SD Card Setup
- Use **FAT32** formatted SD card (max 32GB recommended)
- Insert with contacts facing the PCB
- If SD fails, check GPIO 4 (shared with flash LED)

## Telegram Bot Setup
1. Message @BotFather on Telegram → `/newbot`
2. Save the bot token
3. Get your chat ID: message @userinfobot
4. Add to `config.h`:
   ```c
   #define BOT_TOKEN "your_token_here"
   #define CHAT_ID "your_chat_id"
   ```

## Common Issues
| Issue | Cause | Fix |
|-------|-------|-----|
| Brownout | Insufficient power | Use 5V 2A supply |
| Camera init failed | Loose ribbon cable | Reseat carefully |
| SD mount failed | Wrong format | Format FAT32, <32GB |
| Blurry images | Focus ring | Twist lens gently |
