#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "high_scores.h"

#define HIGH_SCORES_FILE "./high_scores.txt"

char* format_scores(struct HighScore** scores)
{
	char* leaderboard = (char*)malloc(sizeof(char) * 1024);
	int pos = 0;

	pos += sprintf(&leaderboard[pos], "SCORES ");

	for(int i = 0; i < 10; i++)
	{
		char* score;
		pos += sprintf(&leaderboard[pos], "%s,%d;", scores[i]->name, scores[i]->score);
	}
	return leaderboard;
}

struct HighScore** load_scores()
{
	FILE* fptr;
	char* line;
	size_t len;

	if(access(HIGH_SCORES_FILE, F_OK) != 0)
	{
		fptr = fopen(HIGH_SCORES_FILE, "w");
		fclose(fptr);
	}

	fptr = fopen(HIGH_SCORES_FILE, "r");

	struct HighScore** high_scores =
		(struct HighScore**)malloc(MAX_SCORES * sizeof(struct HighScore*));
	for(int i = 0; i < MAX_SCORES; i++)
	{
		high_scores[i] = (struct HighScore*)malloc(sizeof(struct HighScore));
		high_scores[i]->name = (char*)malloc(MAX_NAME_LENGTH * sizeof(char));

		if(getline(&line, &len, fptr) != -1)
		{
			char* name = strtok(line, ";");
			strcpy(high_scores[i]->name, name);

			int score = atoi(strtok(NULL, ";"));
			high_scores[i]->score = score;
		}
		else
		{
			strcpy(high_scores[i]->name, "no highscore yet");
			high_scores[i]->score = 0;
		}
	};

	fclose(fptr);
	free(line);

	return high_scores;
}

void save_scores(struct HighScore** scores)
{
	FILE* fptr;
	fptr = fopen("./high_scores.txt", "w");
	freopen("./high_scores.txt", "a", fptr);

	for(int i = 0; i < MAX_SCORES; i++)
	{
		fprintf(fptr, "%s;%d\n", scores[i]->name, scores[i]->score);
	}

	fclose(fptr);
}

void add_score(struct HighScore** scores, char* name, int score)
{
	for(int i = 0; i < MAX_SCORES; i++)
	{
		if(scores[i]->score >= score)
		{
			continue;
		}

		struct HighScore* replacement_score = scores[MAX_SCORES - 1];

		for(int j = MAX_SCORES - 1; j > i; j--)
		{
			scores[j] = scores[j - 1];
		}

		strcpy(replacement_score->name, name);
		replacement_score->score = score;
		scores[i] = replacement_score;

		break;
	}

	save_scores(scores);
}
