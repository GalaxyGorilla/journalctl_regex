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

	// measure elapsed wall time
    	struct timespec start, stop;
    	clock_gettime(CLOCK_REALTIME, &start);

	signal(SIGINT, sig_handler);

	// Compile regular expression
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
	int log_counter = 0, match_counter=0;
	SD_JOURNAL_FOREACH(j){
		if(!active){
			sd_journal_close(j);
			return 0;
		}
  		SD_JOURNAL_FOREACH_DATA(j, data, length){
			// Execute regular expression 
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
				match_counter++;
				break;
			}
		}
		log_counter++;
	}

	sd_journal_close(j);

    	clock_gettime(CLOCK_REALTIME, &stop);
    	double seconds = (double)((stop.tv_sec+stop.tv_nsec*1e-9) - (double)(start.tv_sec+start.tv_nsec*1e-9));

	printf("\nNumber of logs: %d\n", log_counter);
	printf("Number of matches: %d\n", match_counter);
    	printf("Wall time: %fs\n", seconds);

	return 0;
}
