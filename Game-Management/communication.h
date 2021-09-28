

#ifndef MESSAGES_H_
#define MESSAGES_H_

#define YT_M          "YT"
#define OK_M          "OK"
#define GUESS_M       "GUESS"
#define HIT_M         "HIT"
#define MISS_M        "MISS"
#define SUNK_M        "SUNK"
#define RULES_M       "RULES"
#define MAP_M         "MAP"
#define EARLY_M       "EARLY"
#define DONE_M        "DONE"

void
communication_beacon();

void
communication_message();

void
communication_signal();

#endif //MESSAGES_H_