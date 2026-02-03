#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define LIMIT 30
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

void engine(Player *primary);
int modeDecision();
void addPlayersSoloMode();
void addPlayersTeamMode();
int strFormatCount(char *str);
void cleanedMessage(Message *msg, int);
void readMessageInput();
void helperFunc(FILE *file, enum effectType fx);
void changeHP(Player *player, enum effectType fx, int magnitude);
void printDaySummary();
Player *pickSecondaryPlayer(Player *primary);
void eliminate(Player *player);

int team_mode_on = 0;
char buffer[32] = {'\0'}; /*names*/
char bufferMsg[128] = {'\0'}; /*msgs from inputs*/
int totalPlayerCount = 0;
int remainingPlayers = 0;
int eachTeamHasMax = 0;

int main(){
	int commandToCont, i = 1; int j = 0, night = 0;
	srand(time(NULL));
	/*registering all the players whether it is team or solo mode */
	readMessageInput();
	printf("Team Mode? Y/N\n");
	if(modeDecision())
		addPlayersTeamMode();
	else
		addPlayersSoloMode();
	printDaySummary();
	remainingPlayers = totalPlayerCount;

	do{	// each day logic
		printf("Night #%d\n", night++);
		if(!team_mode_on){ // solo
			for(i = 0; i < 30; i++){
				if(soloPlayers[i] == NULL)
					continue;
				engine(soloPlayers[i]);
			}
		}
		else // team
			for(i = 0; i < 4; i++){
				for(j = 0; j < 10; j++){
					if(teams[i]->teamPlayers[j] == NULL)
						continue;
					engine(teams[i]->teamPlayers[j]);
				}
			}
		printDaySummary();
		if(remainingPlayers == 1)
			break;
		printf("1 to continue, 0 to stop: ");
		scanf("%d", &commandToCont);
	} while(remainingPlayers && commandToCont);
	if(remainingPlayers == 1){
		puts("well someone did win");
	}
}

void engine(Player *primary){
	Player *secondary = NULL;
	int randFx = rand() % 10;
	enum effectType fxPerPlayer = -1;
	double hiOrLo = 0;
	Message *toDisplay = NULL;
	int msgRandomizer = 0;
	//picking msg to display
	switch(randFx){
		case 0:
		case 1: 
			fxPerPlayer = MISC;
			// pick from miscHasNoOdds
			msgRandomizer = rand() % mhno;
			toDisplay = miscHasNoOdds[msgRandomizer];
			break;
		case 2: case 3: case 4: case 5:
		case 6: 
			fxPerPlayer = DAMAGE; 
			hiOrLo = (double)rand() / RAND_MAX;
			if(hiOrLo >= 0.8) {// high damage
				// pick from highDamageLowOdds
				msgRandomizer = rand() % hdlo;
				toDisplay = highDamageLowOdds[msgRandomizer];
			}
			else{
				// pick from lowDamageHighOdds
				msgRandomizer = rand() % ldho;
				toDisplay = lowDamageHighOdds[msgRandomizer];
			}
			break;
		case 7: case 8:
		case 9: 
			fxPerPlayer = HEAL;
			hiOrLo = (double)rand() / RAND_MAX;
			if(hiOrLo >= 0.7) {// high heal
				// pick from highHealLowOdds
				msgRandomizer = rand() % hhlo;
				toDisplay = highHealLowOdds[msgRandomizer];

			}
			else{
				// pick from lowHealHighOdds
				msgRandomizer = rand() % lhho;
				toDisplay = lowHealHighOdds[msgRandomizer];
			}
			break;
		default: puts("error.");
	}
	if(toDisplay->formatCount != 1){ // we need 1 more player to pick
		secondary = pickSecondaryPlayer(primary);
		printf(toDisplay->message, primary, secondary);
	}
	else
		printf(toDisplay->message, primary);
	changeHP(primary, toDisplay->fx, toDisplay->effectAmount);
}

void changeHP(Player *player, enum effectType fx, int magnitude){
	if(player == NULL)
		return;
	if(fx == HEAL){
		player->HP = (magnitude + player->HP >= 100) ? 100: magnitude + player->HP;
		printf("%s +%d\n", player->name, magnitude);
	}
	else if(fx == DAMAGE){
		player->HP -= magnitude;
		printf("%s -%d\n", player->name, magnitude);
		if(player->HP <= 0)
			eliminate(player);
	}	
}

void readMessageInput(){ // fills formatCount, fx
	FILE *healFile = fopen("healmessages.txt", "r");
	FILE *damageFile = fopen("damagemessages.txt", "r");
	FILE *miscFile = fopen("miscmessages.txt", "r");

	helperFunc(healFile, HEAL);	
	helperFunc(damageFile, DAMAGE);
	helperFunc(miscFile, MISC);
}

void helperFunc(FILE *file, enum effectType fx){
	while(fgets(bufferMsg, sizeof(bufferMsg), file) != NULL) {
		Message *append = (Message *)calloc(1, sizeof(Message));
		if(append != NULL){
			cleanedMessage(append, fx);
			append->fx = fx;
			append->formatCount = strFormatCount(append->message);
		}
	}
}
// fills message and effectAmount, also fills all msg arrays
void cleanedMessage(Message *msg, int fx){ 
	if(fx == HEAL || fx == DAMAGE){ // not misc
		msg->effectAmount = atoi(bufferMsg);
		const char *ptr = bufferMsg;
		for(; *ptr != '\0' && (isdigit(*ptr) || *ptr == '-' || *ptr == ' '); ptr++);
		strcpy_s(msg->message, sizeof(msg->message), ptr);
		if(fx == HEAL){
			if(msg->effectAmount > 40){
				highHealLowOdds[hhlo++] = msg;
			}
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
		strcpy_s(msg->message, sizeof(msg->message), bufferMsg);
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
		temp->HP = 100;
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
	eachTeamHasMax = team_size;
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
			temp->HP = 100;
			printf("%dth player in team #%d namely %s\n",
				i + 1, teamNo + 1, teams[teamNo]->teamPlayers[i]->name);
			totalPlayerCount++;
		}
	}
}

Player * pickSecondaryPlayer(Player *primary){
	int i, j;
	int forbiddenTeam = 0;
	Player *returnThis = NULL;
	// secondary CANNOT be the same as primary.
	if(!team_mode_on){ // solo
		int pseudoPtr = rand() % 30;
		while(soloPlayers[pseudoPtr] == NULL || soloPlayers[pseudoPtr] == primary)
				pseudoPtr = rand() % 30;
		
		return soloPlayers[pseudoPtr];
	}
	// secondary CANNOT be in the same team
	else{ // team
		for(i = 0; i < 4; i++)
			for(j = 0; j < 10; j++)
				if(teams[i]->teamPlayers[j] == primary){
					forbiddenTeam = i;
					break;
				}
		while(returnThis == NULL){
			returnThis = teams[(forbiddenTeam + (rand() % 3)) % 4]->teamPlayers[rand() % eachTeamHasMax];
		}
		return returnThis;
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

void printDaySummary(){
	int i, j = 0;
	puts("Remaining Players with their HP");
	if(!team_mode_on) // solo
		for(i = 0; i < 30; i++){
			if(soloPlayers[i] == NULL)
				continue;
			printf("%s: %d\n", soloPlayers[i]->name, soloPlayers[i]->HP);
		}
	else // team mode
		for(i = 0; i < 4; i++){
			for(j = 0; j < 10; j++){
				if(teams[i]->teamPlayers[j] == NULL)
					continue;
				printf("Team: %d %s: %d\n", i, 
					teams[i]->teamPlayers[j]->name, teams[i]->teamPlayers[j]->HP);
			}
		}
}