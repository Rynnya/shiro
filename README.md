<img width="180" align="left" style="float: left; margin: 0 10px 0 0;" alt="Shiro" src="https://i.imgur.com/NuDj76W.png">

# Shiro

Shiro is a server-side implementation of the osu! cho protocol.
It delivers unmatched performance and features an accurate implementation
of the protocol.

It outperforms Ripple, Gigamons and even Bancho itself.
In terms of implementation accuracy, Shiro's implementation is
more accurate than both of Ripple and Gigamons.
<!-- TODO: Adding tests will make these words more weighty -->
<!-- TODO: Also this server cannot be faster than Bancho when using Bancho's API, ironic -->

Shiro also does not separate
Bancho and the Score Submission server.

# Future of project
With upcoming osu!lazer I'm will trying to recreate as many things as possible for me and my schedule<br>
Screenshot server might appear later, but definitely not now<br>
Current list of features that I'm want to implement:
```
reports ingame
achievements
beatmap rating system
2fa system
clan system (something like it's made on gatari?)
irc
anticheat (at least hwid and ip logger, also might be circleguard)
redis communication
```
And current list of features that I'm want to recreate:
```
multiplayer -> from single pool to single pool and pointer to handle for every user
thread pool -> find more ways to use it
```

## Features

Shiro handles all major aspects of a fully
functioning osu! server, which includes:

* Client login
* IRC-less chat & chat bot
* Leaderboards (Classic and Relax)
* Multiplayer (Even !mp commands!)
* Online user listing with their presence
* osu!direct (Supports [Beatconnect][8], [Cheesegull][9], [Hanaru][10] and mirrors)
* osu!tourney
* Rankings
* Replays
* Score submission
* Spectating

## Documentation

Documentation such as installation and usage of Shiro
are available on the [Wiki][1].<br>
There are also [FAQ's][2] here

## Docker [![Docker](https://img.shields.io/docker/pulls/marc3842h/shiro.svg?logo=docker)][3]

Shiro is available as ready-to-use Docker container, including its modules.
Please visit the [Wiki][4] for more information on how
to use the container.

## Contributing

We welcome all contributions to Shiro and its modules.

If you have found a issue with Shiro, feel free to [open a issue][5]
describing the issue in detail and how it occurred.

Code and documentation contributions can be made using
[Pull requests][6]. Please make sure you conform to the
general coding style of Shiro.

## License

Shiro is licensed under the GNU Affero General Public License,
version 3.0. Please visit the [LICENSE.txt][7] file for
the license and a summary of DOs & DON'Ts.

Third party libraries in the `src/thirdparty` may be
licensed under a different, but AGPL-3.0 compatible
license. Please visit the corresponding files and their
associated license for more information.

The branding `osu!` and `ppy` are protected by
trademark law. Fair use applies.

[1]: https://github.com/Marc3842h/shiro/wiki
[2]: https://github.com/Rynnya/shiro/blob/master/FAQ.md
[3]: https://hub.docker.com/r/marc3842h/shiro/
[4]: https://github.com/Marc3842h/shiro/wiki/Docker
[5]: https://github.com/Rynnya/shiro/issues/new
[6]: https://github.com/Rynnya/shiro/compare
[7]: https://github.com/Rynnya/shiro/blob/master/LICENSE.txt

[8]: https://beatconnect.io/
[9]: https://github.com/osuripple/cheesegull
[10]: https://github.com/Rynnya/hanaru
