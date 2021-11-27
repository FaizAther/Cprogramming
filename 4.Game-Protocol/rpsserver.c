#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define WAITING 0
#define PLAYING 1
#define DEAD 2

#define WON 2
#define LOST 3
#define TIE 4
#define ERR 5
#define ACTIVE 6
#define FINISH 7

typedef struct _player *Player;

typedef struct _player {
	int id;
	char name[10];
	char port[10];
	int status;
	int match_id;
	Player enemy;
	int stats[3];
} player;

typedef struct _match *Match;

typedef struct _match {
	Player p1;
	Player p2;
	int status;
	int id;
} match;

typedef struct _playerInfo *PlayerInfo;

typedef struct _playerInfo {
	int total_players;
	int active;
	int engaged;
	Player players[1000];
	Match matches[500];
	int total_matches;
	int active_matches;
} playerInfo;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

PlayerInfo playerInformation;

FILE *STDOUT_2;

void *handle_connection(void *conn_fd_ptr);

void handler(int num) {
	//print out to stdout the players and status
	char output[100] = {'\0'};
	Player curr, prev, tmp, final;
	Player arr[1000];
	for (int i = 0; i < playerInformation->total_players; i++) {
		arr[i] = playerInformation->players[i];
	}
	for (int i = 1; i < playerInformation->total_players; i++) {
		for (int j = 1; j < playerInformation->total_players; j++) {
			if (strcmp(arr[j - 1]->name, arr[j]->name) > 0) {
				//swap
				tmp = arr[j - 1];
				arr[j - 1] = arr[j];
				arr[j] = tmp;
			}
		}
	}
	for (int i = 0; i < playerInformation->total_players; i++) {
		//fprintf(stdout, "%s %d %d %d\n", arr[i]->name, arr[i]->stats[0], arr[i]->stats[1], arr[i]->stats[2]);
		//fflush(stdout);
		snprintf(output, 99, "%s %d %d %d\n", arr[i]->name, arr[i]->stats[0], arr[i]->stats[1], arr[i]->stats[2]);
		write(1, output, strlen(output)+1);
	}
	//fprintf(stdout, "---\n");
	//fflush(stdout);
	//exit(0);
	strcpy(output, "---\n");
	write(1, output, strlen(output)+1);
	//fflush(stdout);
	//exit(0);
}

int main(int argc, char** argv) {
    // See if a portnum was specified, otherwise use zero (ephemeral)
	if (argc!=1) {
        fprintf(stderr, "Usage: rpsserver\n");
        return 1;
    }

	FILE *STDOUT_2 = fdopen(1, "w");

	playerInformation= malloc(sizeof(struct _playerInfo));
	playerInformation->total_players = 0;
	playerInformation->total_matches = 1;
	playerInformation->active = 0;
	playerInformation->engaged = 0;

	signal(SIGHUP, handler);

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
        return 1;   // could not work out the address
    }
    
        // create a socket and bind it to a port
    int serv = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (bind(serv, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        perror("Binding");
        return 3;
    }
    
        // Which port did we get?
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len=sizeof(struct sockaddr_in);
    if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
        perror("sockname");
        return 4;
    }
    printf("%u\n", ntohs(ad.sin_port));
    
    if (listen(serv, 100)) {     // allow up to 100 connection requests to queue
        perror("Listen");
        return 4;
    }
    
    int conn_fd;
    while (conn_fd = accept(serv, 0, 0), conn_fd >= 0) {    // change 0, 0 to get info about other end
		pthread_t t;
		int *pclient = malloc(sizeof(int));
		*pclient = conn_fd;
		pthread_create(&t, NULL, handle_connection, pclient);
    }
	fclose(STDOUT_2);
	fclose(stdout);
    return 0;
}


void *handle_connection(void *conn_fd_ptr) {
	int conn_fd = *((int*)conn_fd_ptr);
	free(conn_fd_ptr);

    int fd2=dup(conn_fd);
    FILE* to=fdopen(conn_fd, "w");
    FILE* from=fdopen(fd2, "r");

	char msg_from[80] = {'\0'};
	fgets(msg_from, 79, from);
	Player player = malloc(sizeof(*player));
	player->enemy = NULL;
	player->match_id = -1;
	player->status = WAITING;
	player->stats[0] = player->stats[1] = player->stats[2] = 0;
	char tok[80] = {'\0'};
	strcpy(tok, msg_from);
	int tok_n = 0;
	int j = 0;
	for (int i = 0; i <= strlen(msg_from)+1; i++) {
		if (msg_from[i] == ':' || msg_from[i] == '\n' || msg_from[i] == '\0') {
			tok[j] = '\0';
			if (tok_n == 0 && strcmp("MR", tok) != 0) {
				//error
			} else if (tok_n == 1) {
				strcpy(player->name, tok);
			} else if (tok_n == 2) {
				strcpy(player->port, tok);
			}
			tok_n += 1;
			strcpy(tok, tok + j + 1);
			j = -1;
		}
		j += 1;
	}


	
	//critical section: add to global player data
	Player copyPlayer, prevPlayer;
	int found = -1;	
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < playerInformation->total_players; i++) {
		copyPlayer = playerInformation->players[i];
		if (copyPlayer != NULL && strcmp(copyPlayer->name, player->name) == 0 && copyPlayer->status != DEAD) {
			free(player);
			fputs("BADNAME\n",to);
			fflush(to);
		    fclose(to);
		    fclose(from);
			pthread_mutex_unlock(&mutex);
			return NULL;
		} else if (copyPlayer != NULL && strcmp(copyPlayer->name, player->name) == 0 && copyPlayer->status == DEAD) {
			found = 0;
			prevPlayer = copyPlayer;
		}
	}
	if (found == -1) {
		player->id = playerInformation->total_players;
		playerInformation->players[playerInformation->total_players] = player;
		playerInformation->total_players += 1;
		playerInformation->active += 1;
	} else {
		prevPlayer->status = WAITING;
		strcpy(prevPlayer->port, player->port);
		free(player);
		player = prevPlayer;
	}
	pthread_mutex_unlock(&mutex);
	//find opponent || matched
	Player foundPlayer = NULL;
	Match match = NULL;
	while (foundPlayer == NULL) {
		pthread_mutex_lock(&mutex);
		//printf("2 mutex locked on %s", player->name);
		if (player->enemy != NULL) {
			foundPlayer = player->enemy;
			match = playerInformation->matches[player->match_id];
		} else if (playerInformation->active - 1 > 0) {
			Player searchPlayer = NULL;
			int i = 0;
			while (i < playerInformation->total_players) {
				searchPlayer = playerInformation->players[i];
				if (searchPlayer != NULL && searchPlayer->id != player->id && searchPlayer->status == WAITING) {
					foundPlayer = searchPlayer;
					match = malloc(sizeof(struct _match));
					match->p1 = player;
					match->p2 = foundPlayer;
					match->status = ACTIVE;
					match->id = playerInformation->total_matches;
					playerInformation->matches[playerInformation->total_matches] = match;
					playerInformation->total_matches += 1;
					playerInformation->active -= 2;
					playerInformation->active_matches += 1;
					match->p1->status = PLAYING;
					match->p2->status = PLAYING;
					match->p1->match_id = match->id;
					match->p2->match_id = match->id;
					player->enemy = foundPlayer;
					foundPlayer->enemy = player;
				}
				i++;
			}
		}
		pthread_mutex_unlock(&mutex);
		//usleep(20);
	}

    char msg_to[80] = {'\0'};
	snprintf(msg_to, 79, "MATCH:%d:%s:%s\n", match->id, player->enemy->name, player->enemy->port);
    fputs(msg_to, to);
    fflush(to);

	fgets(msg_from, 79, from);
	//printf("%s", msg_from); //RESULT:<GAME_ID>:<WIN_NAME||TIE>
	char winName[20] = {'\0'};
	char tokk[80] = {'\0'};
	strcpy(tokk, msg_from);
	tok_n = 0;
	j = 0;
	for (int i = 0; i <= strlen(msg_from)+1; i++) {
		if (msg_from[i] == ':' || msg_from[i] == '\n' || msg_from[i] == '\0') {
			tokk[j] = '\0';
			if (tok_n == 2) {
				strcpy(winName, tokk);
			}
			tok_n += 1;
			strcpy(tokk, tokk + j + 1);
			j = -1;
		}
		j += 1;
	}
	//printf("playing %s, winner %s\n", player->name, winName);
	if (strcmp(player->name, winName) == 0) {
		//printf("here\n");
		player->stats[0] += 1;
	} else if (strcmp(winName, "TIE") == 0) {
		player->stats[2] += 1;
	} else {
		player->stats[1] += 1;
	}
	//printf("2 mutex locked on %s", player->name);
	pthread_mutex_lock(&mutex);
	if (match->status == ACTIVE) {
		playerInformation->active_matches -= 1;
	}
	match->status = FINISH;
	player->status = DEAD;
	//playerInformation->players[player->id] = NULL;
	//free(player);
	playerInformation->active += 1;
	pthread_mutex_unlock(&mutex);
	player->enemy = NULL;
	player->match_id = -1;
	strcpy(player->port, "");
	//wait until enough player to match
	//send opponent creds

    fclose(to);
    fclose(from);

	return NULL;
}