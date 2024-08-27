#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "include/fortune.h"
#include "include/queue.h"
#include "include/geometry.h"
#include "include/beachline.h"
#include "include/plot.h"

/* TODO
 * [ ] Plotting module. To see seeds and events for the moment (whole diagram in the future)
 *    [ ] This may serve to verify current code...
 * [ ] Work on DCEL to construct real voronoi diagram...
 * [ ] Make code more efficient: Use binary trees istead of linked lists for the beachline.
*/


/* FROM: CMSC 754: Lecture 11, Voronoi Diagrams and Fortune’s Algorithm, Dave Mount, Spring 2020
 *
 * Site event:  Sweep line passes over a new site.
 *                - The new site has a degenerate arc (vertical line). Assume
 *                  it does not fall immeadetly below an existing vertex.
 *                - The existing arch is then split into three:
 *                  < ... pi ... >    ->    < ... pi pj pi ... >
 *                - Note that these events are known in advance!
 *
 * Vertex event:  Generated by adjacent arcs, once one "disappears".
 *                  - Consider pi, pj, pk sucht that: 
 *                    < ... pj pi pj pk ... >  .
 *                    There is a circumcircle containint these 3 points.
 *                  - If the circumcircle does not contain any other site 
 *                    inside (bellow the sweep-line), no future point will 
 *                    block the creation of the vertex.
 *                  - The vertex is created when the sweep-liine reaches the
 *                    lowest point of the circumcircle.
 *                  - The bisectors (pi, pj) and (pj, pk) have met each other.
 *                    Now a new bisector (pi, pk) is created.
 *                    < .. pj pi pj pk .. >    ->    < .. pj pi pk .. >
*/


void remove_files_in_directory(const char *path)
{
  struct dirent *entry;
  DIR *dp = opendir(path);

  if (dp == NULL) {
    perror("opendir");
    return;
  }

  while ((entry = readdir(dp))) {
    // Skip the "." and ".." directories
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Construct the full path of the file to be deleted
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

    // Remove the file
    if (remove(filepath) != 0) {
      printf("Coultd not remove file...\n");
      exit(1);
    }
  }

  closedir(dp);
}


void event_site(queue_T *queue, beachline_T *bline, event_T event, point2D_T *seeds, int N)
{ 
  arc_T *arc_above = find_arc_above(*bline, event.p);
  remove_vertex_events_involving(queue, arc_above);
  arc_T *arc = insert_arc(bline, arc_above, event.p);
  //printf("%f\n", event.p.y);
  add_vertex_events_involving(queue, arc, event.p.y, seeds, N);
}


void event_vertex(queue_T *queue, beachline_T *bline, event_T event, point2D_T *seeds, int N)
{
  arc_T *left = event.arc->left; 
  arc_T *right = event.arc->right;
  remove_vertex_events_involving(queue, event.arc);
  delete_arc(bline, event.arc);
  if (left) add_vertex_events_involving(queue, left, event.p.y, seeds, N);
  if (right) add_vertex_events_involving(queue, right, event.p.y, seeds, N);
}


site_T *fortune_algorithm(point2D_T *seeds, int N)
{
remove_files_in_directory("./frames");

  FILE *pipe = popen_gnuplot("./plot.png");
  start_plot(pipe);
  add_seeds(pipe, seeds, N);

  queue_T queue = initialize_queue(seeds, N);

  site_T *sites = initialise_sites(seeds, N);
  beachline_T bline = NULL;
  
  double prev_y = 1;
  while (queue) {
    event_T event = pop_event(&queue);
    putchar('\n');
    print_beachline(bline);
    print_event(&event);
    print_queue(queue);

    // PRINT BEACHLINE EVOLUTION
    for (int ii=1; ii<10; ii++) {
      double aux_y = prev_y - (prev_y - event.p.y) / 10 * ii;
      plot_current_frame(bline, aux_y, seeds, N);
    }
    prev_y = event.p.y;

    plot_current_frame(bline, event.p.y, seeds, N);
    if (event.type == EVENT_SITE) {
      event_site(&queue, &bline, event, seeds, N);
    } else if (event.type == EVENT_VERTEX) {
      event_vertex(&queue, &bline, event, seeds, N);
      add_point(pipe, event.circ_c, "pt 7 ps 3 lc 'red'");
      printf("%f, %f\n", event.circ_c.y, event.p.y);
      add_circle(pipe, event.circ_c, event.circ_c.y - event.p.y);
      printf("x: %f, y: %f\n", event.circ_c.x, event.circ_c.y);
    } else {
      printf("ERROR! Unknown event! Should never see this...\n");
      exit(1);
    }
    
    printf("\n");
    //print_beachline(bline);
    //print_queue(queue);
    printf("\n NEXT \n");
 }

  //print_beachline(bline);
  //free_beachline(bline);

  end_plot(pipe);
  
  system("rm output.mp4");
  system("ffmpeg -r 24 -i ./frames/frame_%d.png -c:v libx264 -pix_fmt yuv420p output.mp4");
  return sites;
}


