#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "map.h"
#include "error.h"

#ifdef PADAWAN

void map_new (unsigned width, unsigned height)
{
  map_allocate (width, height);

  for (int x = 0; x < width; x++)
    map_set (x, height - 1, 0); // Ground

  for (int y = 0; y < height - 1; y++) {
    map_set (0, y, 1); // Wall
    map_set (width - 1, y, 1); // Wall
  }

  map_object_begin (6);

  // Texture pour le sol
  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
  // Mur
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
  // Gazon
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
  // Marbre
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);
  // Flower
  map_object_add ("images/flower.png", 1, MAP_OBJECT_AIR);
  // Coin
  map_object_add ("images/coin.png", 20, MAP_OBJECT_AIR | MAP_OBJECT_COLLECTIBLE);

  map_object_end ();

}

void map_save (char *filename)
{
  // TODO : Verifier les valeurs de retours des read/write
  mkdir("./maps", 0777);
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  int error = 0;
  // Find and write the sizes of the map
  int width = map_width();
  int height = map_height();
  write(fd, &width, sizeof(int))<0?++error:error;
  write(fd, &height, sizeof(int))<0?++error:error;

  // Find and write the different objects used in the map
  int nb_obj_diff = map_objects();
  int objs_diff[nb_obj_diff]; // The object types "found already"
  // objs_old_to_new[objs_diff[i]] == i, Array used so we
  // don't have to search for the object every time when writing
  int objs_old_to_new[nb_obj_diff];
  for (int i = 0 ; i < nb_obj_diff ; i++) objs_diff[i] = -1; // Initialization
  
  // Look for all object_types in the map
  int cpt = 0; // Number of object types found
  for (int y = 0 ; y < height ; y++) { // For each square in the map
    for (int x = 0 ; x < width ; x++) {
      int obj_temp = map_get(x, y);

      if(obj_temp != MAP_OBJECT_NONE) { // If its object isn't empty
        int new_obj = 0;
        for (int i = 0 ; i < cpt ; i++) {
          if (obj_temp == objs_diff[i]) { // And we don't know it yet
            new_obj = 1;
            break;
          }
        }
        if (new_obj == 0) { // Then we write it in a list
          objs_old_to_new[obj_temp] = cpt;
          objs_diff[cpt++] = obj_temp;
        }
      }
    }
  }

  // Write the number of objects so it know how many have to be read
  write(fd, &cpt, sizeof(int))<0?++error:error;
  // Then write the characteristics of the objects
  for(int i = 0 ; i < cpt ; i++) {
    int obj = objs_diff[i];
    char* obj_filename = map_get_name(obj);
    int value = strlen(obj_filename);
    write(fd, &value, sizeof(int))<0?++error:error; // The length of the filename
    write(fd, obj_filename, value*sizeof(char))<0?++error:error; // Then, the filename

    unsigned value_uns = map_get_frames(obj);
    write(fd, &value_uns, sizeof(unsigned))<0?++error:error; // The number of frames/sprites

    value = map_get_solidity(obj);
    write(fd, &value, sizeof(int))<0?++error:error; // The solidity of the object (0|1|2)

    value = map_is_destructible(obj);
    write(fd, &value, sizeof(int))<0?++error:error; // The destructibility of the object

    value = map_is_collectible(obj);
    write(fd, &value, sizeof(int))<0?++error:error; // The collectibility of the object

    value = map_is_generator(obj);
    write(fd, &value, sizeof(int))<0?++error:error; // The generability of the object
  }
  
  // Now we write all the objects themselves
  for (int y = 0 ; y < height ; y++) { // For each square in the map
    for (int x = 0 ; x < width ; x++) {
      int obj_temp = map_get(x, y);

      if(obj_temp != MAP_OBJECT_NONE) { // If its object isn't empty
        // Write its object's new_ID in the file
        write(fd, objs_old_to_new+obj_temp, sizeof(int))<0?++error:error;
      }
	  else // If it IS empty
		// Write -1 as object in the file
		write(fd, &obj_temp, sizeof(int))<0?++error:error;
    }
  }

  close(fd);

  if(error>0){
    fprintf (stderr, "Error occured during save\n");
  }	
}

void map_load (char *filename)
{
  int fd = open(filename, O_RDONLY);
  int error = 0;
  
  // Read and remember the sizes of the map
  int width = map_width();
  int height = map_height();
  read(fd, &width, sizeof(int))<0?++error:error;
  read(fd, &height, sizeof(int))<0?++error:error;

  map_allocate (width, height);
  
  // Read the number of different objects and make the program know it
  int nb_obj;
  read(fd, &nb_obj, sizeof(int))<0?++error:error;
  map_object_begin (nb_obj);
  
  // Then read the characteristics of the objects and create it
  for(int i = 0 ; i < nb_obj ; i++) {
    int file_length;
    read(fd, &file_length, sizeof(int))<0?++error:error;
    char obj_filename[file_length+1];
    for (int j = 0 ; j < file_length ; j++)
      read(fd, &obj_filename[j], sizeof(char))<0?++error:error;
    obj_filename[file_length]='\0';
    //read(fd, obj_filename, file_length*sizeof(char));

    unsigned nb_sprites;
    read(fd, &nb_sprites, sizeof(unsigned))<0?++error:error; // The number of frames/sprites

    int solidity;
    read(fd, &solidity, sizeof(int))<0?++error:error; // The solidity of the object (0|1|2)

    int destructibility;
    read(fd, &destructibility, sizeof(int))<0?++error:error; // The destructibility of the object (0|1)
    destructibility *= MAP_OBJECT_DESTRUCTIBLE;

    int collectibility;
    read(fd, &collectibility, sizeof(int))<0?++error:error; // The collectibility of the object (0|1)
    collectibility *= MAP_OBJECT_COLLECTIBLE;

    int generability;
    read(fd, &generability, sizeof(int))<0?++error:error; // The generability of the object (0|1)
    generability *= MAP_OBJECT_GENERATOR;
	
    map_object_add (obj_filename, nb_sprites,
			solidity | destructibility | collectibility | generability);
  }
  
  // Then read the objects on the map
  for (int y = 0 ; y < height ; y++) { // For each square in the map
    for (int x = 0 ; x < width ; x++) {
      int obj_val;
	  read(fd, &obj_val, sizeof(int))<0?++error:error;
	  printf("%d.", obj_val==-1?0:1);
	  if (obj_val != MAP_OBJECT_NONE) // No need to insert an empty object
		map_set(x, y, obj_val);
    }
	printf("\n");
  }
  
  map_object_end ();
  close(fd);
  if(error>0){
    fprintf (stderr, "Error occured during load\n");
  }	
  //exit_with_error ("Map load is not yet implemented\n");
}

#endif
