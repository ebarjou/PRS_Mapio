#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

enum action_enum {GET, SET_DIM, SET_OBJECT};
enum mapvar_enum {WIDTH, HEIGHT, OBJECTS, INFO};

void usage(){
	printf("maputil <file> --get[width, height, objects, info]\n");
	printf("maputil <file> --set[width, height] <size>\n");
	printf("maputil <file> --setobjects { <filename> <frames> <solidity> <destructible> <collectible> <generator> }\n");
	printf("maputil <file> --pruneobjects\n");
	exit(0);
}

int parseAction(char* arg){
	char* cmd = arg;
	if(cmd[0] == '-' && cmd[1] == '-'){
		cmd = &arg[2];
		if(cmd[1] == 'e' && cmd[2] == 't'){
			if(cmd[0] == 'g'){ // GET
				return GET;
			} else if(cmd[0] == 's'){ // SET
				cmd = &cmd[3];
				if (strcmp(cmd, "height") == 0 || strcmp(cmd, "width") == 0){
					return SET_DIM;
				} else if(strcmp(cmd, "objects") == 0){
					return SET_OBJECT;
				}
			}
		}
	}
	return -1;
}

int parseMapvar(char* arg){
	cmd = arg+5; // Skip "--[g/s]et"
	if (strcmp(cmd, "height") == 0){
		return HEIGHT;
	} else if(strcmp(cmd, "width") == 0){
		return WIDTH;
	} else if(strcmp(cmd, "objects") == 0){ // Shouldn't get to this else if --set
		return OBJECTS;
	} else if(strcmp(cmd, "info") == 0){
		return INFO;
	}
	return -1;
}

int main(int argc, char* argv[]){
	if (argc < 3) usage();
	printf("Ouverture du fichier %s\n", argv[1]);
	int action = parseAction(argv[2]);
	int type = parseMapvar(argv[2]);
	if(action == -1 || type == -1){
		usage();
	}

	switch (action){
		case GET:
			printf("Action GET\n");
			switch (type){
				case WIDTH:
					printf("Action WIDTH\n");
					break;
				case HEIGHT:
					printf("Action HEIGHT\n");
					break;
				case OBJECTS:
					printf("Action OBJECTS\n");
					break;
				case INFO:
					printf("Action INFO\n");
					break;
				case default:
					break;
			}
			break;
		case SET_DIM:
			printf("Action SET_DIM\n");
			switch (type){
				case WIDTH:
					printf("Action WIDTH\n");
					break;
				case HEIGHT:
					printf("Action HEIGHT\n");
					break;
				case default:
					break;
			break;
		case SET_OBJECT:
			printf("Action SET_OBJECT\n");
			break;
		case default:
			break;
	}

	return EXIT_SUCCESS;
}