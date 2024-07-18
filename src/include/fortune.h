#ifndef VORONOI_FORTUNE_H
#define VORONOI_FORTUNE_H

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
 *
*/

typedef struct point2D_T {
  double x;
  double y;
} point2D_T;

point2D_T *random_seeds(double size, int N);


// The beachline struct consists of the sorted sequence of foci whose arcs 
// form the current beachline. It is updated with the events.
// Linked list of arcs, where the arcs are simply the foci.

typedef struct arc_T {
  struct point2D_T focus;
  struct arc_T *left;
  struct arc_T *right;
} arc_T;

typedef arc_T *beachline_T;

void print_beachline(beachline_T bline);

arc_T *new_arc(point2D_T focus);

double parabola(point2D_T f, double directrix, double x);

typedef struct roots2_T {
  double pos;
  double neg;
} roots2_T;

roots2_T intersect_parabs(point2D_T f1, point2D_T f2, double directrix_y);

arc_T *find_arc_above(beachline_T bline, point2D_T focus);

arc_T *insert_arc(beachline_T *bline, point2D_T focus);

void delete_arc(arc_T *bline, const arc_T *arc);


typedef struct circle_T {
  double A;  // x-center
  double B;  // y-center
  double R;
} circle_T;

circle_T points2circle(point2D_T p1, point2D_T p2, point2D_T p3);

// Event queue:  linked list of events
enum event_type {
    EVENT_SITE,
    EVENT_VERTEX
};

typedef struct event_T {
  enum event_type type;
  struct event_T *next;
  //point2D_T seed;
  //double y;
  double x;  // Coordinates associated to the event
  double y;
} event_T;

event_T *new_event(enum event_type type, double x, double y);

event_T *initialize_queue(const point2D_T *seeds, int N);

event_T pop_event(event_T **queue);

void add_event(event_T **queue);

void print_queue(const event_T *queue);

void print_event(const event_T *event);

int circle_contains_seed_p(event_T *queue, circle_T circle);

void add_vertex_events(event_T **queue, const arc_T *arc);


// Voronoi diagram:
typedef struct vertex_T {
  double x, y;
  int ii;
  struct edge_T *inc_edge;
} vertex_T;

typedef struct edge_T {
  struct edge_T *twin;
  struct vertex_T *start;
  struct face_T *inc_face;  // Left face
  struct edge_T *next;
  struct edge_T *prev;
} edge_T;

typedef struct face_T {
  struct edge_T *edges;
  point2D_T *seed;
} face_T;

typedef struct vor_diagram_T {
  int N_faces;
  struct face_T *faces;
} vor_diagram_T;

vor_diagram_T *fortune_algorithm(point2D_T *seeds, int N);


#endif
