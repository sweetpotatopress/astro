astrology charts in the terminal/tty

![picture of the chart with the planet table open](https://i.postimg.cc/gjz4c9Xm/screenshot.png)

--xo-INSTALL--o-

```bash
sudo make clean install
```

run by typing `astro`

--o-DEPENDENCIES---

- swiss ephemeris (built with the makefile)
- ncurses
- GNU coreutil date
- /etc/localtime symlink to IANA timezone, to autofill.

basically, it should work on most linux distros, it may not work on systems
using musl, you are welcome to share patches in order to fix that //
send bug reports.
i am interested in it working with BSD and will hear bug reports.
i will not fix bug reports for winblows, wsl users or macos

--HELP WANTED--o-

writing a better planet collision offset that keeps them within their house
it's /usually/ ok right now, but gets worse the smaller the screen is
you can find it in astro.c function `planet_pos()`;

--o-KEYBINDS---o-

**NORMAL MODE** - esc

| | |
|---|---|
| navigation | hjkl |
| draw chart | enter |
| autofill systime | tab |

| | |
|---|---|
| save chart | w |
| choose dir | q |
| mkdir | m |
| cancel save | esc |
| load chart | e |
| choose file | l / enter |
| cancel | q |

**INSERT MODE** - i (on by default)

| | |
|---|---|
| navigation | arrowkeys |

**CHART VIEW**

| | |
|---|---|
| planet table | p |
| animate chart | enter |
| swap increment | h/l |
| ++ and -- | j/k |
| exit program | q |
| new chart | i |

**DATA**

charts are saved in `$XDG_DATA_HOME`
(`~/.local/share/astro/c`)

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
| declination/bounds table | |
| terminal window autoresize | |
| zodiacal releasing | |

feel free to suggest additions.

--license--

astro
AGPLv3

geonames - (city-db)
CC BY 4.0
https://creativecommons.org/licenses/by/4.0/
