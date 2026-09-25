--o https://sweetpotato.press --o  
~:o irc.libera.chat #astro  
--- --- --- --- --- --- ---   
![miles davis birth chart drawn in astro](https://sweetpotato.press/img/astro.png)

##### DEPENDENCIES  

ncurses  
iana timezone database  

##### KEYBINDS  

```
enter		animate chart  
	h/l 	time increment  
	k/j 	up/down  

0...9		select chart number
alt+0...9	synastry with selected chart
t			transits

s			solar return

z			zodiacal releasing
	k/j		up/down period
	h/l		up/down layer
	tab		switch lot
	
tab			live update  
d			toggle dst

r			redraw chart  
R			redraw config chart
q			exit astro  

o 			toggle right table
p			toggle left table

w			save chart  
	m		make dir  
	enter	choose dir
	esc		cancel  
	
e		 	load chart  
	l/enter	choose  
	esc		cancel  
 
i			input chart data  
	F1		clear fields  
	tab		fill local time  
	arrows	navigate  
	\		draw chart
	esc		cancel
```

##### DATA DIRECTORIES

$XDG_DATA_HOME/astro  
- city-db :   
- swisseph/ephe/ :  
- charts/ :  
 
OPTIONAL  
<sub>set default iana timezone and location</sub>

$XDG_CONFIG_HOME/astro  
- config :  

#### config template
```
timezone = America/Chicago  
latitude = 41.85003  
longitude = -87.65005  
```
TODO  
 
- [x] save and load charts
- [x] animate chart
- [x] lot of spirit + fortune
- [x] next retrograde/ next station
- [x] element colors
- [x] solar return
- [x] essential dignities
- [x] synastry/ transit view
- [ ] house system selection		
- [ ] secondary progressions
- [x] aspects
- [ ] planet day/hour
- [ ] primary directions
- [x] eclipse data
- [x] multiple charts
- [x] terminal window resize refresh
- [x] user default config
- [ ] code documentation
- [ ] astro tutorial
- [x] zodiacal releasing
 
##### LICENSE  
astro  
<sub>AGPLv3</sub>
 
swiss ephemeris  
<sub>https://github.com/aloistr/swisseph</sub>
 
geonames - (city-db)  
<sub>CC BY 4.0</sub>
