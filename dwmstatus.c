#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <err.h>
#include <errno.h>
#include <X11/Xlib.h>

/* Network interfaces */


/* Battery defines */
/* For VAIO */
#define BATT_NOW        "/sys/class/power_supply/BAT1/capacity"
#define BATT_STATUS     "/sys/class/power_supply/BAT1/status"

#define BAT_CHARGING	"Charging"
#define BAT_DISCHARGING	"Discharging"
#define BAT_FULL	"Full"

/* Loadavg defines */
#define LOADAVG_ELEM	1

/* Network defines */

/* Temperature defines */
#define TEMP_NOW        "/sys/class/hwmon/hwmon0/temp1_input"

#define MAX_STATUS_LEN	64

#ifndef DEBUG
static Display *dpy;
#endif

static void closedisplay (int sig)
{
#ifndef DEBUG
    XCloseDisplay(dpy);
#endif
    exit(0);
}

static void opendisplay (void)
{
#ifndef DEBUG
    if (!(dpy = XOpenDisplay(NULL))) {
	err(1, "Unable to open display");
    }
#endif
    signal(SIGINT, closedisplay);
    signal(SIGTERM, closedisplay);
}

static void set_status (char *str)
{
#ifndef DEBUG
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
#else
    puts(str);
#endif
}

/*
 * Get battery status
 */
static int get_battery (char *buf, size_t buflen) {
    int charge_now;
    char status[12];
    char s = '?';
    FILE *fp = NULL;

    if ((fp = fopen(BATT_NOW, "r"))) {
	fscanf(fp, "%d\n", &charge_now);
	fclose(fp);

	fp = fopen(BATT_STATUS, "r");
	fscanf(fp, "%s\n", status);
	fclose(fp);

	if (strncmp(status, BAT_CHARGING, strlen(BAT_CHARGING)) == 0) {
	    s = '+';
	} else if (strncmp(status, BAT_DISCHARGING, strlen(BAT_DISCHARGING)) == 0) {
	    s = '-';
	} else if (strncmp(status, BAT_FULL, strlen(BAT_FULL)) == 0) {
	    s = '=';
	}

	return snprintf(buf, buflen, "%c%d%%", s, charge_now);
    } else {
	buf[0] = '\0';
	return 0;
    }
}

/*
 * Get date
 */
static int get_timendate (char *buf, size_t buflen) {
    time_t now = time(NULL);
    struct tm *tm;

    tm = localtime(&now);
    
    return strftime(buf, buflen, "%a %b %d %H:%M %Y", tm);
}

/*
 * Get loadavg
 */
static int get_loadavg (char *buf, size_t buflen) {
    double loadavg[LOADAVG_ELEM];

    if (getloadavg(loadavg, LOADAVG_ELEM) > 0) {
	snprintf(buf, buflen, "%6.2f", loadavg[0]);
    }

    return 0;
}

/*
 * Get temperature
 */
static int get_temp (char *buf, size_t buflen) {
    int temp;
    FILE *fp = NULL;

    if ((fp = fopen(TEMP_NOW, "r"))) {
	fscanf(fp, "%d\n", &temp);
	fclose(fp);

	return snprintf(buf, buflen, "%d deg C", temp / 1000);
    }

    return 0;
}

/*
 * Get network info
 */
static int get_network (char *buf, size_t buflen) {
    return 0;
}

int main (void)
{
    char status[MAX_STATUS_LEN] = { 0, };
    char *buf;
    size_t offset, buflen;

#ifndef DEBUG
    daemon(0, 0);
#endif

    opendisplay();

    offset = 0;
    do {
	buflen = sizeof(status);
	buf = status;

	/* get network status */
	offset += get_network(buf + offset, buflen - offset);
	buf[offset++] = ' ';
	buf[offset++] = '|';
	buf[offset++] = ' ';

	/* get time n date */
	offset += get_timendate(buf + offset, buflen - offset);
	buf[offset++] = ' ';
	buf[offset++] = '|';
	buf[offset++] = ' ';

	/* get battery info */
	offset += get_battery(buf + offset, buflen - offset);
	buf[offset++] = ' ';
	buf[offset++] = '|';
	buf[offset++] = ' ';

	/* get temp info */
	offset += get_temp(buf + offset, buflen - offset);
	buf[offset++] = ' ';
	buf[offset++] = '|';
	buf[offset++] = ' ';

	/* get loadavg */
	offset += get_loadavg(buf + offset, buflen - offset);

	set_status(status);
	offset = 0;
    } while (!sleep(1));

    closedisplay(0);

    return 0;
}
