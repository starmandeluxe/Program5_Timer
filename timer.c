/*! \file
*
* Alex Kort, 5-21-06, Program 5, timer.c
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <sys/time.h>
#include <ctype.h>


/****
 *
 * This program is a simple example of using setitimer and sigaction to
 * generate and handle SIGALRM signals.  The program sets up an interval timer
 * that generates a SIGALRM once a second.  The SIGALRM handler simply
 * increments a signal counter and prints out its value.
 *
 * Before starting the timer, the program puts the terminal in "raw mode".
 * This means that it does not echo input characters, wait for a newline to
 * complete an input, or respond to interrupt characters.
 *
 * After starting the timer and setting up the handler, the main function waits
 * for the user to type the character 'q' to quit.  Since the terminal is in
 * raw mode, typing 'q' is the only way to stop, since all other printable
 * characters are ignored, and terminal interrupt characters are disabled.
 *
 */

#define TICKS_PER_SECOND 40
/**
 * The ticks variable counts the number of one-second ticks since the program
 * started.
 */
static int ticks = 0;
static int minute = 0;
static int hour = 0;
char neg = ' ';
int isNeg = 0;
char* twirler = "|/-\\";
int ind = 0;
int numMicrosecs = 0;
int printBeep = 0;
int paused = 0;

/**
 * The tick function is the handler for SIGALRM.
 */
void tick(int sig) 
{
	///do nothing if paused
	if (paused == 1)
		return;
	///cycle the twirler string
	if (ind == 4)
	{
		ind = 0;
	}
	///if the time is up print a message	
	if (ticks == 0 && minute == 0 && hour == 0 && printBeep == 0)
	{
		printBeep = 1;
		printf("%cBeeeep! Time's up!\n", 7);
		fflush(stdin);
	}
	///change between negative and positive time as necessary
	if ((ticks < 0 && minute <= 0 && hour <= 0) ||  (minute < 0 && hour <= 0) ||  hour < 0)
	{
		neg = '-';
		isNeg = 1;
	}
	else if (hour >= 0)
	{
		neg = ' ';
		isNeg = 0;
	}
	///count to one second
        numMicrosecs = numMicrosecs + 10;
        if (numMicrosecs == 1000)
	{
	   numMicrosecs = 0;
	   --ticks;
	   printBeep = 0;
	   ///handle the time as each second passes
	   if (isNeg == 0)
	   {
		   if (ticks < 0)
		   {
		   	if (minute > 0 || hour > 0)
		   	{
		   		ticks = 59;
		   		--minute;
		   	}
		   }
		   else if (ticks >= 60)
		   {
		   	ticks = 0;
		   	--minute;
		   }
		   if (hour > 0 && minute < 0)
		   {
		   	hour--;
		   	minute = 59;
		   }
	   }
	   else
	   {
	   	   if (ticks >= 0)
		   {
		   	if (minute > 0 || hour > 0)
		   	{
		   		ticks = -59;
		   		--minute;
		   	}
		   }
		   else if (ticks <= -60)
		   {
		   	ticks = 0;
		   	--minute;
		   }
		   if (hour <= 0 && minute <= -60)
		   {
		   	hour--;
		   	minute = 0;
		   }
	   }
	   
	   if (ticks%10 == 0)
	   	printf("%c %c%d:%.2d%:%.2d     \r", twirler[ind++], neg, abs(hour), abs(minute), abs(ticks));
	   	fflush(stdout);
        }
        else
	{
	   ///move the twirler at 1/25 of a second
	   if (numMicrosecs%TICKS_PER_SECOND == 0)
	   	printf("%c %c%d:%.2d%:%.2d     \r", twirler[ind++], neg, abs(hour), abs(minute), abs(ticks));
	   	fflush(stdout);
	}
}

/**
 * Set the terminal to "raw" input mode.  This is defined by setting terminal
 * control flags to noncanonical mode, turning off character echoing, and
 * ignoring signaling characters.  Before setting to raw mode, save the
 * current mode so it can be restored later.  After setting, return the
 * saved mode.
 *
 * For explanatory details, see Sections 18.10, 18.11 of Stevens, the
 * termio(7I) man page, and the tcsetattr(3C) man page.  (To see a particular
 * man page section, use the "-s" argument, e.g., "man -s 7I termio" on
 * falcon/hornet.)
 */
struct termios set_raw_term_mode() {
    struct termios cur_term_mode, raw_term_mode;

    tcgetattr(STDIN_FILENO, &cur_term_mode);
    raw_term_mode = cur_term_mode;
    raw_term_mode.c_lflag &= ~(ICANON | ECHO | ISIG) ;
    raw_term_mode.c_cc[VMIN] = 1 ;
    raw_term_mode.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term_mode);

    return cur_term_mode;
}

/**
 * Restore the terminal mode to that saved by set_raw_term_mode.
 */
void restore_term_mode(struct termios saved_term_mode) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_term_mode);
}

/**
 * Set the terminal to raw mode, set up a one-second timer, set up the SIGARLM
 * handler, and then wait for the user to type 'q'.
 *
 * For details of timer setup, see Stevens Section 6.1 and the man pages for
 * setitimer(2) and gettimeofday(3C).
 *
 * For details of signal setup, see Stevens Section 10.14, and the man page for
 * sigaction.
 */
int main(int argc, char** argv) {
	int i;
	///do error handling first
	if (argc <= 1 || argc > 2)
	{
		fprintf(stderr, "usage: timer <seconds>\n");
		exit(-1);
	}
	if (argv[1][0] != '0' && atoi(argv[1]) == 0)
	{
		fprintf(stderr, "\"%s\" is not a number.\n", argv[1]);
		exit(-1);
	}
	for (i = 0; i < strlen(argv[1]); i++)
	{
		if (argv[1][i] == '.' || isalpha(argv[1][i]))
		{
			fprintf(stderr, "%s: malformed time.\nusage: timer <seconds>\n", argv[1]);
			exit(-1);
		}
	}
	if (atoi(argv[1]) >= 3600000)
	{
		fprintf(stderr, "%d: out of range\n", atoi(argv[1]));
		exit(-1);
	}
	if(atoi(argv[1]) < 0)
	{
		fprintf(stderr, "Invalid time (%d). Must be >= 0.\n", atoi(argv[1]));
		exit(-1);
	}
	
	
	char c;

    struct termios saved_term_mode;      /* saved entering terminal mode */
    struct itimerval tbuf;               /* interval timer structure */
    struct sigaction action;             /* signal action structure */

    /*
     * Initialize time
     */
    ticks = atoi(argv[1]);
    hour = ticks / 3600;
    ticks = ticks % 3600;
    minute = ticks / 60;
    ticks = ticks % 60;

    /*
     * Set up the SIGALRM handler.
     */
    action.sa_handler = tick;     /* set tick to be the handler function */
    sigemptyset(&action.sa_mask); /* clear out masked functions */
    action.sa_flags   = 0;        /* no special handling */

    /*
     * Use the sigaction function to associate the signal action with SIGALRM.
     */
    if (sigaction(SIGALRM, &action, NULL) < 0 ) {
        perror("SIGALRM");
        exit(-1);
    }

    /*
     * Define a one-second timer.
     */
    tbuf.it_interval.tv_sec  = 0;
    tbuf.it_interval.tv_usec = 1;
    tbuf.it_value.tv_sec  = 0;
    tbuf.it_value.tv_usec = 1;

    /*
     * Use the setitimer function to start the timer.
     */
    if ( setitimer(ITIMER_REAL, &tbuf, NULL) == -1 ) {
        perror("setitimer");
        exit(-1);                   /* should only fail for serious reasons */
    }

    /*
     * Set the terminal to raw mode.
     */
    saved_term_mode = set_raw_term_mode();
    printf("%c %c%d:%.2d%:%.2d     \r", twirler[ind++], neg, abs(hour), abs(minute), abs(ticks));
    fflush(stdout);
    /*
     * Busy wait until the user types 'q'.
     */
    while ((c = getchar()) != 'q') 
    {
    	///handle all of the possible options that can be input while the clock is run
    	switch(c)
    	{
    		case 'c':
    		///clear the clock
    			ticks = 0;
    			minute = 0;
    			hour = 0;
    			break;
    		case 'r':
    		///pause/unpause the clock
    			if (paused == 0)
    				paused = 1;
    			else if (paused == 1)
    				paused = 0;
    			break;
    		case 'h':
    		///handle hour adding
    				if (minute < 0 && hour == 0)
    				{
    					minute += 60;
    					ticks *= -1;
    				}
    				else if (isNeg == 1 && hour == 0 && minute == 0 && ticks < 0)
    				{
    					minute += 59;
    					ticks = 60 - abs(ticks);
    					isNeg = 0;
    					neg = ' ';
    				}
    				else if (isNeg == 1 && hour > 0)
    				{
    					hour++;
    					isNeg = 0;
    					neg = ' ';
    					ticks *= -1;
    					minute *= -1;
    				}
    				else
    				{
    					hour++;
    				}
    			break;
    		case 'H':
    		///handle hour subtracting
    				if (minute > 0 && hour == 0)
	    			{
	    				minute -= 60;
	    				ticks *= -1;
	    			}
    				else if (isNeg == 0 && minute == 0 && hour == 0 && ticks > 0)
    				{
    					minute -= 59;
    					ticks = -1*(60 - ticks);
    					isNeg = 1;
    					neg = '-';
    				}
    				else if (isNeg == 0 && hour < 0)
    				{
    					hour--;
    					isNeg = 1;
    					neg = '-';
    					ticks *= -1;
    					minute *= -1;
    				}
    				else
    				{
    					hour--;	
    				}
    			break;
    		case 'm':
    		///handle minute adding
    			if (ticks < 0 && minute == 0 && hour == 0)
    			{
    				ticks += 60;
    			}
    			else
    			{
    				minute++;
    				if (isNeg == 0 && minute >= 60)
    				{
    					minute = 0;
    					hour++;
    				}
    				else if (isNeg == 1 && hour < 0 && minute > 0)
    				{
    					minute = -59;
    					hour++;
    				}
    			}
    			break;
    		case 'M':
    		///handle minute subtracting
    			if (ticks > 0 && minute == 0 && hour == 0)
    			{
    				ticks -= 60;
    			}
    			else
    			{
    				minute--;
    				if (isNeg == 0 && hour > 0 && minute <= 0)
    				{
    					minute = 59;
    					hour--;
    				}
    				else if (isNeg == 1 && minute <= -60)
    				{
    					minute = 0;
    					hour--;
    				}
    			}
    			break;
    		case 's':
    		///handle second adding
    			ticks++;
    			if (ticks >=60 && isNeg == 0)
    			{
    				ticks = 0;
    				minute++;
    				if (minute == 60)
    				{
    					hour++;
    					minute = 0;
    				}
    			}
    			else if (ticks >= 0 && isNeg == 1)
    			{
    				if (minute < 0)
    				{
	    				ticks = -59;
	    				minute++;
	    				if (hour <= 0 && minute > 0)
	    				{
	    					hour++;
	    					minute = -59;
	    				}
    				}
    				else if (hour < 0)
    				{
    					ticks = -59;
    					hour++;
    					minute = -59;
    				}
    			}
    			break;
    		case 'S':
    		///handle second subtracting
    			ticks--;
    			if (ticks <= -60  && isNeg == 1)
    			{
    				ticks = 0;
    				minute--;
    				if (minute == -60)
    				{
    					hour--;
    					minute = 0;
    				}	
    			}
    			else if (ticks <= 0 && isNeg == 0)
    			{
    				if (minute > 0)
    				{
    					ticks = 59;
    					minute--;
    					if (hour > 0 && minute < 0)
    					{
    						hour--;
    						minute = 59;
    					}
    				}
    				else if (hour > 0)
    				{
    					ticks = 59;
    					hour--;
    					minute = 59;
    				}
    			}
    			
    			break;
    	}
    }
    --ind;
    printf("%c %c%d:%.2d%:%.2d     \n", twirler[ind], neg, abs(hour), abs(minute), abs(ticks));
    fflush(stdout);
    /*
     * Restore the terminal to the mode it was in at program entry.
     */
    restore_term_mode(saved_term_mode);

}

