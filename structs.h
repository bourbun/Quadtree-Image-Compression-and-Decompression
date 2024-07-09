#ifndef STRUCTS_H
#define STRUCTS_H

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structura destinata vectorului de compresie
typedef struct QuadtreeNode {
  unsigned char blue, green, red;
  uint32_t area;
  unsigned char node_type;
  int32_t top_left, top_right;
  int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode;

// Structura unui nod
typedef struct Node {
  unsigned char blue, green, red;
  uint32_t area;
  unsigned char node_type;
  int poz;
  struct Node *top_left, *top_right, *bottom_left, *bottom_right;
} Node;

// Structura pentru culori
typedef struct RGB {
  unsigned char red, green, blue;
} RGB;

// Structura unei imagini de tip ppm
typedef struct ImageRGB {
  char tip[3];
  unsigned long long width, height, factor;
  RGB **data;
} ImageRGB;

typedef struct qCell {
  Node *elem;
  struct qCell *next;
} qcell;

typedef struct Queue {
  qcell *front, *tail;
} queue;

#endif // STRUCTS_H
