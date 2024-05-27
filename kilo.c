/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

// CTRL key mask
#define CTRL_KEY(key) ((key) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
	// Clear the screen on exit
	write(STDOUT_FILENO, "\x1b[2j", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}

void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
	atexit(disableRawMode);

	// Disable the flags
	// Tunr off canonical mode
	struct termios raw = orig_termios;

	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | IXON);
	raw.c_oflag &= ~(OPOST);

	// Bit mask sets the chars size CS to 8 bits per byte
	raw.c_cflag |= (CS8);

	// Bitwise-NOT of ECHO flag and Bitwise AND result 00000000000...
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");

}

char editorReadKey() {
	int nread;
	char c;

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	
	return c;
}

/*** output ***/

void editorDrawRows() {
	int y;
	for (y = 0; y < 24; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);	
	}
}

void editorRefreshScreen() {
	// x1b is the escape character
	write(STDOUT_FILENO, "\x1b[2j", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	// Draw the tildes
	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3);
}


/*** input ***/


void editorProcessKeypress() {
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):	
			write(STDOUT_FILENO, "\x1b[2j", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}

/*** init ***/

int main() {
	enableRawMode();
	
	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}


