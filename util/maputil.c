#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

enum action_enum {GET, SET_DIM, SET_OBJECT};
enum mapvar_enum {WIDTH, HEIGHT, OBJECTS, INFO}

void usage(){
	printf("maputil <file> --get[width, height, objects, info]\n");
	printf("maputil <file> --set[width, height] <size>\n");
	printf("maputil <file> --setobjects { <filename> <frames> <solidity> <destructible> <collectible> <generator> }\n");
	printf("maputil <file> --pruneobjects\n");
	exit(0);
}

int parseArgs(char* arg){
	int action;
	char* cmd = arg;
	if(cmd[0] == '-' && cmd[1] == '-'){
		cmd = &arg[2];
		if(cmd[1] == 'e' && cmd[2] == 't'){
			if(cmd[0] == 'g'){ // GET
				action = GET;
				cmd = &cmd[3];
				printf("GET : %s\n", cmd);
				return action;
			} else if(cmd[0] == 's'){ // SET
				cmd = &cmd[3];
				if (strcmp(cmd, "height") == 0 || strcmp(cmd, "width") == 0){
					action = SET_DIM;
					printf("SET_DIM : %s\n", cmd);
					return action;
				} else if(strcmp(cmd, "objects") == 0){
					action = SET_OBJECT;
					printf("SET : %s\n", cmd);
					return action;
				}
			}
		}
	}
	return -1;
}

int main(int argc, char* argv[]){
	if (argc < 3) usage();
	int action;
	printf("Ouverture du fichier %s\n", argv[1]);
	action = parseArgs(argv[2]);

	switch (action){
		case (-1):
			printf("Erreur : argument invalide\n");
			break;
		case GET:
			printf("Action GET\n");
			break;
		case SET_DIM:
			printf("Action SET_DIM\n");
			break;
		case SET_OBJECT:
			printf("Action SET_OBJECT\n");
			break;
	}

	return EXIT_SUCCESS;
}