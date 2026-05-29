fast astrology charts in the terminal/tty

![picture of the chart with the planet table open](https://sweetpotato.press/astro.png)


in-development program with no stable version  

--xo-INSTALL--o-

```bash
git clone https://codeberg.org/yamlynn/astro.git  
doas/sudo make clean install
```

--o-DEPENDENCIES---

- ncurses

works on all linux/bsd systems. if not, tell me!  
not interested in other OS at this time.  


FOUNDATION--0-  
  
built on the IANA time zone database, which has correct historical DST accuracy.
the choice: spend over 300 dollars on "professional" proprietary software with poor DST accuracy or pay 0 for free beautiful accuracy in all your charts by using astro~  ;3  

made with love, nyaa  

--o-KEYBINDS---o-

**mode INSERT ( i )** 
 
navigation: 	arrowkeys  
clear fields	F1  

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
realtime chart: tab  
animate chart:	enter  
	--increment:	h/l  
	--up/down:		j/k  
new chart:		i  
exit:			q  

-)(DATA--DIR_)--o  

 'charts' directory, city-db and swisseph 'ephe' directory: $XDG_DATA_HOME  

 swiss ephemeris header files: /usr/local/include  
 
 swiss ephemeris library 'libswe.a': /usr/local/lib  
 
 compiled astro binary: /usr/local/bin  
 
 
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
 | next/last retrograde and station | |
 | add elemental colors | [DONE] |
 | essential dignities table | |
 | synastry/ transit view | |
 | aspects | |
 | eclipse calender | |
 | declination/out of bounds table | |
 | terminal window autoresize | |
 | user default config file | |
 | zodiacal releasing | |
 
 feel free to suggest additions and send pull requests. big or small!
 
 --license--
 
 astro
 AGPLv3
 
 swiss ephemeris 
 https://github.com/aloistr/swisseph
 
 geonames - (city-db)
 CC BY 4.0
 
 --0o-free and always will be-00o  
 

