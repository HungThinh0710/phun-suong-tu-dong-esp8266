# Hệ thống phun sương tự động - ESP8266

# Mist cooling automatic irrigation system by ESP8266

Hệ thống phun sương tự động, sử dụng cho cá nhân (Phi thương mại)

Mist cooling automatic irrigation system by ESP8266 (Non-Commercial)



<img width="200" alt="portfolio_view" src="https://github.com/HungThinh0710/phun-suong-tu-dong-esp8266/blob/master/screenshot.jpg?raw=true">

## Information

| Libraries        | Information                                   |
| ---------------- | --------------------------------------------- |
| ESP8266WebServer | Deploy a webserver and control by client-side |
| WiFiManager      | Automatic config wifi                         |
| LittleFS         | Read/Write file from by LITTLEFS filesystem   |
| NTPtimeESP       | Sync time from NTP server                     |

You can install there libs from `libs` folder or it's homepage.

### DataSheet

- Output PIN: **D0** - (GPIO16)

## Features 

- [x] See status `PIN D0` turn on or off.
- [x] Turn on mist cooling (Manual)
- [x] Turn of mist cooling (Manual)
- [x] Activate schedule task / Deactivate schedule task
- [x] Add new task into schedule list
- [x] Remove a task into schedule list
- [x] Remove all tasks 
- [ ] Ajax - In progress
- [ ] Reset by press external reset button - In progress

## Installation

### How to use ESP8266LittleFS 

Copy `ESP8266LittleFS` from `tools` folder to `C:\Users\<YourUser>\Documents\Arduino\tools` folder.

Example: `C:\Users\Phoenix\Documents\Arduino\tools\ESP8266LittleFS\tool\esp8266littlefs.jar`

### Upload necessary files into LittleFS storage (Your ESP8266)

Restart Adruino IDE and navigate to `Tools` > `ESP8266 LittleFS Data Upload` (Make sure `data` folder already in your sketch).

Notes: data files included in this sketch (`phunsuongtudong-phoenix/data` )

You can use EEPROM instead of LittleFS if you want. 

### Upload Code into ESP8266

Upload code from `phunsuongtudong-phoenix.ino` file to your esp8266 ( Find at `phunsuongtudong-phoenix` folder).

## FAQ 

> Q: Why don't I see `save`, `add`, `remove`, `clear` function in the server-side ? 
>
> A: Because I'm not goot at C program language and moreover, this is a project just for my house, so that I handle it by client-side (Javascript) and of course, your ESP8266 will be lite and less handler

> Q: So, how does `save`, `add`, `remove`, `clear` works ?
>
> A: Data will be handle by client's browser (I sure it enough performance to do it) and send it to esp8266. ESP8266 just need save data given by client-side and save it into the littleFS storage.

> Q: Can I use this project as an assignment or a project for a subject of school ?
>
> A: Sure, you can do anything if you want, pull this repository and develop it by your ideas (At this moment, I'm a student too)

> Q: Why client-side have only Vietnamese language ?
>
> A: Users of this project are my parents, so that's I must use mother tongue for them (You can contribute if you want).

**USE AT YOUR OWN RISK.**

##  © 2021 by Hưng Thịnh a.k.a Phoenix

