#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct _enemy *Enemy;

typedef struct _enemy {
	int match_id;
	char name[10];
	char port[10];
} enemy;

#define ROCK 0
#define PAPER 1
#define SCISSORS 2

void print_quads(in_addr_t x) {
    char* bytes = (char*)&x;  
    for (int i=0; i<sizeof(x); ++i) {
        printf("%u.", (unsigned)bytes[i]);
    }
}

int main(int argc, char** argv) {
    if (argc<4) {
        fprintf(stderr, "Usage: rpsclient name matches port\n");
        return 1;
    }
	for (int n = 0; n < strlen(argv[1]); n++) {
		if (!isalpha(argv[1][n]) && !isdigit(argv[1][n])) {
		    fprintf(stderr, "Invalid name\n");
			return 2;
		}
	}
	int STDFD_2 = dup(1);
	FILE *STDOUT_2 = fdopen(STDFD_2, "w");
	int check_it = -1;
	int num_matches = -1;
	check_it = sscanf(argv[2], "%d", &num_matches);
	if (check_it != 1 || num_matches < 1) {
        fprintf(stderr, "Invalid match count\n");
		return 3;
	}
	for (int n = 0; n < strlen(argv[3]); n++) {
		if (!isdigit(argv[3][n])) {
        	fprintf(stderr, "Invalid port number\n");
			return 4;
		}
	}
	int port_i = -1;
	check_it = sscanf(argv[3], "%d\n", &port_i);
	if (check_it != 1 || port_i >= 100000) {
        fprintf(stderr, "Invalid port number\n");
		return 4;
	}
	int seed = 0;
	for(int i = 0; i < strlen(argv[1]); i++) {
		seed += argv[1][i];
	}
	srand(seed);
    const char* server_port=argv[3];
//server-start
	struct addrinfo* ai = 0;
	struct addrinfo hints;

	char *port = "0";

	memset(& hints, 0, sizeof(struct addrinfo));
	hints.ai_family=AF_INET;        // IPv4  for generic could use AF_UNSPEC
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;  // Because we want to bind with it    
	int err;
	if (err=getaddrinfo("127.0.0.1", port, &hints, &ai)) {
	    freeaddrinfo(ai);
	    fprintf(stderr, "%s\n", gai_strerror(err));
	    return 4;   // could not work out the address
	}
	
	    // create a socket and bind it to a port
	int serv = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
	if (bind(serv, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
	    perror("Binding");
	    return 4;
	}
	
	    // Which port did we get?
	struct sockaddr_in ad;
	memset(&ad, 0, sizeof(struct sockaddr_in));
	socklen_t len=sizeof(struct sockaddr_in);
	if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
	    perror("sockname");
	    return 4;
	}
	if (listen(serv, 10)) {     // allow up to 10 connection requests to queue
		perror("Listen");
		return 4;
	}
//server-end
	char ending_message[1024] = {'\0'};
	while (num_matches > 0) {
		struct addrinfo* server_ai = 0;
		struct addrinfo server_hints;
		memset(& server_hints, 0, sizeof(struct addrinfo));
		server_hints.ai_family=AF_INET;        // IPv4  for generic could use AF_UNSPEC
		server_hints.ai_socktype=SOCK_STREAM;
		int server_err;
		if (server_err=getaddrinfo("127.0.0.1", server_port, &server_hints, &server_ai)) {
		    freeaddrinfo(server_ai);
       		fprintf(stderr, "Invalid match count");
		    return 4;   // could not work out the address
		}

		int server_fd=socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
		if (connect(server_fd, (struct sockaddr*)server_ai->ai_addr, sizeof(struct sockaddr))) {
		    perror("Connecting");
		    return 4;
		}
		// fd is now connected
		// we want separate streams (which we can close independently)
		
		int server_fd2=dup(server_fd);
		FILE* server_to=fdopen(server_fd, "w");
		FILE* server_from=fdopen(server_fd2, "r");
		

	    //printf("%u\n", ntohs(ad.sin_port));
		//printf("MR:%s:%u\n", argv[1], ntohs(ad.sin_port)); //send a match request and wait
		fprintf(server_to, "MR:%s:%u\n", argv[1], ntohs(ad.sin_port)); //send a match request and wait
		fflush(server_to);
		
		char buffer[80];
		fgets(buffer, 79, server_from);
		//fprintf(stdout, "%s", buffer);
		if (buffer == EOF || strcmp(buffer, "") == 0 || buffer[0] == '\0') {
        	fprintf(stderr, "Invalid port number\n");
			return 4;
		}
		if (strcmp("BADNAME\n", buffer) == 0) {
			return 2;
		}

		Enemy enemy = malloc(sizeof(struct _enemy));
		char tok[80] = {'\0'};
		strcpy(tok, buffer);
		int tok_n = 0;
		int j = 0;
		for (int i = 0; i <= strlen(buffer)+1; i++) {
			if (buffer[i] == ':' || buffer[i] == '\n' || buffer[i] == '\0') {
				tok[j] = '\0';
				if (tok_n == 0 && strcmp("MATCH", tok) != 0) {
					//error
					fprintf(stderr, "Invalid port number\n");
					return 4;
				} else if (tok_n == 1) {
					enemy->match_id = atoi(tok);
				} else if (tok_n == 2) {
					strcpy(enemy->name, tok);
				} else if (tok_n == 3) {
					strcpy(enemy->port, tok);
				}
				tok_n += 1;
				strcpy(tok, tok + j + 1);
				j = -1;
			}
			j += 1;
		}

		struct addrinfo* enemy_ai = 0;
		struct addrinfo enemy_hints;
		memset(& enemy_hints, 0, sizeof(struct addrinfo));
		enemy_hints.ai_family=AF_INET;        // IPv4  for generic could use AF_UNSPEC
		enemy_hints.ai_socktype=SOCK_STREAM;
		int enemy_err;
		if (enemy_err=getaddrinfo("127.0.0.1", enemy->port, &enemy_hints, &enemy_ai)) {
		    freeaddrinfo(enemy_ai);
		    fprintf(stderr, "%s\n", gai_strerror(enemy_err));
		    return 1;   // could not work out the address
		}

		int conn_fd;
		int enemy_fd=socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
		char msg[80];
		//printf("Lets play a game match_id: %d, enemy: %s, enemy_port %s!\n", enemy->match_id, enemy->name, enemy->port);
		
		//printf("connecting\n");
		int con_er = connect(enemy_fd, (struct sockaddr*)enemy_ai->ai_addr, sizeof(struct sockaddr));
		//printf("accepting\n");		
		conn_fd = accept(serv, 0, 0);
		if (conn_fd < 0 || con_er) {
			if (conn_fd < 0 || con_er) {
				perror("Connecting");
				return 3;
			}
		}
		// fd is now connected
		// we want separate streams (which we can close independently)
		FILE* enemy_from=fdopen(enemy_fd, "r"); // read on enemy, enemy will write

		//while (conn_fd = accept(serv, 0, 0), conn_fd >= 0)   // change 0, 0 to get info about other end
			// play the game        
		FILE* stream = fdopen(conn_fd, "w"); //write on mine enemy will read
		
		int status = 0;
		int enemy_move = -1;
		for (int turns = 0; turns < 5; turns++) {
			int move_val = rand() % 3;
			snprintf(msg, 79, "%d\n", move_val);
			//printf("mymove=%s\n", msg);
			fputs(msg, stream);
			fflush(stream);
			//printf("getmove\n");
			fgets(msg, 79, enemy_from);
			//printf("%s\n", msg);
			//printf("%s", msg);
			sscanf(msg, "%d\n", &enemy_move);
			if (move_val == ROCK && enemy_move == PAPER) {
				status -= 1;
			} else if (move_val == ROCK && enemy_move == SCISSORS) {
				status += 1;
			} else if (move_val == PAPER && enemy_move == ROCK) {
				status += 1;
			} else if (move_val == PAPER && enemy_move == SCISSORS) {
				status -= 1;
			} else if (move_val == SCISSORS && enemy_move == ROCK) {
				status -= 1;
			} else if (move_val == SCISSORS && enemy_move == PAPER) {
				status += 1;
			}
			//printf("status=%d\n", status);
		}
		int done = 0;
		if (status == 0) {
			for (int turns = 0; turns < 15 && done == 0; turns++) {
				int move_val = rand() % 3;
				snprintf(msg, 79, "%d\n", move_val);
				//printf("mymove=%s\n", msg);
				fputs(msg, stream);
				fflush(stream);
				//printf("getmove\n");
				fgets(msg, 79, enemy_from);
				//printf("%s\n", msg);
				//printf("%s", msg);
				sscanf(msg, "%d\n", &enemy_move);
				if (move_val == ROCK && enemy_move == PAPER) {
					status -= 1;
					done = -1;
				} else if (move_val == ROCK && enemy_move == SCISSORS) {
					status += 1;
					done = -1;
				} else if (move_val == PAPER && enemy_move == ROCK) {
					status += 1;
					done = -1;
				} else if (move_val == PAPER && enemy_move == SCISSORS) {
					status -= 1;
					done = -1;
				} else if (move_val == SCISSORS && enemy_move == ROCK) {
					status -= 1;
				} else if (move_val == SCISSORS && enemy_move == PAPER) {
					status += 1;
					done = -1;
				}
				//printf("status=%d\n", status);
			}
		}
		char result[10] = {'\0'};
		char res_msg[10] = {'\0'};
		if (status == 0) {
			strcpy(result, "TIE");
			strcpy(res_msg, "TIE");
		} else if (status > 0) {
			strcpy(result, argv[1]);
			strcpy(res_msg, "WIN");
		} else {
			strcpy(result, enemy->name);
			strcpy(res_msg, "LOST");
		}
		//printf("status=%d:RESULT:%d:%s:num_matches=%d\n", status, enemy->match_id, result, num_matches); //send result to server
		fprintf(server_to, "RESULT:%d:%s\n", enemy->match_id, result); //send result to server
		fflush(server_to);
		char finish[50] = {'\0'};
		snprintf(finish, 49, "%d %s %s\n", enemy->match_id, enemy->name, res_msg);
		strcat(ending_message, finish);
		num_matches -= 1;
		fclose(stream);
		usleep(250000);
	}
	write(1, ending_message, strlen(ending_message)+1);
	//fprintf(STDOUT_2, "%s", ending_message);
	//fflush(STDOUT_2);
	fclose(STDOUT_2);
	fclose(stdout);
    return 0;
}