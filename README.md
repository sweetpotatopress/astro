fast astrology charts in the terminal/tty

![picture of the chart with the planet table open](https://i.postimg.cc/gjz4c9Xm/screenshot.png)

--xo-INSTALL--o-

```bash
sudo make clean install
```

--o-DEPENDENCIES---

- swiss ephemeris (built with the makefile)
- ncurses
- GNU coreutil date
- /etc/localtime symlink to IANA timezone, to autofill.

basically, it should work on most linux distros, it may not work on systems
using musl, you are welcome to share patches in order to fix that //
send bug reports.
in theory astro works on BSD. in theory . . 
please send issue reports for any and everything, id like this to work smoothly
and your suggestion may do so not just for you, but all! 

--o-KEYBINDS---o-

| mode INSERT (i) | mode NORMAL (esc) | CHART VIEW |
|---|---|---|
| arrowkeys | navigation: hjkl | planet table: p |
|| draw chart: enter | animate chart: enter |
|| autofill systime: tab | swap increment: h/l |
||  save chart: w | ++ and --: j/k |
||  choose dir: q | exit program: q |
||  mkdir: m | new chart: i |
||  cancel save: esc | |
||  load chart: e | |
||  choose file: l/enter | |
||  cancel: q | |

**DATA**

charts, city-db, and the swiss ephemeris
 are saved in `$XDG_DATA_HOME`
 
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
 https://creativecommons.org/licenses/by/4.0/
