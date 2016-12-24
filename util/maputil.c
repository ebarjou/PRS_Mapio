#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

enum action_enum {GET, SET_DIM, SET_OBJECT, PRUNE};
enum mapvar_enum {WIDTH, HEIGHT, OBJECTS, INFO};
enum object_carac {SOLID, DESTRU, COLLEC, GENER};

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
			printf("\tSprites : %d\n\tSolidity : %d\n\tDestructibility : %d\n\tCollectibility : %d\n\tGenerability : %d\n",
					nb_sprites, solidity, destructibility, collectibility, generability);
		}
	}
}

void setObjects(int fd, int nbval, char* val[]) {
	//TODO Fonctionne uniquement pour ajouter un objet en l'état
	//val[6] : filename, frames, solidity, destructability, collectibility, generator
	char* file = "filename";//val[0];
	unsigned frames = 1;//(unsigned)atoi(val[1]);
	int param[4];//solidity, destruct, collec, gener
	for (int i = 0; i < 4; ++i) param[i] = 1;
	//parse args
	for (int i = 0; i < nbval; ++i){
		printf("%s\n", val[i]);
		if (strcmp(val[i], "solid") == 0){
			if (!strcmp(val[i], "not-") == 0){
				param[1] = 1;
			}
		}
	}
	
	//TODO Erreur lors de la réécriture - Vérifier si c'est corrigé
	//Read then write new object number
	int nb_obj, width, height;
	read(fd, &width, sizeof(int));
	read(fd, &height, sizeof(int));
	read(fd, &nb_obj, sizeof(int));
	lseek(fd, sizeof(int)*2, SEEK_SET);
	nb_obj+= nbval;
	write(fd, &nb_obj, sizeof(int));
	// Remember how the objects are placed on the map,
	// which are written after their characteristics
	lseek(fd, -sizeof(int)*width+height, SEEK_END);
	int obj_on_map[height][width];
	for (int y = 0 ; y < height ; y++) { // For each square in the map
	    for (int x = 0 ; x < width ; x++) {
	        read(fd, obj_on_map[height]+height, sizeof(int));
	    }
	}
	//write new object(s)
	lseek(fd, -sizeof(int)*width+height, SEEK_END);
	for(int i = 0 ; i < nbval ; i++) {
		//val[6] : filename, frames, solidity, destructability, collectibility, generator
		char* file = val[0+i*6];//"filename";
		unsigned frames = (unsigned)atoi(val[1+i*6]);//1;
		int param[4];//solidity, destruct, collec, gener
		for (int j = 0; j < 4; ++j) param[j] = 0;
		//parse args
		for (int j = 0; j < 4; ++j){
			printf("%s\n", val[j+i*6+2]);
			val[j+i*6+2][4] = 0;
			// Only need to verify if the "not-" is here, we can verify
			// the rest elsewhere if we want to make sure it's typed correctly
			if (strcmp(val[i+i*6+2], "not-") != 0){
				param[i] = 1;
			}
		}
	    write(fd, file, 8*sizeof(char));
	    write(fd, &frames, sizeof(unsigned));
	    for (int i = 0; i < 4; ++i) {
	        write(fd, &param[i], sizeof(int));
	    }
	}
	// Rewrite the objects in their position on the map
	for (int y = 0 ; y < height ; y++) { // For each square in the map
	    for (int x = 0 ; x < width ; x++) {
	        write(fd, obj_on_map[height]+height, sizeof(int));
	    }
	}
}

void setDim(int fd, int type, int newVal) {
	int oldHeight, oldWidth, newHeight, newWidth;
	read(fd, &oldWidth, sizeof(int));
	read(fd, &oldHeight, sizeof(int));
	lseek(fd, 0, SEEK_SET);
	if (type == WIDTH){
		write(fd, &newVal, sizeof(int));
		printf("Set width : %d\n", newVal);
		newWidth = newVal;
		newHeight = oldHeight;
	}
	if (type == HEIGHT){
		lseek(fd, sizeof(int), SEEK_SET);
		write(fd, &newVal, sizeof(int));
		printf("new height : %d\n", newVal);
		newWidth = oldWidth;
		newHeight = newVal;
	}
	//Now we need to move all objects on the map to match the new size
	//	TODO : Nettoyer le code, remplacer les read par des lseek
	int null, nb_obj;
	lseek(fd, 0, SEEK_SET);
	read(fd, &null, sizeof(int));
	read(fd, &null, sizeof(int));
	read(fd, &nb_obj, sizeof(int));
	for(int i = 0; i < nb_obj; i++) {
		int file_length;
		read(fd, &file_length, sizeof(int));
		char obj;
		for (int j = 0 ; j < file_length ; j++)
			read(fd, &obj, sizeof(char));
		read(fd, &null, sizeof(unsigned));
		read(fd, &null, sizeof(int));
		read(fd, &null, sizeof(int));
		read(fd, &null, sizeof(int));
		read(fd, &null, sizeof(int));
	}
	//Init 2D array that represent the new map
	int** objMap;
	objMap = malloc(sizeof(int*)*newWidth);
	for (int i = 0; i < newWidth; ++i) objMap[i] = malloc(sizeof(int)*newHeight);
	//Init the map empty
	for (int y = 0 ; y < newHeight ; y++)
		for (int x = 0 ; x < newWidth ; x++) objMap[x][y] = -1;
	//Save the location of the grid and put old value in the new grid
	int position = lseek(fd, 0, SEEK_CUR);
	for (int y = 0 ; y < oldHeight ; y++) {
		for (int x = 0 ; x < oldWidth ; x++) {
			int obj_val;
			read(fd, &obj_val, sizeof(int));
			if (x < newWidth && y < newHeight) {
				objMap[x][y] = obj_val;
			}
		}
	}
	//And we rewrite the new value
	lseek(fd, position, SEEK_SET);
	for (int y = 0 ; y < newHeight ; y++) {
		for (int x = 0 ; x < newWidth ; x++) {
			printf("%d.", objMap[x][y]==-1?0:1);
			write(fd, &objMap[x][y], sizeof(int));
		}
		printf("\n");
	}
	//Now we truncate the file if needed (new size < old size)
	//TODO : si le fichier est plus, petit, ftruncate(fd, size);
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
		} else if (strcmp(cmd, "pruneobjects") == 0){
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
			fd = open(argv[1], O_RDONLY);
			get(fd, type);
			break;
		case SET_DIM:
			fd = open(argv[1], O_RDWR);
			setDim(fd, type, atoi(argv[3]));
			break;
		case SET_OBJECT:
			printf("Action SET_OBJECT\n");
			fd = open(argv[1], O_RDWR);
			setObjects(fd, argc-3, argv+3);
			break;
		case PRUNE:
			printf("Action PRUNE\n");
			fd = open(argv[1], O_RDWR);
			break;
		default:
			break;
	}
	close(fd);
	return EXIT_SUCCESS;
}
