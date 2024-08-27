#include <stdio.h>
#include <stdlib.h>
#include "include/fortune.h"
#include "include/queue.h"
#include "include/geometry.h"
#include "include/beachline.h"
#include "include/plot.h"


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
  
  // OPEN PIPES FOR PLOTTING AND VIDEO
  system("rm -f ./video.mp4");
  FILE *pipe_plot = popen_gnuplot("./plot.png");
  FILE *pipe_video = popen_gnuplot("|ffmpeg -loglevel error -f png_pipe -s:v 1920x1080 -i pipe: -pix_fmt yuv420p -c:v libx264 -crf 18 ./video.mp4");
  start_plot(pipe_plot);
  add_seeds(pipe_plot, seeds, N);

  // INITIALISE
  queue_T queue = initialise_queue(seeds, N);
  site_T *sites = initialise_sites(seeds, N);
  beachline_T bline = NULL;
  
  double prev_y = 1;
  while (queue) {
    event_T event = pop_event(&queue);
    printf("Current y: %f\n", event.p.y);
    //putchar('\n');
    //print_beachline(bline);
    //print_event(&event);
    //print_queue(queue);

    // PLOT VIDEO
    for (int ii=1; ii<10; ii++) {
      double aux_y = prev_y - (prev_y - event.p.y) / 10 * ii;
      start_plot(pipe_video);
      add_seeds(pipe_video, seeds, N);
      add_bline(pipe_video, bline, aux_y);
      add_yline(pipe_video, aux_y);
      end_plot(pipe_video);
    }
    prev_y = event.p.y;
    
    // PROCESS EVENT
    //plot_current_frame(bline, event.p.y, seeds, N);
    if (event.type == EVENT_SITE) {
      event_site(&queue, &bline, event, seeds, N);
    } else if (event.type == EVENT_VERTEX) {
      event_vertex(&queue, &bline, event, seeds, N);
      add_point(pipe_plot, event.circ_c, "pt 7 ps 3 lc 'red'");
      add_circle(pipe_plot, event.circ_c, event.circ_c.y - event.p.y);
      printf("VORONOI VERTEX: x=%f, y=%f\n", event.circ_c.x, event.circ_c.y);
    } else {
      printf("ERROR! Unknown event! Should never see this...\n");
      exit(1);
    }
    
 }

  end_plot(pipe_plot);
  pclose(pipe_plot);
  pclose(pipe_video);
  
  return sites;
}


