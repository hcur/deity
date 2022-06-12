# deity

deity is a little program I wrote to simulate emacs' [god-mode](https://github.com/emacsorphanage/god-mode)
at a text-editor agnostic level. To use it, simply:

	deity {-s, -t} {-m, --modifier} <X11 Key Mask>

example:

	deity -s -m ControlMask

where:

	-s: shortened version of --state; one keypress is in "deity mode", then exit
	-m: shortened version of --modifier: specify which mask to use
	ControlMask: an Xlib key mask, specifies to append C- to any incoming key

Therefore, the command above when ran would wait for user input, intercept the
key (an "x" keypress, for example), and return C-x to the focused window.
For more precise information as to how deity works, look over the `--help` output
below or peruse the source---deity is ~250 loc with comments, and I like to think
it's somewhat readable. :)

## usage

```
Usage: deity [OPTION...] <MODIFER>
deity --- a daemon-esque way to simulate modifier keys

	-m, --modifier=<MODIFIER>  Modifier to prefix keys with.
	-s, --state                Start deity in deity-state.
	-t, --mode                 Start deity in deity-mode.
	-?, --help                 Give this help list
	--usage                    Give a short usage message
	-V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <hcurfman@keemail.me>.
```

## install

deity comes with a Makefile, so just:

	git clone https://github.com/hydra989/deity
	cd deity
	sudo make install

## todo

- ~~support for multiple modifiers~~
   - deity allows adding a mask to a keypress with modifier
   - next support passing multiple modifiers into deity
- better error handling
- verbose mode?
