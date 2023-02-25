<img src="pics/speedybirdie_logo_small.png" align="right" />

## [![.github/workflows/main.yml](https://github.com/xeonqq/SpeedyBirdie/actions/workflows/main.yml/badge.svg)](https://github.com/xeonqq/SpeedyBirdie/actions/workflows/main.yml)
> An ESP8266 powered affordable open-source badmintion shuttlecock launcher, most of the parts are 3D printable. 
> It can be easily configured and controlled by mobile web app.


https://user-images.githubusercontent.com/4160429/221366746-91ebf04f-b197-4e0b-ada8-2b8b55a60042.mp4


> SSID: SpeedyBirdie

> password: chinasprung

> ip: 192.168.0.1

Main page (index.html)     |  Configuration page (dev.html)
:-------------------------:|:-------------------------:
![](pics/phone1.jpg)  |  ![](pics/phone2.jpg)

### Build and Flash (on Linux)
The project depends on https://github.com/SmingHub/Sming, therefore it is easier to use docker

#### Build
```bash
git clone https://github.com/xeonqq/SpeedyBirdie
cd SpeedyBirdie

# connect the esp8266 
docker-compose run --rm sming-cli
make
```

#### Flash
```bash
# when inside docker
# option1 flash app and upload website data: (flash for the first time)
make flash 

# option2 flash app only: (not the first time flash)
make flashapp
```

#### Debug
```bash
# when inside docker
make terminal # ctrl-[ to exit
```

#### Build Instruction & 3d printed parts
https://www.thingiverse.com/thing:5827863
