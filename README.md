# randomwire 0.2

Terminal-based end-fed random wire antenna length screening utility. C11 and ncurses; intended for Linux and FreeBSD. It is not an EFHW or dipole calculator.

## Build

On Linux Mint/Ubuntu, install `build-essential` and `libncurses-dev` if missing. On FreeBSD, use the system C compiler, make and ncurses headers/libraries; override `CURSES_LIBS` or `CPPFLAGS` if needed. FreeBSD testing is pending.

```sh
make
make test
./randomwire
```

Optional system installation: `sudo make install` (or use root on FreeBSD). Default prefix `/usr/local`; `make install PREFIX=$HOME/.local` installs for one user. `make uninstall` with matching paths removes the installed files. `DESTDIR` supports staging. For FreeBSD, override `MANDIR=/usr/local/man/man1` if that is the local manual path.

## Operation

Use Up/Down to move, Space to toggle a band, A to select all, N to select none, C to calculate, W to analyze a wire, M to toggle quarter-wave enforcement, S for settings and Q to quit. Results are ranked by minimum resonance clearance. Up/Down scrolls results; Q or Enter returns. Settings are in-memory only. The main menu requires a terminal of at least 65 columns by 23 rows.

Defaults: 40–10 m including WARC bands, 20–150 ft, 5% tolerance, half-wave constant 468 ft MHz, quarter-wave minimum enabled. The 60 m envelope is a conservative modeling input, not a statement of authorized transmit frequencies. Band edges are fixed in the source.

## Model

For frequency f in MHz, half-wave multiples occur at n*K/f feet, where n is a positive integer and K defaults to 468 ft MHz. The program excludes intervals around these lengths across each selected frequency range, merges overlaps, and ranks remaining intervals by maximum worst-band fractional clearance. The optional minimum is K/(2*f_low), where f_low is the lowest selected band edge. This is a design heuristic, not a mandatory physical limit. The 5% margin is configurable and is not an empirically calibrated standard.

The model is intended for screening end-fed random wires used with a matching network, such as a 9:1 unun and tuner. It does not predict feed-point impedance, SWR, radiation efficiency, or tuner range. Wire geometry, insulation, counterpoise, ground, feed line and installation affect actual performance. Verify with an antenna analyzer. Do not treat displayed rounded interval endpoints as exact cutting limits.

## Validation

On devbox (Linux Mint 21.3), four built-in numerical regression tests passed twice. Interactive wire analysis at 67 and 71 ft passed its expected behavior checks for 40–10 m. Interactive calculation for 80–10 m including 60 m yielded one interval, 70.20–71.16 ft, optimum 70.655 ft, 5.68% clearance, limited by 40 m. FreeBSD and empirical analyzer tests remain pending. Packaging build/install verification is pending on devbox.

## References

- https://sprott.physics.wisc.edu/technote/randwire.htm
- https://udel.edu/~mm/ham/randomWire/indexOld.html

License selection pending; no license grant is implied by this package.
