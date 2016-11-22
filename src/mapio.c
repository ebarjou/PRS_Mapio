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
  int fd = open(filename, O_WRONLY || O_CREAT || O_TRUNC, 0666);
  int nb_obj = map_objects();
  write(fd, &nb_obj, sizeof(int));
  printf("%d\n", map_objects());
  // Trouver comment récupérer la liste des objets utilisés.
  fprintf (stderr, "Sorry: Map save is not yet implemented\n");
  close(fd);
}

void map_load (char *filename)
{
  // TODO
  exit_with_error ("Map load is not yet implemented\n");
}

#endif
