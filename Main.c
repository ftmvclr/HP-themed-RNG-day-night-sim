#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define LIMIT 10
typedef struct player{
	char name[32];
	int HP;
}Player;
Player *soloPlayers[30] = {NULL};

typedef struct team{
	Player *teamPlayers[10];
}Team;
Team *teams[5] = {NULL};

int modeDecision();
void addPlayersSoloMode();
void addPlayersTeamMode();
void printAllPlayers();

int team_mode_on = 0;
char buffer[32] = {'\0'};

int main(){
	printf("Team Mode? Y/N\n");
	if(modeDecision()){
		addPlayersTeamMode();
	}
	else
		addPlayersSoloMode();

}
int modeDecision(){
	char mode = '\0';
	do{
		scanf(" %c", &mode); // space then c, got it
		while((getchar()) != '\n'); // this discards any trailing char

		if(toupper(mode) == 'Y'){
			team_mode_on = 1;
			return 1;
		}
		else if(toupper(mode) == 'N') break;
		else{
			printf("Character preferably though\n");
			mode = '\0';
		}
	} while(!mode);
	return 0;
}

void addPlayersSoloMode(){
	int i;
	printf("Add players to the Harry Potter simulation\n* to stop\n");
	for(i = 0; i < LIMIT; i++){
		scanf("%31s", buffer);
		if(buffer[0] == '*'){
			printf("Okay, there are (%d) players in.\n", i);
			break;
		}
		Player *temp = (Player *)malloc(sizeof(Player));
		if(temp == NULL)
			exit(111);
		soloPlayers[i] = temp;
		strcpy_s(temp->name, 32, buffer);
		printf("%dth player namely %s\n", i + 1, soloPlayers[i]->name);
	}
	if(i == LIMIT)
		puts("(reached the limit for max players)");
}

void addPlayersTeamMode(){
	int team_size, i, teamNo = 0;
	puts("team consists of max ___ players.");
	scanf("%d", &team_size);
	for(i = 0; i < 5; i++){
		teams[i] = (Team *)malloc(sizeof(Team));
		if(teams[i] == NULL)
			exit(89);
	}
	for(; teamNo < 5; teamNo++){
		for(i = 0; i < team_size; i++){
			printf("Enter player #%d for team #%d\n* for next team\n\\for exitting menu\n", i + 1, teamNo + 1);
			scanf(" %31[^\n]", buffer);
			if(buffer[0] == '*'){
				printf("Okay team #%d is complete\n", teamNo + 1);
				break;
			}
			if(buffer[0] == '\\'){
				printf("Okay there are %d teams\n", teamNo + 1);
				teamNo = 5; // to get rid of the outer loop
				break;
			}
			Player *temp = (Player *)malloc(sizeof(Player));
			if(temp == NULL)
				exit(111);
			(teams[teamNo])->teamPlayers[i] = temp;
			strcpy_s(temp->name, 32, buffer);
			printf("%dth player in team #%d namely %s\n",
				i + 1, teamNo + 1, teams[teamNo]->teamPlayers[i]->name);
		}
	}
}

void printAllPlayers(){
	if(team_mode_on){

	}
	else{

	}
}