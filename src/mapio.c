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
  for (int i = 0 ; i < nb_obj_diff ; i++) objs_diff[i] = -1; // Initialization
  int cpt = 0; // Number of object types found
  for (int y = 0 ; y < height ; y++) { // For each square in the map
    for (int x = 0 ; x < width ; x++) {
      int obj_temp = map_get(x, y);
      write(fd, &obj_temp, sizeof(int)); // Write its object in the file

      if(obj_temp != MAP_OBJECT_NONE) { // If its object isn't empty
        int new_obj = 0;
        for (int i = 0 ; i < cpt ; i++) { // And we don't know it yet
          if (obj_temp == objs_diff[i]) { // TODO Verification failing
            new_obj = 1;
            break;
          }
        }
        if (new_obj == 0) {
          objs_diff[cpt++] = obj_temp; // OVERWRITE WIDTH
        }
      }
    }
  }

  // Write the number of objects so it know how many have to be read
  write(fd, &cpt, sizeof(int));
  for(int i = 0 ; i < cpt ; i++) { // Then write the caracteristics of the objects
    int obj = objs_diff[i];
    char* map_name = map_get_name(obj);
    int value = sizeof(map_name);
    write(fd, &value, sizeof(int)); // The length of the filename
    write(fd, map_name, value); // Then, the filename

    unsigned value_uns = map_get_frames(obj);
    write(fd, &value_uns, sizeof(unsigned)); // The number of frames/sprites

    value = map_get_solidity(obj);
    write(fd, &value, sizeof(int)); // The solidity of the object (0,1 or 2)

    value = map_is_destructible(obj);
    write(fd, &value, sizeof(int)); // The destructability of the object (0 or 1)

    value = map_is_collectible(obj);
    write(fd, &value, sizeof(int)); // The collectibility of the object (0 or 1)

    value = map_is_generator(obj);
    write(fd, &value, sizeof(int)); // The generability of the object (0 or 1)
  }

  //fprintf (stderr, "Sorry: Map save is not yet implemented\n");
  close(fd);
}

void map_load (char *filename)
{
  int fd = open(filename, O_RDONLY);
  // TODO Read in order :
  // Width
  // Height
  // The content of each case with "for(y{for(x{})})""
  // The number of objects
  // The characteristics of each object (filename.length, filename, nb_frames, etc.)
  close(fd);
  exit_with_error ("Map load is not yet implemented\n");
}

#endif
