#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

  map_object_begin (4);

  // Texture pour le sol
  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
  // Mur
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
  // Gazon
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
  // Marbre
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);

  map_object_end ();

}

void map_save (char *filename)
{
  // TODO
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);

  // Find and write the sizes of the map
  int width = map_width();
  int height = map_height();
  write(fd, &width, sizeof(int));
  write(fd, &height, sizeof(int));

  // Find and write the different objects used in the map
  int nb_obj_diff = map_objects();
  int objs_diff[nb_obj_diff]; // The object types "found already"
  // objs_old_to_new[objs_diff[i]] == i, Array used so we
  // don't have to search for the object every time when writing
  int objs_old_to_new[nb_obj_diff];
  for (int i = 0 ; i < nb_obj_diff ; i++) objs_diff[i] = -1; // Initialization
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
          objs_diff[cpt++] = obj_temp; // OVERWRITE WIDTH
        }
		// Write its object's new ID in the file
        write(fd, objs_old_to_new+obj_temp, sizeof(int));
      }
	  else
		write(fd, &obj_temp, sizeof(int)); // Write -1 as object in the file
    }
  }

  // Write the number of objects so it know how many have to be read
  write(fd, &cpt, sizeof(int));
  // Then write the characteristics of the objects
  for(int i = 0 ; i < cpt ; i++) {
    int obj = i;
    char* obj_filename = map_get_name(obj);
    int value = sizeof(obj_filename);
    write(fd, &value, sizeof(int)); // The length of the filename
    write(fd, obj_filename, value*sizeof(char)); // Then, the filename

    unsigned value_uns = map_get_frames(obj);
    write(fd, &value_uns, sizeof(unsigned)); // The number of frames/sprites

    value = map_get_solidity(obj);
    write(fd, &value, sizeof(int)); // The solidity of the object (0|1|2)

    value = map_is_destructible(obj);
    write(fd, &value, sizeof(int)); // The destructibility of the object

    value = map_is_collectible(obj);
    write(fd, &value, sizeof(int)); // The collectibility of the object

    value = map_is_generator(obj);
    write(fd, &value, sizeof(int)); // The generability of the object
  }

  //fprintf (stderr, "Sorry: Map save is not yet implemented\n");
  close(fd);
}

void map_load (char *filename)
{
  int fd = open(filename, O_RDONLY);
  
  // Read and remember the sizes of the map
  int width = map_width();
  int height = map_height();
  read(fd, &width, sizeof(int));
  read(fd, &height, sizeof(int));
  
  for (int y = 0 ; y < height ; y++) { // For each square in the map
    for (int x = 0 ; x < width ; x++) {
      int obj_val;
	  read(fd, &obj_val, sizeof(int));
	  //if (obj_val != MAP_OBJECT_NONE)
	  map_set(x, y, obj_val);
    }
  }
  // Read the number of different objects and make the program know it
  int nb_obj;
  read(fd, &nb_obj, sizeof(int));
  map_object_begin (nb_obj);
  
  // Then read the characteristics of the objects and create it
  for(int i = 0 ; i < nb_obj ; i++) {
    int file_length;
	read(fd, &file_length, sizeof(int));
	char obj_filename[file_length];
	read(fd, obj_filename, file_length*sizeof(char));

    unsigned nb_sprites;
    read(fd, &nb_sprited, sizeof(unsigned)); // The number of frames/sprites

    int solidity;
    read(fd, &solidity, sizeof(int)); // The solidity of the object (0|1|2)

    int destructibility;
    read(fd, &destructibility, sizeof(int)); // The destructibility of the object (0|1)

    int collectibility;
    read(fd, &collectibility, sizeof(int)); // The collectibility of the object (0|1)

    int generability;
    read(fd, &generability, sizeof(int)); // The generability of the object (0|1)
	
	map_object_add (obj_filename, nb_sprites,
			solidity | destructibility | collectibility | generability);
  }
  
  map_object_end ();
  close(fd);
  //exit_with_error ("Map load is not yet implemented\n");
}

#endif
