fast astrology charts in the terminal/tty

![picture of astro in a small terminal window](https://sweetpotato.press/astro.png)

--xo-INSTALL--o-

```bash
git clone https://codeberg.org/yamlynn/astro.git  
doas/sudo make clean install
```

--o-DEPENDENCIES---

- ncurses

works on all linux/bsd systems. if not, tell me!  

FOUNDATION--0-  
  
built on the IANA time zone database, which has correct historical DST accuracy.
the choice: spend over 300 dollars on "professional" proprietary software with poor DST accuracy or pay 0 for free beautiful accuracy in all your charts by using astro~  ;3  

made with love, nyaa  

**--o-KEYBINDS---o-**  

**CHART VIEW**  

w - - - save chart  
	m --makedir  
	esc --cancel  
	--choose directory by hitting esc or q  
	--chart saves after writing name and hitting enter  
	
e - - - load chart  
	l/enter --choose  
	esc --cancel  
 
r - - - redraw chart  
tab - - - live update  
q - - - exit astro  

s - - - solar return  
k/j --up/down year  
s/q/esc --exit mode  

enter - - - animate chart  
	h/l --time increment  
	k/j --up/down  

p - - - toggle planet/point data  
o - - - toggle retro/speed data  

i - - - input chart data  

**i - - - INSERT mode**  

\ - - - exit data input  
F1 - - - clear fields  
tab - - - fill local time  
arrowkeys - - - navigate  

**esc - - - NORMAL mode**  

enter - - - draw chart  

w - - - save chart  
e - - - load chart  
q - - - exit data input  

k/j - - - up/down  
h/l - - - left/right  

-)(DATA--DIR_)--o  

 'charts' directory, city-db and swisseph 'ephe' directory: $XDG_DATA_HOME  

 swiss ephemeris header files: /usr/local/include  
 
 swiss ephemeris library 'libswe.a': /usr/local/lib  
 
 compiled astro binary: /usr/local/bin  
 
 create a file named 'config' in .config/astro to set open location  
 config template, tz is IANA format  
 ```
 timezone = America/Chicago
 latitude = 50.123456
 longitude = -100.123456
 ```
 
 **TODO**
 
 | | |
 |---|---|
 | save and load charts | [DONE] |
 | animate chart | [DONE] |
 | lot of spirit + fortune | [DONE] |
 | next retrograde/ next station | [DONE] |
 | element colors | [DONE] |
 | solar return | [DONE] |
 | essential dignities | |
 | synastry/ transit view | |
 | aspects | |
 | eclipse calender | |
 | declination/out of bounds | |
 | tabs/multiple charts open | |
 | terminal window resize refresh | [DONE] |
 | user default config | [DONE] |
 | code documentation | |
 | zodiacal releasing | |
 
 --license--
 
 astro
 AGPLv3
 
 swiss ephemeris 
 https://github.com/aloistr/swisseph
 
 geonames - (city-db)
 CC BY 4.0
 
 --0o-free and always will be-00o  
