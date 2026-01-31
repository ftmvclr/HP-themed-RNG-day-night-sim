#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define LIMIT 10
enum effectType {MISC, HEAL, DAMAGE};

typedef struct player{
	char name[32];
	int HP;
}Player;
Player *soloPlayers[30] = {NULL};

typedef struct team{
	Player *teamPlayers[10];
}Team;
Team *teams[4] = {NULL};

typedef struct message{
	char message[128];
	enum effectType fx;
	int effectAmount;
	int formatCount; // number of %s's
}Message;
/*5*/
Message *highDamageLowOdds[10]; int hdlo; /*20%*/ // (more than 50 damage)
Message *lowDamageHighOdds[10]; int ldho; /*80%*/
/*3*/
Message *highHealLowOdds[10];	int hhlo; /*30%*/ // (more than 40 hp)
Message *lowHealHighOdds[10];	int lhho; /*70%*/
/*2*/
Message *miscHasNoOdds[20];		int mhno; /*100%*/


void engine();
int modeDecision();
void addPlayersSoloMode();
void addPlayersTeamMode();
void printAllPlayers();
int strFormatCount(char *str);
void cleanedMessage(Message *msg, int);
void readMessageInput();
void helperFunc(FILE *file, Message *append, enum effectType fx);
void changeHP(Player *player, enum effectType fx, int magnitude);
void eliminate(Player *player);

int team_mode_on = 0;
char buffer[32] = {'\0'}; /*names*/
int totalPlayerCount = 0;
int remainingPlayers = 0;
/*
i think each day we should loop through every player in single mode
and in teams mode i think looping through teams is enough
1 person selected from each team or like maybe 2 if it is a crowded lobby
ok so we have formatCount and that is gonna come in handy
i will make sure to make it so the first %s is the one taking damage in case of formatCount = 2
*/

/*
check if effectAmount 0, then print directly (misc perhaps)
if positive check type then do whatever
if negative check type then do whatever
usually the second %s won't matter
ADD more though
also TODO: i messed up the msg files 
*/
int main(){
	int commandToCont, i = 1;
	/*registering all the players whether it is team or solo mode */
	readMessageInput();
	printf("Team Mode? Y/N\n");
	if(modeDecision())
		addPlayersTeamMode();
	else
		addPlayersSoloMode();
	printAllPlayers();
	remainingPlayers = totalPlayerCount;
	// each day logic
	do{
		printf("Night #%d", i++);
		engine();
		printf("1 to continue, 0 to stop: ");
		scanf("%d", &commandToCont);
	} while(remainingPlayers && commandToCont);
}

void engine(){
	/* select the next player.
	 roll a dice to decide between the big 3
	 then if it is not MISC decide if it is going 
	 to be low or high odds
	 select the message AND THEN 
	 select 2ndary player if necessary (formatCount > 1)
	*/
}

void changeHP(Player *player, enum effectType fx, int magnitude){
	if(fx == HEAL){
		player->HP = (magnitude + player->HP) % 100;
	}
	else if(fx == DAMAGE){
		player->HP -= magnitude;
		if(player->HP <= 0)
			eliminate(player);
	}	
}

void readMessageInput(){ // fills formatCount, fx
	FILE *healFile = fopen("healmessages.txt", "r");
	FILE *damageFile = fopen("damagemessages.txt", "r");
	FILE *miscFile = fopen("miscmessages.txt", "r");
	Message *append = NULL;

	helperFunc(healFile, append, HEAL);
	helperFunc(damageFile, append, DAMAGE);
	helperFunc(miscFile, append, MISC);
}

void helperFunc(FILE *file, Message *append, enum effectType fx){
	while(fgets(buffer, sizeof(buffer), file) != NULL) {
		append = (Message *)calloc(1, sizeof(Message));
		if(append != NULL){
			cleanedMessage(append, fx);
			append->fx = fx;
			append->formatCount = strFormatCount(append->message);
		}
	}
}
// fills message and effectAmount, also fills all msg arrays
void cleanedMessage(Message *msg, int fx){ 
	if(fx){ // not misc
		msg->effectAmount = atoi(buffer);
		const char *ptr = buffer;
		for(; *ptr != '\0' && (isdigit(*ptr) || *ptr == '-' || *ptr == ' '); ptr++);
		strcpy_s(msg->message, sizeof(msg->message), ptr);
		if(fx == HEAL){
			if(msg->effectAmount > 40)
				highHealLowOdds[hhlo++] = msg;
			else
				lowHealHighOdds[lhho++] = msg;
		}
		else if(fx == DAMAGE){
			if(msg->effectAmount > 50)
				highDamageLowOdds[hdlo++] = msg;
			else
				lowDamageHighOdds[ldho++] = msg;
		}
	}
	else{ // misc, no need for taking the int out
		strcpy_s(msg->message, sizeof(msg->message), buffer);
		miscHasNoOdds[mhno++] = msg;
	}
}

void addPlayersSoloMode(){
	int i = 0;
	printf("Add players to the Harry Potter simulation\n* to stop\n");
	for(i = 0; i < LIMIT; i++){
		scanf(" %31[^\n]", buffer);
		if(buffer[0] == '*'){
			printf("Okay, there are (%d) players in.\n", i);
			break;
		}
		Player *temp = (Player *)calloc(1, sizeof(Player));
		if(temp == NULL)
			exit(111);
		soloPlayers[i] = temp;
		strcpy_s(temp->name, 32, buffer);
		printf("%dth player namely %s\n", i + 1, soloPlayers[i]->name);
		totalPlayerCount++;
	}
	if(i == LIMIT)
		puts("(reached the limit for max players)");
}

void addPlayersTeamMode(){
	int team_size, i, teamNo = 0;
	puts("team consists of max ___ players.");
	scanf("%d", &team_size);
	for(i = 0; i < 4; i++){
		teams[i] = (Team *)calloc(1, sizeof(Team));
		if(teams[i] == NULL)
			exit(89);
	}
	printf("*for next team\n\\for exitting menu\n");
	for(; teamNo < 4; teamNo++){
		for(i = 0; i < team_size; i++){
			printf("Enter player #%d for team #%d\n", i + 1, teamNo + 1);
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
			Player *temp = (Player *)calloc(1, sizeof(Player));
			if(temp == NULL)
				exit(111);
			(teams[teamNo])->teamPlayers[i] = temp;
			strcpy_s(temp->name, 32, buffer);
			printf("%dth player in team #%d namely %s\n",
				i + 1, teamNo + 1, teams[teamNo]->teamPlayers[i]->name);
			totalPlayerCount++;
		}
	}
}

//prolly should print their HP too 
void printAllPlayers(){
	int i, j = 0;
	if(team_mode_on){
		for(i = 0; teams[i] != NULL ; i++) // iterating teams
			for(j = 0; teams[i]->teamPlayers[j] != NULL ; j++) // iterating team players
				printf("team: %d player: %d  :  %s\n", i + 1, j + 1, teams[i]->teamPlayers[j]->name);
	}
	else{
		for(i = 0; soloPlayers[i] != NULL; i++){
			printf("%d: %s\n", i + 1, soloPlayers[i]->name);
		}
	}
}

void eliminate(Player *player){
	/*gotta remove from logic to prevent them from taking actions*/
	int i, j = 0;
	if(team_mode_on){
		for(j = 0; j < 4; j++)
			for(i = 0; i < 10; i++){
				if(teams[j]->teamPlayers[i] == player){
					// remove and break
					teams[j]->teamPlayers[i] = NULL;
					j = 5;
					printf("%s has been eliminated today.\n", player->name);
					remainingPlayers--;
					break;
				}
			}
	}
	else{
		for(i = 0; i < totalPlayerCount; i++){
			if(player == soloPlayers[i]){
 				//remove and break
				printf("%s has been eliminated today.\n", player->name);
				soloPlayers[i] = NULL;
				remainingPlayers--;
				break;
			}
			
		}
	}
}

int strFormatCount(char *str){
	int count = 0;
	while((str = strstr(str, "%s")) != NULL) {
		count++;
		str += 2; // 2 because % and s
	}
	return count;
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
