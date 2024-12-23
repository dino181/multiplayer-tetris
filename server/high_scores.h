#ifndef HIGH_SCORES_H
#define HIGH_SCORES_H

#define MAX_SCORES 10
#define MAX_NAME_LENGTH 20

struct HighScore{
	char* name;
	int score;
};

struct HighScore** load_scores();

void save_scores(struct HighScore** scores);

void add_score(struct HighScore** scores, char* name, int score);

char* format_scores(struct HighScore** scores);

#endif
