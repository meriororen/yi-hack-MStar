/*
 * This file is part of libipc (https://github.com/TheCrypt0/libipc).
 * Copyright (c) 2019 roleo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Send message to a IPC queue.
 */

#include "ipc_cmd.h"
#include "getopt.h"
#include "signal.h"

mqd_t ipc_mq;

int open_queue()
{
    ipc_mq = mq_open(IPC_QUEUE_NAME, O_RDWR);
    if(ipc_mq == -1) {
        fprintf(stderr, "Can't open mqueue %s\n", IPC_QUEUE_NAME);
        return -1;
    }
    return 0;
}

int ipc_start()
{
    int ret = -1;

    ret = open_queue();
    if(ret != 0) {
        return -1;
    }

    return 0;
}

void ipc_stop()
{
    if(ipc_mq > 0) {
        mq_close(ipc_mq);
    }
}

void print_usage(char *progname)
{
    fprintf(stderr, "\nUsage: %s [t ON/OFF] [-s SENS] [-l LED] [-v WHEN] [-i IR] [-r ROTATE] [-m MOVE] [-p NUM] [-f FILE] [-S] [-T] [-d]\n\n", progname);
    fprintf(stderr, "\t-t ON/OFF, --switch ON/OFF\n");
    fprintf(stderr, "\t\tswitch ON or OFF the cam\n");
    fprintf(stderr, "\t-s SENS, --sensitivity SENS\n");
    fprintf(stderr, "\t\tset sensitivity: LOW, MEDIUM or HIGH\n");
    fprintf(stderr, "\t-l LED, --led LED\n");
    fprintf(stderr, "\t\tset led: ON or OFF\n");
    fprintf(stderr, "\t-v WHEN, --save WHEN\n");
    fprintf(stderr, "\t\tset save mode: ALWAYS or DETECT\n");
    fprintf(stderr, "\t-i IR, --ir IR\n");
    fprintf(stderr, "\t\tset ir led: ON or OFF\n");
    fprintf(stderr, "\t-r ROTATE, --rotate ROTATE\n");
    fprintf(stderr, "\t\tset rotate: ON or OFF\n");
    fprintf(stderr, "\t-b BABYCRYING, --babycrying BABYCRYING\n");
    fprintf(stderr, "\t\tset baby crying detection: ON or OFF\n");
    fprintf(stderr, "\t-m MOVE, --move MOVE\n");
    fprintf(stderr, "\t\tsend PTZ command: RIGHT, LEFT, DOWN, UP or STOP\n");
    fprintf(stderr, "\t-p NUM, --preset NUM\n");
    fprintf(stderr, "\t\tsend PTZ go to preset command: NUM = [0..7]\n");
    fprintf(stderr, "\t-f FILE, --file FILE\n");
    fprintf(stderr, "\t\tread binary command from FILE\n");
    fprintf(stderr, "\t-x, --xxx\n");
    fprintf(stderr, "\t\tsend xxx message\n");
    fprintf(stderr, "\t-S TIME, --start_motion TIME\n");
    fprintf(stderr, "\t\tstart a motion detection event that lasts TIME\n");
    fprintf(stderr, "\t\t(0<TIME<=300 seconds)\n");
    fprintf(stderr, "\t-T, --stop_motion\n");
    fprintf(stderr, "\t\tstop a motion detection event\n");
    fprintf(stderr, "\t-d, --debug\n");
    fprintf(stderr, "\t\tenable debug\n");
    fprintf(stderr, "\t-j [0-90],[0-90]");
    fprintf(stderr, "\t\tjump to point X and Y on screen");
    fprintf(stderr, "\t-h, --help\n");
    fprintf(stderr, "\t\tprint this help\n");
}

int main(int argc, char ** argv)
{
    int errno;
    char *endptr;
    int c, ret;
    int switch_on = NONE;
    int sensitivity = NONE;
    int jumpto[2];
    int led = NONE;
    int save = NONE;
    int ir = NONE;
    int rotate = NONE;
    int babycrying = NONE;
    int move = NONE;
    int preset = NONE;
    int debug = 0;
    unsigned char preset_msg[20];
    int start = 0;
    int stop = 0;
    int time_to_stop = 60;
    char file[1024];
    unsigned char msg_file[1024];
    FILE *fIn;
    int nread = 0;
    int xxx = 0;

    file[0] = '\0';
    jumpto[0] = -1;
    jumpto[1] = -1;

    while (1) {
        static struct option long_options[] =
        {
            {"switch",  required_argument, 0, 't'},
            {"sensitivity",  required_argument, 0, 's'},
            {"led",  required_argument, 0, 'l'},
            {"save",  required_argument, 0, 'v'},
            {"ir",  required_argument, 0, 'i'},
            {"rotate",  required_argument, 0, 'r'},
            {"babycrying",  required_argument, 0, 'b'},
            {"move",  required_argument, 0, 'm'},
            {"preset",  required_argument, 0, 'p'},
            {"file", required_argument, 0, 'f'},
            {"start", required_argument, 0, 'S'},
            {"jump", required_argument, 0, 'j'},
            {"stop", no_argument, 0, 'T'},
            {"xxx", no_argument, 0, 'x'},
            {"debug",  no_argument, 0, 'd'},
            {"help",  no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "t:s:l:v:i:r:b:m:p:f:S:j:Txdh",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 't':
            if (strcasecmp("on", optarg) == 0) {
                switch_on = SWITCH_ON;
            } else if (strcasecmp("off", optarg) == 0) {
                switch_on = SWITCH_OFF;
            }
            break;

        case 's':
            if (strcasecmp("low", optarg) == 0) {
                sensitivity = SENSITIVITY_LOW;
            } else if (strcasecmp("medium", optarg) == 0) {
                sensitivity = SENSITIVITY_MEDIUM;
            } else if (strcasecmp("high", optarg) == 0) {
                sensitivity = SENSITIVITY_HIGH;
            }
            break;

        case 'l':
            if (strcasecmp("off", optarg) == 0) {
                led = LED_OFF;
            } else if (strcasecmp("on", optarg) == 0) {
                led = LED_ON;
            }
            break;

        case 'v':
            if (strcasecmp("always", optarg) == 0) {
                save = SAVE_ALWAYS;
            } else if (strcasecmp("detect", optarg) == 0) {
                save = SAVE_DETECT;
            }
            break;

        case 'i':
            if (strcasecmp("off", optarg) == 0) {
                ir = IR_OFF;
            } else if (strcasecmp("on", optarg) == 0) {
                ir = IR_ON;
            }
            break;

        case 'r':
            if (strcasecmp("off", optarg) == 0) {
                rotate = ROTATE_OFF;
            } else if (strcasecmp("on", optarg) == 0) {
                rotate = ROTATE_ON;
            }
            break;

        case 'b':
            if (strcasecmp("off", optarg) == 0) {
                babycrying = BABYCRYING_OFF;
            } else if (strcasecmp("on", optarg) == 0) {
                babycrying = BABYCRYING_ON;
            }
            break;

        case 'm':
            if (strcasecmp("right", optarg) == 0) {
                move = MOVE_RIGHT;
            } else if (strcasecmp("left", optarg) == 0) {
                move = MOVE_LEFT;
            } else if (strcasecmp("down", optarg) == 0) {
                move = MOVE_DOWN;
            } else if (strcasecmp("up", optarg) == 0) {
                move = MOVE_UP;
            } else if (strcasecmp("stop", optarg) == 0) {
                move = MOVE_STOP;
            }
            break;

        case 'p':
            errno = 0;    /* To distinguish success/failure after call */
            preset = strtol(optarg, &endptr, 10);

            /* Check for various possible errors */
            if ((errno == ERANGE && (preset == LONG_MAX || preset == LONG_MIN)) || (errno != 0 && preset == 0)) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if (endptr == optarg) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;

        case 'j':
	    printf("j command\n");
            /* Check for various possible errors */
            if (strlen(optarg) < 1023) {
		printf("j argument: %s\n", optarg);
	        if (strchr(optarg, ',') == NULL) {
		    print_usage(argv[0]);
		    exit(EXIT_FAILURE);
		} else {
		    printf("strtoking\n");
	            char *jX = strtok(optarg, ",");
		    printf("token1: %s\n", jX);
		    if (jX == NULL) {
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		    }
		    char *jY = strtok(NULL, ",");
		    printf("token2: %s\n", jX);
		    if (jY == NULL) {
		   	print_usage(argv[0]);
		   	exit(EXIT_FAILURE);
		    }
		    printf("strtoling %s, %s\n", jX, jY);
		    jumpto[0] = strtol(jX, NULL, 10);
		    jumpto[1] = strtol(jY, NULL, 10);
		    printf("validating\n");
		    if (jumpto[0] > 90 || jumpto[0] < 0 || jumpto[1] < 0 || jumpto[1] > 90) {
			printf("X and Y must be between [0, 90]\n");
		   	exit(EXIT_FAILURE);
		    }
		}
            } else {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;

        case 'f':
            /* Check for various possible errors */
            if (strlen(optarg) < 1023) {
                strcpy(file, optarg);
            } else {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;

        case 'S':
            errno = 0;    /* To distinguish success/failure after call */
            time_to_stop = strtol(optarg, &endptr, 10);

            /* Check for various possible errors */
            if ((errno == ERANGE && (time_to_stop == LONG_MAX || time_to_stop == LONG_MIN)) || (errno != 0 && time_to_stop == 0)) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if ((time_to_stop <= 0) || (time_to_stop > 300)) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if (endptr == optarg) {
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            start = 1;
            break;

        case 'T':
            stop = 1;
            break;

        case 'd':
            fprintf (stderr, "debug on\n");
            debug = 1;
            break;

        case 'x':
            xxx = 1;
            break;

        case 'h':
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;

        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    if (argc == 1) {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    ret=ipc_start();
    if(ret != 0) {
        exit(EXIT_FAILURE);
    }

    if (switch_on == SWITCH_ON) {
        mq_send(ipc_mq, IPC_SWITCH_ON, sizeof(IPC_SWITCH_ON) - 1, 0);
    } else if (switch_on == SWITCH_OFF) {
        mq_send(ipc_mq, IPC_SWITCH_OFF, sizeof(IPC_SWITCH_OFF) - 1, 0);
    }

    if (sensitivity == SENSITIVITY_LOW) {
        mq_send(ipc_mq, IPC_SENS_LOW, sizeof(IPC_SENS_LOW) - 1, 0);
    } else if (sensitivity == SENSITIVITY_MEDIUM) {
        mq_send(ipc_mq, IPC_SENS_MEDIUM, sizeof(IPC_SENS_MEDIUM) - 1, 0);
    } else if (sensitivity == SENSITIVITY_HIGH) {
        mq_send(ipc_mq, IPC_SENS_HIGH, sizeof(IPC_SENS_HIGH) - 1, 0);
    }

    if (led == LED_OFF) {
        mq_send(ipc_mq, IPC_LED_OFF, sizeof(IPC_LED_OFF) - 1, 0);
    } else if (led == LED_ON) {
        mq_send(ipc_mq, IPC_LED_ON, sizeof(IPC_LED_ON) - 1, 0);
    }

    if (save == SAVE_ALWAYS) {
        mq_send(ipc_mq, IPC_SAVE_ALWAYS, sizeof(IPC_SAVE_ALWAYS) - 1, 0);
    } else if (save == SAVE_DETECT) {
        mq_send(ipc_mq, IPC_SAVE_DETECT, sizeof(IPC_SAVE_DETECT) - 1, 0);
    }

    if (ir == IR_OFF) {
        mq_send(ipc_mq, IPC_IR_OFF, sizeof(IPC_IR_OFF) - 1, 0);
    } else if (ir == IR_ON) {
        mq_send(ipc_mq, IPC_IR_ON, sizeof(IPC_IR_ON) - 1, 0);
    }

    if (rotate == ROTATE_OFF) {
        mq_send(ipc_mq, IPC_ROTATE_OFF, sizeof(IPC_ROTATE_OFF) - 1, 0);
    } else if (rotate == ROTATE_ON) {
        mq_send(ipc_mq, IPC_ROTATE_ON, sizeof(IPC_ROTATE_ON) - 1, 0);
    }

    if (babycrying == BABYCRYING_OFF) {
        mq_send(ipc_mq, IPC_BABYCRYING_OFF, sizeof(IPC_BABYCRYING_OFF) - 1, 0);
    } else if (babycrying == BABYCRYING_ON) {
        mq_send(ipc_mq, IPC_BABYCRYING_ON, sizeof(IPC_BABYCRYING_ON) - 1, 0);
    }

    if (move == MOVE_RIGHT) {
        mq_send(ipc_mq, IPC_MOVE_RIGHT, sizeof(IPC_MOVE_RIGHT) - 1, 0);
    } else if (move == MOVE_LEFT) {
        mq_send(ipc_mq, IPC_MOVE_LEFT, sizeof(IPC_MOVE_LEFT) - 1, 0);
    } else if (move == MOVE_DOWN) {
        mq_send(ipc_mq, IPC_MOVE_DOWN, sizeof(IPC_MOVE_DOWN) - 1, 0);
    } else if (move == MOVE_UP) {
        mq_send(ipc_mq, IPC_MOVE_UP, sizeof(IPC_MOVE_UP) - 1, 0);
    } else if (move == MOVE_STOP) {
        mq_send(ipc_mq, IPC_MOVE_STOP, sizeof(IPC_MOVE_STOP) - 1, 0);
    }

    if (jumpto[0] > 0 || jumpto[1] > 0) {
	printf("Jump to sending..\n");
	char jumppoint[32] = IPC_JUMP_POINT;
	jumppoint[16] = jumpto[0];
	jumppoint[20] = jumpto[1];
	mq_send(ipc_mq, (const char *)jumppoint, sizeof(IPC_JUMP_POINT) - 1, 0);
    }

    if (preset != NONE) {
        memcpy(preset_msg, IPC_GOTO_PRESET, sizeof(IPC_GOTO_PRESET) - 1);
        preset_msg[16] = preset & 0xff;
        mq_send(ipc_mq, preset_msg, sizeof(IPC_GOTO_PRESET) - 1, 0);
    }

    if (file[0] != '\0') {
        fIn = fopen(file, "r");
        if (fIn == NULL) {
            fprintf(stderr, "Error opening file %s\n", file);
            exit(EXIT_FAILURE);
        }

        // Tell size
        fseek(fIn, 0L, SEEK_END);
        nread = ftell(fIn);
        fseek(fIn, 0L, SEEK_SET);

        if (fread(msg_file, 1, nread, fIn) != nread) {
            fprintf(stderr, "Error reading file %s\n", file);
            exit(EXIT_FAILURE);
        }
        fclose(fIn);
        mq_send(ipc_mq, msg_file, nread, 0);
    }

    if (start == 1) {
        mq_send(ipc_mq, IPC_MOTION_START, sizeof(IPC_MOTION_START) - 1, 0);

        pid_t pid;

        /* Fork off the parent process */
        pid = fork();
        /* An error occurred */
        if (pid < 0)
            exit(EXIT_FAILURE);
         /* Success: Let the parent terminate */
        if (pid > 0)
            exit(EXIT_SUCCESS);
        /* On success: The child process becomes session leader */
        if (setsid() < 0)
            exit(EXIT_FAILURE);
        /* Catch, ignore and handle signals */
        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        /* Fork off for the second time*/
        pid = fork();
        /* An error occurred */
        if (pid < 0)
            exit(EXIT_FAILURE);
        /* Success: Let the parent terminate */
        if (pid > 0)
            exit(EXIT_SUCCESS);
        /* Set new file permissions */
        umask(0);
        /* Change the working directory to the root directory */
        /* or another appropriated directory */
        chdir("/");
        /* Close all open file descriptors */
        int x;
        for (x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
            close (x);
        }

        sleep(time_to_stop);
        mq_send(ipc_mq, IPC_MOTION_STOP, sizeof(IPC_MOTION_STOP) - 1, 0);
    }

    if (stop == 1) {
        mq_send(ipc_mq, IPC_MOTION_STOP, sizeof(IPC_MOTION_STOP) - 1, 0);
    }

    if (xxx == 1) {
        mq_send(ipc_mq, IPC_XXX_0, sizeof(IPC_XXX_0) - 1, 0);
    }

    ipc_stop();

    return 0;
}
