fast astrology charts in the terminal/tty

![picture of the chart with the planet table open](https://sweetpotato.press/astro.png)


in-development program with no stable version  

--xo-INSTALL--o-

```bash
doas/sudo make clean install
```

--o-DEPENDENCIES---

- swiss ephemeris (built with the makefile)
- ncurses

basically, it should work on most linux distros, it may not work on systems
using musl, you are welcome to share patches in order to fix that //
send bug reports.
in theory astro works on BSD. in theory . . 

--o-KEYBINDS---o-

**mode INSERT ( i )** 
 
navigation: 	arrowkeys  

**mode NORMAL ( esc )**  

navigation:    hjkl  
draw chart:		enter  
fill systime:	tab  

save chart:		w  
	--makedir:	m  
	--cancel:	esc  

load chart:		e  
	--choose:	l/enter  
	--cancel:	q  
 
**CHART VIEW**  

planet table:	p  
planet speed:	o  
animate chart:	enter  
	--increment:	h/l  
	--up/down:		j/k  
new chart:		i  
exit:			q  

**DATA**

 charts, city-db, swiss ephemeris $XDG_DATA_HOME  

 swiss ephemeris header files /usr/local/include  
 
 swiss ephemeris library /usr/local/lib  
 
 astro binary /usr/local/bin  
 
 
 **TODO**
 
 | | |
 |---|---|
 | save and load charts | [DONE] |
 | animate chart | [DONE] |
 | add birth data to chartview | [DONE] |
 | write makefile | [DONE] |
 | window with exact planet stats | [DONE] |
 | add nodes, outers | [DONE] |
 | add lot of spirit + fortune | [DONE] |
 | retrograde, station, speed table | |
 | add elemental colors | |
 | essential dignities table | |
 | aspects | |
 | eclipse calender | |
 | declination/out of bounds table | |
 | terminal window autoresize | |
 | zodiacal releasing | |
 
 feel free to suggest additions.
 
 --license--
 
 astro
 AGPLv3
 
 geonames - (city-db)
 CC BY 4.0

