# mines

A simple console-based minesweeper game for DOS.

Mines was written for DOS machines, as a fun exercise. Originally I compiled
it with OpenWatcom (since it was the best DOS compiler I could find that worked
on Windows, which I used when I wrote this), but it uses mostly standard C++,
so it should compile on any suitable compiler, on most systems. The only
issue you might encounter is that the some of the displayed characters use
CP 437, which you may want to change to suitable equivalents on other
systems.

There was a weird quirk I found when running this off a floppy disk on real
hardware, which I never actually resolved. This is the purpose of the
`REAL_MACHINE` flag, which just disables disk use.

Mines was written in October/November 2023.

## License

mines is licensed under th GNU General Public License v3. See LICENSE for
details.

Copyright © 2025, Nathan Gill
