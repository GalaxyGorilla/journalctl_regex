#include <stdio.h>
#include <systemd/sd-journal.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <regex.h> 

struct sigaction sigact;
int active = 1;

void sig_handler(int signum){
	active = 0;
	printf("\nTerminated by user.\n");
}

int main (int argc, char *argv[]){

	signal(SIGINT, sig_handler);

	/* Compile regular expression */
	regex_t regex;
	int reti = regcomp(&regex, argv[1], 0);
	if (reti) {
    		fprintf(stderr, "Could not compile regex\n");
    		return 1;
	}

	sd_journal *j = NULL;

	int r = -1;
	if(argc == 2) 		r = sd_journal_open(&j, SD_JOURNAL_LOCAL_ONLY);
	else if(argc == 3) 	r = sd_journal_open_directory(&j, argv[2], 0);
	if(argc>3 || r<0){ 			
		printf("Error: unable to open journal.\n"); 
		return 1;
	}

	sd_journal_seek_head(j);

	const void *data;
  	size_t length;
	int counter = 0;
	SD_JOURNAL_FOREACH(j){
		if(!active){
			sd_journal_close(j);
			return 0;
		}
  		SD_JOURNAL_FOREACH_DATA(j, data, length){
			/* Execute regular expression */
			reti = regexec(&regex, (char *) data, 0, NULL, 0);
			if (!reti){
				printf("---------------------------------------------------\n");
				printf("MATCH: %s\n", (char *) data);
				printf("---------------------------------------------------\n");
				sd_journal_restart_data(j);
  				SD_JOURNAL_FOREACH_DATA(j, data, length){
    					printf("%.*s\n", (int) length, (char *) data);
				}
				printf("\n");
				break;
			}
		}
		counter++;
	}

	sd_journal_close(j);

	printf("\nNumber of logs: %d\n", counter);
	return 0;
}
