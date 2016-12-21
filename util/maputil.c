#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

enum action_enum {GET, SET_DIM, SET_OBJECT, PRUNE};
enum mapvar_enum {WIDTH, HEIGHT, OBJECTS, INFO};

void usage(){
	printf("maputil <file> --get[width, height, objects, info]\n");
	printf("maputil <file> --set[width, height] <size>\n");
	printf("maputil <file> --setobjects { <filename> <frames> <solidity> <destructible> <collectible> <generator> }\n");
	printf("maputil <file> --pruneobjects\n");
	exit(0);
}

void get(int fd, int type) {
	int error = 0;
	int width, height;
	// Read height and width
	read(fd, &width, sizeof(int))<0?++error:error;
	read(fd, &height, sizeof(int))<0?++error:error;
	if (type == WIDTH || type == INFO){
		printf("width : %d\n", width);
		if (type == WIDTH) return;
	}
	if (type == HEIGHT || type == INFO){
		printf("height : %d\n", height);
		if (type == HEIGHT) return;
	}
	
	int nb_obj, solidity, destructibility, collectibility, generability;
	unsigned nb_sprites;
	
	// Read the number of different objects
	read(fd, &nb_obj, sizeof(int))<0?++error:error;
	printf("objects : %d\n", nb_obj);
	// Read the characteristics of the objects and create it
	for(int i = 0 ; i < nb_obj ; i++) {
		int file_length;
		read(fd, &file_length, sizeof(int))<0?++error:error;
		char obj_filename[file_length+1];
		for (int j = 0 ; j < file_length ; j++)
			read(fd, &obj_filename[j], sizeof(char))<0?++error:error;
		obj_filename[file_length]='\0';
		//read(fd, obj_filename, file_length*sizeof(char));
		
		read(fd, &nb_sprites, sizeof(unsigned))<0?++error:error; // The number of frames/sprites
		
		read(fd, &solidity, sizeof(int))<0?++error:error; // The solidity of the object (0|1|2)
		
		read(fd, &destructibility, sizeof(int))<0?++error:error; // The destructibility of the object (0|1)
		
		read(fd, &collectibility, sizeof(int))<0?++error:error; // The collectibility of the object (0|1)
		
		read(fd, &generability, sizeof(int))<0?++error:error; // The generability of the object (0|1)
		
		if (type == OBJECTS || type == INFO){
			printf("Object : %s\n", obj_filename);
			printf("\tSprites : %d\n\tSolidity : %d\n\tDestructibility : %d\n\tCollectibility : %d\n\tGenerability : %d\n\t",
					nb_sprites, solidity, destructibility, collectibility, generability);
		}
	}
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
		} else if (strcmp(cmd, "pruneobjects")){
			return PRUNE;
		}
	}
	return -1;
}

int parseMapvar(char* arg){
	char* cmd = arg+5; // Skip "--[g/s]et"
	if (strcmp(cmd, "height") == 0){
		return HEIGHT;
	} else if(strcmp(cmd, "width") == 0){
		return WIDTH;
	} else if(strcmp(cmd, "objects") == 0){
		return OBJECTS;
	} else if(strcmp(cmd, "info") == 0){ // Shouldn't get to this else if --set
		return INFO;
	}
	return -1;
}

int main(int argc, char* argv[]){
	if (argc < 3) usage();
	int action, type, fd;
	printf("Ouverture du fichier %s\n", argv[1]);
	action = parseAction(argv[2]);
	if(action == -1){
		usage();
	}
	if (action != PRUNE){
		type = parseMapvar(argv[2]);
		if(type == -1){
			usage();
		}
	}
	switch (action){
		case GET:
			printf("Action GET\n");
			fd = open(argv[1], O_RDONLY);
			get(fd, type);
			break;
		case SET_DIM:
			printf("Action SET_DIM\n");
			fd = open(argv[1], O_RDWR);
			switch (type){
				case WIDTH:
					printf("Action WIDTH\n");
					break;
				case HEIGHT:
					printf("Action HEIGHT\n");
					break;
				default:
					break;
			}
			break;
		case SET_OBJECT:
			printf("Action SET_OBJECT\n");
			fd = open(argv[1], O_RDWR);
			break;
		case PRUNE:
			printf("Action PRUNE\n");
			fd = open(argv[1], O_RDWR);
			break;
		default:
			break;
	}

	return EXIT_SUCCESS;
}
