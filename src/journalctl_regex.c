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

	sd_journal *j = NULL;
	sd_journal *j_match = NULL;

	int match_counter=0, log_counter=0;

	const void *data;
  	size_t length;

	const void *data_match;
  	size_t length_match;

	if(argc == 4){
	 	int r = sd_journal_open_directory(&j, argv[3], 0);
	 	int r_match = sd_journal_open_directory(&j_match, argv[3], 0);
		if(r<0 || r_match<0){
			printf("Error: unable to open journal.\n"); 
			return 1;	
		}

		// Compile regular expression
		regex_t regex;
		int reti = regcomp(&regex, argv[2], 0);
		if (reti) {
    			fprintf(stderr, "Could not compile regex\n");
    			return 1;
		}

		sd_journal_query_unique(j, argv[1]);

		int match_flag=0;
		SD_JOURNAL_FOREACH_UNIQUE(j, data, length){
			if(!active) break;
			reti = regexec(&regex, (char *) data, 0, NULL, 0);
			if (!reti){
				sd_journal_add_match(j_match, data, 0);
				match_flag = 1;
			}
			log_counter++;
		}

		if(match_flag==0) return 0;

		SD_JOURNAL_FOREACH(j_match){
			if(!active) break;
  			SD_JOURNAL_FOREACH_DATA(j_match, data_match, length_match){
    				printf("%.*s\n", (int) length_match, (char *) data_match);
			}
			match_counter++;
			printf("\n");
		}
		sd_journal_close(j);
		sd_journal_close(j_match);
	}

	// argc=3
	else{
	 	int r = sd_journal_open_directory(&j, argv[2], 0);
		if(r<0){
			printf("Error: unable to open journal.\n"); 
			return 1;	
		}

		// Compile regular expression
		regex_t regex;
		int reti = regcomp(&regex, argv[1], 0);
		if (reti) {
    			fprintf(stderr, "Could not compile regex\n");
    			return 1;
		}

		SD_JOURNAL_FOREACH(j){
			if(!active) break;
  			SD_JOURNAL_FOREACH_DATA(j, data, length){
				reti = regexec(&regex, (char *) data, 0, NULL, 0);
				if (!reti){
  					SD_JOURNAL_FOREACH_DATA(j, data_match, length_match){
    						printf("%.*s\n", (int) length_match, (char *) data_match);
					}
					printf("\n");
					match_counter++;
					break;
				}
			}
			log_counter++;
		}
		sd_journal_close(j);
	}

    	clock_gettime(CLOCK_REALTIME, &stop);
    	double seconds = (double)((stop.tv_sec+stop.tv_nsec*1e-9) - (double)(start.tv_sec+start.tv_nsec*1e-9));

	printf("Number of matches: %d\n", match_counter);
	printf("Number of logs: %d\n", log_counter);
    	printf("Wall time: %fs\n", seconds);

	return 0;
}
