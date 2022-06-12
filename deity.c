#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


/* this is the key that signals an escape from
 * deity mode --- feel free to change it to any
 * valid x11 keysym */
#define DEITY_ESCAPE_KEY XK_Escape

/* this is define outside of the argument struct
   because it is used throughout later on independently */
enum deitymasks {
	CONTROLMASK,
	MOD1MASK, // alt
	MOD4MASK,
	SHIFTMASK,
	MODERR
};

/* xlib necessities */
static Display *dpy;
static Window root;
static unsigned int numlockmask = 0;

/* handle argp */
const char *argp_program_version = "deity 20220611-01";
const char *argp_program_bug_address = "<hcurfman@keemail.me>";
static char doc[] = "deity --- a daemon-esque way to simulate modifier keys";
static char args_doc[] = "<MODIFER>";
static struct argp_option options[] = {
	{"mode",     't', 0,            0, "Start deity in deity-mode."},
	{"state",    's', 0,            0, "Start deity in deity-state."},
	{"modifier", 'm', "<MODIFIER>", 0, "Modifier to prefix keys with."},
	{0}
};

struct arguments {
	enum {
		DEITY_MODE,
		DEITY_STATE,
		TYPERR
	} type;
	enum deitymasks modifier;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = state->input;
	
	/* match args */
	switch (key)
	{
	case 'm':
		/* set modifier */
		if (strcmp("Control", arg) == 0 || strcmp("ControlMask", arg) == 0)
			args->modifier = CONTROLMASK;
		else if (strcmp("Mod1", arg) == 0 || strcmp("Mod1Mask", arg) == 0)
			args->modifier = MOD1MASK;
		else if (strcmp("Mod4", arg) == 0 || strcmp("Mod4Mask", arg) == 0)
			args->modifier = MOD4MASK;
		else if (strcmp("Shift", arg) == 0 || strcmp("ShiftMask", arg) == 0)
			args->modifier = SHIFTMASK;
		break;
	case 's':
		/* deity state */
		args->type = DEITY_STATE;
		break;
	case 't':
		/* deity mode */
		args->type = DEITY_MODE;
		break;
	default:
		/* do nothing -- err will  throw somewhere else needbe */
		break;
	}
	
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };
struct arguments arguments;

/* function declarations */
static XKeyEvent createxev(XEvent sev, int type, unsigned int mask, Window win);
static void deity(void);
static void deitygrabkeys(void);
static unsigned int matchenumkeysym(void);
static void setup(void);
static void updatenumlockmask(void);
int main(int argc, char *argv[]);

/* returns the XEvent that is sent to a client */
XKeyEvent
createxev(XEvent sev, int type, unsigned int mask, Window win)
{
	/* check pointer */
	int root_x, root_y, win_x, win_y;
	unsigned int mask_return;
	Window root_return, child_window;
	XQueryPointer(dpy, win, &root_return, &child_window,
		      &root_x, &root_y, &win_x, &win_y, &mask_return);

	/* craft and return new event */
	XKeyEvent newev = {
		.type = type,
		.send_event = 1,
		.display = dpy,
		.window = win,
		.root = root,
		.subwindow = None,
		.time = CurrentTime,
		.x = win_x,
		.y = win_y,
		.x_root = root_x,
		.y_root = root_y,
		.state = mask,
		.keycode = sev.xkey.keycode,
		.same_screen = True
        };
	return newev;
}

/* deity mode/state function */
void
deity(void)
{
	XEvent ev;
	int exit = 0;
	int keysyms, i;

	deitygrabkeys();
	while (exit != 1)
	{
		updatenumlockmask();
		XNextEvent(dpy, &ev);
		if (ev.type == KeyPress)
		{
			KeySym *keysym = XGetKeyboardMapping(dpy, ev.xkey.keycode, 1, &keysyms);

			if (arguments.type == DEITY_STATE || keysym[0] != DEITY_ESCAPE_KEY) {
				XUngrabKey(dpy, AnyKey, AnyModifier, root);

				/* get focused window */
                Window winFocus;
				int revert;
				XGetInputFocus(dpy, &winFocus, &revert);

				/* create and send the key events */
				XKeyEvent newxev1 = createxev(ev, KeyPress, matchenumkeysym(), winFocus);
				XKeyEvent newxev2 = createxev(ev, KeyRelease, matchenumkeysym(), winFocus);
				XSendEvent(dpy, winFocus, True, KeyPressMask, (XEvent *)&newxev1);
				XSendEvent(dpy, winFocus, True, KeyReleaseMask, (XEvent *)&newxev2);
				XFlush(dpy);

				/* reset */
				deitygrabkeys();
			} else if (keysym[0] == DEITY_ESCAPE_KEY)
				exit = 1;
			if (arguments.type == DEITY_STATE)
				exit = 1;

			XFree(keysym);
		}
	}
}

/* grab keyboard for deity */
void
deitygrabkeys(void)
{
	unsigned int j;
	unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for (j = 0; j < MODERR; j++)
		XGrabKey(dpy, AnyKey, modifiers[j], root, False, GrabModeAsync, GrabModeAsync);
}

/* utility function, matches enum types to KeySyms */
unsigned int
matchenumkeysym(void)
{
	switch(arguments.modifier)
	{
	case CONTROLMASK:
		return ControlMask;
	case MOD1MASK:
		return Mod1Mask;
	case MOD4MASK:
		return Mod4Mask;
	case SHIFTMASK:
		return ShiftMask;
	default:
		return -1;
	}
}

/* setup the aforementioned xlib necessities */
void
setup(void)
{
	dpy = XOpenDisplay(NULL);
	int screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
}

/* shamelessly borrowed from dwm 6.2's source code */
void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

int main(int argc, char *argv[])
{
	setup();
	/* set defaults --- can't run without flags */
	arguments.type = TYPERR;
	arguments.modifier = MODERR;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	/* check to ensure flags have been set */
	if (arguments.modifier == MODERR)
		/* FIXME: should replace with argp_usage */
		exit(1);
	else if (arguments.type == TYPERR)
		/* FIXME: ^ */
		exit(1);
	
	deity();
	
	/* if we've made it here, everything likely hasn't burned down,
	 * so exit gracefully */
	return 0;
}
