fast astrology charts in the terminal/tty

![picture of the chart with the planet table open](https://sweetpotato.press/astro.png)

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
	--choose the directory to save by hitting esc or q
	--chart saves after writing name and hitting enter

load chart:		e  
	--choose:	l/enter  
	--cancel:	esc   
 
**CHART VIEW**  

save chart: w  
load chart: e  
redraw chart: r  

solar return: s  
--up/down k/j  
--exit s/q/esc  

animate chart:	enter  
	--increment:	h/l  
	--up/down:		k/j  

planet table:	p  
retrograde table:	o  

realtime chart: tab  

new chart:		i  
exit program: q  

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
 | next retrograde/ next station | [DONE] |
 | add elemental colors | [DONE] |
 | essential dignities table | |
 | synastry/ transit view | |
 | aspects | |
 | eclipse calender | |
 | declination/out of bounds table | |
 | terminal window resize refresh | [DONE] |
 | user default config file | |
 | zodiacal releasing | |
 
 --license--
 
 astro
 AGPLv3
 
 swiss ephemeris 
 https://github.com/aloistr/swisseph
 
 geonames - (city-db)
 CC BY 4.0
 
 --0o-free and always will be-00o  
 
