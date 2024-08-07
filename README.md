# k88 (cksmaid@rizon, ckatsmaid@libera)
everyone seems to have some terribly structured irc bot with unmaintainable code these days, this is my take on it

## features
at the core, its another attempt at a somewhat maintainable (for me) bot system with the following features:
- multithreaded
- module system
- multiple server support
- ini config
- self updating
- TLS v1.2

### modules
an ever incomplete list of the functionality provided through modules:
- ddg search (shoddy instant result api)
- ddg ai chat (non documented api eternally broken)
- wolfram alpha
- markov posting (learns on every channel its enabled on)
- onion link translating
- link reader
- 4chan scraper
- cross server bridging
- al search
- image saucing
- message triggers
- typical irc nonsense (decide, IBIP, fortunes, modulemanagement etc)

## building and running\*
```
git submodule init
git submodule update
make run
```
\*: it is not recommended to actually use this bot, the source is only public so other people can fix issues for me. if you, for whatever reason, want the bot in your channel contact me instead and I'll join my instance.

## incomplete guide to writing modules
module boilerplating is generated at `make` through the shellscript, all thats needed to add a new module is create a new directory under `mods/` with a `.c` file. see the other modules on how this is formatted.

### module contents
a module is structured as a bunch of `handle_` functions, and an `_init` function. inside the `_init` the module is added to the module system with its name and handler functions, any other initialisation the module might need should also be done here. again refer to all other mods on how this is structured.

### module 'api'
while everything is made to minimize having to look at the core bot code, a few things have to be known:
- `send_privmsg` for privmsgs
- `send_notice` for notices
- `log_info`, `log_err` for anything you want to log to console
- all of the above take printf like formatting, log functions expect a `\n` at the end
- for interfacing with api endpoints, see the `ddg` or `wolfram` module for how I (ab)use curl
- the signatures of all `handle_` functions are to be copied **1:1**, they're used by macros
- **do not** alter any strings your module is passed, these are reused by other modules
- it shouldnt matter too much, but your module code should be (mostly) threadsafe
- utils that will be reused over multiple modules are to be implemented in `utils/`

# FAQ
- why is this written so poorly?/why is x done like y when z makes more sense?\
I have given up. this is a combination of many dead bot projects I had laying around. I may or may not slowly clean up the most terrible parts over time. feel free to do it for me in PRs as well
