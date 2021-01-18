# k88 (cksmaid@rizon, ckatsmaid@freenode)
everyone seems to have some terribly structured irc bot with unmaintainable code these days, this is my take on it

## features
at the core, its another attempt at a somewhat maintainable (for me) bot system with the following features:
- multithreaded
- module system
- multiple server support
- ini config
- self updating
- SSL (and only SSL connections will work)

### modules
an ever incomplete list of the functionality provided through modules:
- ddg search (shoddy instant result api)
- wolfram alpha
- markov posting (learns on every channel its enabled on)
- onion link translating
- link reader
- 4chan scraper
- typical irc nonsense (decide, IBIP, fortunes, modulemanagement etc)

## building and running\*
```
git submodule init
git submodule update
make
./k88 # this is fine for testing, dont bother with the helper scripts
```
\*: it is not recommended to actually use this bot, the source is only public so other people can fix issues for me. if you, for whatever reason, want the bot in your channel contact me instead and I'll join my instance.

## incomplete guide to writing modules
module boilerplating is generated at `make` through the shellscript, all thats needed to add a new module is create a new directory under `mods/` with a `.c` file. see the other modules on how this is formatted.

### module contents
a module is structured as a bunch of `handle_` functions, and an `_init` function. inside the `_init` the module is added to the module system with its name and handler functions, any other initialisation the module might need should also be done here. again refer to all other mods on how this is structured.

### module 'api'
while everything is made to minimize having to look at the core bot code, a few things have to be known:
- use `send_privmsg` for unformatted privmsgs
- use `send_fprivmsg` for a privmsg with formatted(as in printf like) privmsg
- similar for `send_notice`/`send_fnotice`
- for interfacing with api endpoints, see the `ddg` or `wolfram` module for how I (ab)use curl
- the signature of all `handle_` functions are to be copied **1:1**, they're used by macros
- **do not** alter any strings your module is passed, these are reused by other modules
- it shouldnt matter too much, but your module code should be (mostly) threadsafe
- utils that will be reused over multiple modules are to be implemented in `utils/`

# FAQ
- why is this written so poorly?/why is x done like y when z makes more sense?\
I have given up. this is a combination of many dead bot projects I had laying around. I may or may not slowly clean up the most terrible parts over time. feel free to do it for me in PRs as well
