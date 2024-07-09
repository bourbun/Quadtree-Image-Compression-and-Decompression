/* VASILICĂ Andrei 314-CC */

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int max(int a, int b) {
  if (a > b) return a;
  return b;
}

int min(int a, int b) {
  if (a < b) return a;
  return b;
}

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

// Strcutura pentru culori
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

qcell *initQCell(Node *elem) {
  qcell *p = (qcell *)malloc(sizeof(qcell));
  p->elem = elem;
  p->next = NULL;
  return p;
}

queue *initQ() {
  queue *q = (queue *)malloc(sizeof(queue));
  q->front = q->tail = NULL;
  return q;
}

// verificam daca coada este goala
int emptyQ(queue *q) {
  if (q->front == NULL) return 1;
  return 0;
  // 1 = empty
}

void popQ(queue *q) {
  qcell *u = q->front;
  q->front = u->next;

  // eliberam memoria alocata pentru qcell
  free(u);
}

void addQ(queue *q, Node *elem) {
  // cream un nou element de adaugat in coada
  qcell *nou = initQCell(elem);

  // verificam daca este primul element de adaugat in coada
  if (q->front == NULL) {
    q->front = nou;
    q->tail = nou;
  } else {
    q->tail->next = nou;
    q->tail = nou;
  }
}

// Functia citeste un fisier de tip ppm si returneaza datele obtinute
ImageRGB *read_image(char *image_file, char *image_factor) {
  ImageRGB *image;
  int max_color;

  FILE *f = fopen(image_file, "rb");
  if (!f) {
    printf("Unable to open file '%s'\n", image_file);
    return 0;
  }

  // Cream imaginea
  image = (ImageRGB *)malloc(sizeof(ImageRGB));
  if (!image) {
    printf("Unable to allocate memory\n");
    return 0;
  }

  // Salvam factorul de compresie, trecandu-l din forma de string in int
  image->factor = atoi(image_factor);

  // Citim primele 3 randuri ale fisierului ppm
  fscanf(f, "%s%lld %lld%d", image->tip, &image->width, &image->height,
         &max_color);
  fseek(f, 1, SEEK_CUR);

  // Salvam intr-o marice de pixeli imaginea
  image->data = (RGB **)malloc(image->height * sizeof(RGB *));

  // Citim, pe rand, fiecare pixel, mai exact cele 3 culori ale sale
  for (int i = 0; i < image->height; i++) {
    image->data[i] = (RGB *)malloc(3 * image->width * sizeof(RGB));

    for (int j = 0; j < image->width; j++)
      fscanf(f, "%c%c%c", &image->data[i][j].red, &image->data[i][j].green,
             &image->data[i][j].blue);
  }

  fclose(f);
  return image;
}

// Functia returneaza culorile medii si calculeaza valoarea mean ceruta
RGB mean_calc(ImageRGB *image, unsigned long long int *mean, int x, int y,
              int size) {
  unsigned long long int red = 0, green = 0, blue = 0;
  RGB ret_rgb;

  // Parcurgem blocul de pixeli al fiecarui patrat
  for (int i = x; i < x + size; i++) {
    for (int j = y; j < y + size; j++) {
      red += image->data[i][j].red;
      green += image->data[i][j].green;
      blue += image->data[i][j].blue;
    }
  }

  // Obtinem valoarea medie pentru fiecare culoare
  red = red / (size * size);
  green = green / (size * size);
  blue = blue / (size * size);

  ret_rgb.red = (unsigned char)red;
  ret_rgb.green = (unsigned char)green;
  ret_rgb.blue = (unsigned char)blue;

  // Parcurgem inca o data blocul de pixeli pentru a afla valoarea mean
  for (int i = x; i < x + size; i++) {
    for (int j = y; j < y + size; j++)
      *mean +=
          (red - image->data[i][j].red) * (red - image->data[i][j].red) +
          (green - image->data[i][j].green) *
              (green - image->data[i][j].green) +
          (blue - image->data[i][j].blue) * (blue - image->data[i][j].blue);
  }

  *mean = *mean / (3 * size * size);

  return ret_rgb;
}

// Functia parcurge recursiv matricea de pixeli, pana la o culoare uniforma
Node *quadtree(ImageRGB *image, int x, int y, int size, int *nr_nod,
               int *colors) {
  unsigned long long mean = 0;
  RGB mean_rgb;

  // Cream nodul pentru fiecare bloc de pixeli pana la culoarea uniforma
  Node *nod = malloc(sizeof(Node));
  nod->area = (uint32_t)(size * size);

  // Calculam valoarea mean-ului
  mean_rgb = mean_calc(image, &mean, x, y, size);

  // Adaugam culoriile medii pentru fiecare nod
  nod->red = mean_rgb.red;
  nod->green = mean_rgb.green;
  nod->blue = mean_rgb.blue;

  // Contorizam nr de noduri
  *nr_nod += 1;

  // Verificam daca blocul de pixeli creeaza o imagine cu o nuanta uniforma
  if (mean <= image->factor) {
    nod->top_left = NULL;
    nod->top_right = NULL;
    nod->bottom_right = NULL;
    nod->bottom_left = NULL;

    // Contorizam cate blocuri de pixeli au o culoare relativ uniforma
    (*colors)++;
  } else if (size > 1) {
    nod->top_left = quadtree(image, x, y, size / 2, nr_nod, colors);
    nod->top_right = quadtree(image, x, y + size / 2, size / 2, nr_nod, colors);
    nod->bottom_right =
        quadtree(image, x + size / 2, y + size / 2, size / 2, nr_nod, colors);
    nod->bottom_left =
        quadtree(image, x + size / 2, y, size / 2, nr_nod, colors);
  }
  return nod;
}

int hasChildren(Node *node) {
  if (node->top_left != NULL || node->top_right != NULL ||
      node->bottom_left != NULL || node->bottom_right != NULL)
    return 1;
  else
    return 0;
}

// Adaugam nodurile intr-un vector de compresie
void fill_qt(Node *nod, QuadtreeNode *tree) {
  tree->red = nod->red;
  tree->green = nod->green;
  tree->blue = nod->blue;
  tree->area = nod->area;

  // Verificam daca nodul este terminal sau are cei 4 fii
  if (nod->top_left != NULL) {
    tree->node_type = 0;
    tree->top_left = nod->top_left->poz;
    tree->top_right = nod->top_right->poz;
    tree->bottom_right = nod->bottom_right->poz;
    tree->bottom_left = nod->bottom_left->poz;
  } else {
    tree->node_type = 1;
    tree->top_left = -1;
    tree->top_right = -1;
    tree->bottom_right = -1;
    tree->bottom_left = -1;
  }
  free(nod);
}

// Parcurgem arborele pe latime, creand vectorul de noduri asa cum este dorit
QuadtreeNode *bfs(Node *nod, int nr_nod) {
  Node **arr = (Node **)malloc(nr_nod * sizeof(Node *));
  int k = 0, i = 1;
  Node *aux = nod;

  // Adaugam radacina arborelui in vector
  arr[k++] = (Node *)nod;

  // Adaugam pe rand nodurile pana cand nu mai ramane niciunul pe dinafara
  while (k < nr_nod) {
    // Verificam ca nodul sa nu fie frunza pentru a adauga fii sai
    if (aux->top_left != NULL) {
      arr[k] = aux->top_left;
      aux->top_left->poz = k++;

      arr[k] = aux->top_right;
      aux->top_right->poz = k++;

      arr[k] = aux->bottom_right;
      aux->bottom_right->poz = k++;

      arr[k] = aux->bottom_left;
      aux->bottom_left->poz = k++;
    }

    aux = arr[i++];
  }

  // Cream vectorul de compresie utilizand functia precedenta
  QuadtreeNode *tree = (QuadtreeNode *)malloc(nr_nod * sizeof(QuadtreeNode));

  for (i = 0; i < nr_nod; i++) {
    fill_qt(arr[i], &tree[i]);
  }

  // Eliberam memoria alocata vectorului de noduri
  free(arr);

  // Returnam vectorul de compresie
  return tree;
}

// Functia printeaza in fisier datele vectorului de compresie
void print_compress(QuadtreeNode *tree, int nr_nod, int colors,
                    char *compress_file) {
  FILE *fout = fopen(compress_file, "wb");

  if (!fout) {
    printf("Unable to open file '%s'\n", compress_file);
    return;
  }

  // Parcurgem vectorul de compresie, afisand simultan fiecare element
  int area = sqrt(tree[0].area);
  fwrite(&area, sizeof(int), 1, fout);
  for (int i = 0; i < nr_nod; i++) {
    if (tree[i].node_type == 1) {
      fwrite(&tree[i].node_type, sizeof(unsigned char), 1, fout);
      fwrite(&tree[i].red, sizeof(unsigned char), 1, fout);
      fwrite(&tree[i].green, sizeof(unsigned char), 1, fout);
      fwrite(&tree[i].blue, sizeof(unsigned char), 1, fout);
    } else {
      fwrite(&tree[i].node_type, sizeof(unsigned char), 1, fout);
    }
  }

  // Eliberam memoria alocata vectorului de compresie
  free(tree);

  fclose(fout);
}

void freeNode(Node *nod) {
  if (nod == NULL) return;
  if (nod->node_type == 0) {  // Only free children if the node is not a leaf
    freeNode(nod->top_left);
    freeNode(nod->top_right);
    freeNode(nod->bottom_right);
    freeNode(nod->bottom_left);
  }
  free(nod);
}

void decompress(Node **root, queue *q, FILE *f) {
  *root = (Node *)malloc(sizeof(Node));
  Node *nod;
  addQ(q, (*root));
  while (!emptyQ(q)) {
    nod = q->front->elem;

    fread(&nod->node_type, sizeof(unsigned char), 1, f);
    if (nod->node_type == 1) {
      fread(&nod->red, sizeof(unsigned char), 1, f);
      fread(&nod->green, sizeof(unsigned char), 1, f);
      fread(&nod->blue, sizeof(unsigned char), 1, f);
    } else {
      nod->top_left = (Node *)malloc(sizeof(Node));
      nod->top_right = (Node *)malloc(sizeof(Node));
      nod->bottom_right = (Node *)malloc(sizeof(Node));
      nod->bottom_left = (Node *)malloc(sizeof(Node));

      addQ(q, nod->top_left);
      addQ(q, nod->top_right);
      addQ(q, nod->bottom_right);
      addQ(q, nod->bottom_left);
    }
    popQ(q);
  }
  free(q);
}

int depth(Node *nod) {
  if (nod == NULL)
    return 0;
  else {
    int tl = depth(nod->top_left);
    int tr = depth(nod->top_right);
    int br = depth(nod->bottom_right);
    int bl = depth(nod->bottom_left);

    // Găsim maximul dintre adâncimile subarborelui
    return max(max(tl, tr), max(br, bl)) + 1;
  }
}

int find_max_uniform_area(QuadtreeNode *tree, int nr_nod) {
  int max_area = 0;

  for (int i = 0; i < nr_nod; i++) {
    if (tree[i].top_left == -1) {  // Nod terminal
      max_area = max(max_area, tree[i].area);
    }
  }

  int square_size = sqrt(max_area);
  return square_size;
}

void create_matrix(Node *nod, RGB ***colors, int x, int y, int size) {
  int i, j;

  if (nod->node_type == 1) {
    for (i = x; i < x + size; i++) {
      for (j = y; j < y + size; j++) {
        (*colors)[i][j].red = nod->red;
        (*colors)[i][j].green = nod->green;
        (*colors)[i][j].blue = nod->blue;
      }
    }
  } else {
    create_matrix(nod->top_left, colors, x, y, size / 2);
    create_matrix(nod->top_right, colors, x, y + size / 2, size / 2);
    create_matrix(nod->bottom_right, colors, x + size / 2, y + size / 2,
                  size / 2);
    create_matrix(nod->bottom_left, colors, x + size / 2, y, size / 2);
  }
}

// Functia rezolva cerinta 1 a temei
void task1(char *image_file, char *compress_file, char *image_factor) {
  ImageRGB *image;
  Node *nod = NULL;
  QuadtreeNode *tree;
  int nr_nod = 0, colors = 0;

  // Citim fisierul ppm si stocam datele
  image = read_image(image_file, image_factor);

  // Returnam valoarea radacinii in urma crearii arborelui
  nod = quadtree(image, 0, 0, image->width, &nr_nod, &colors);

  int num_levels = depth(nod);
  // Verificam daca procesul anterior a fost executat cu succes
  if (nod != NULL)
    // Cream vectorul de compresie
    tree = bfs(nod, nr_nod);

  int size = find_max_uniform_area(tree, nr_nod);

  FILE *fout = fopen(compress_file, "w");
  fprintf(fout, "%d\n%d\n%d\n", num_levels, colors, size);
  fclose(fout);

  // Eliberarea memoriei matricei de pixeli
  for (int i = 0; i < image->height; i++) free(image->data[i]);
  free(image->data);
  free(image);
  free(tree);
}

void task2(char *image_file, char *compress_file, char *image_factor) {
  ImageRGB *image;
  Node *nod = NULL;
  QuadtreeNode *tree;
  int nr_nod = 0, colors = 0;

  // Citim fisierul ppm si stocam datele
  image = read_image(image_file, image_factor);

  // Returnam valoarea radacinii in urma crearii arborelui
  nod = quadtree(image, 0, 0, image->width, &nr_nod, &colors);

  // Verificam daca procesul anterior a fost executat cu succes
  if (nod != NULL)
    // Cream vectorul de compresie
    tree = bfs(nod, nr_nod);

  // Afisam continutul vectorului de compresie
  print_compress(tree, nr_nod, colors, compress_file);

  // Eliberarea memoriei matricei de pixeli
  for (int i = 0; i < image->height; i++) free(image->data[i]);
  free(image->data);
  free(image);
}

void task3(char *compress_file, char *image_file) {
  FILE *f = fopen(compress_file, "rb");
  FILE *fout = fopen(image_file, "wb");

  if (!f) {
    printf("Unable to open file '%s'\n", compress_file);
    return;
  }

  if (!fout) {
    printf("Unable to open file '%s'\n", image_file);
    fclose(f);  // Close the file before returning
    return;
  }

  int width, height, max_color = 255;
  Node *root;
  queue *q = initQ();

  fread(&width, sizeof(int), 1, f);
  height = width;

  // Decompress the tree from the file
  decompress(&root, q, f);

  // Create the matrix for the image data
  RGB **colors = (RGB **)malloc(height * sizeof(RGB *));
  for (int i = 0; i < height; i++) {
    colors[i] = (RGB *)malloc(width * sizeof(RGB));
  }

  // Fill the matrix with colors based on the decompressed quadtree
  create_matrix(root, &colors, 0, 0, width);

  // Write the PPM file header
  fprintf(fout, "P6\n%d %d\n%d\n", width, height, max_color);

  // Write the pixel data to the PPM file
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      fwrite(&colors[i][j].red, sizeof(unsigned char), 1, fout);
      fwrite(&colors[i][j].green, sizeof(unsigned char), 1, fout);
      fwrite(&colors[i][j].blue, sizeof(unsigned char), 1, fout);
    }
    free(colors[i]);  // Free each row after writing it to the file
  }
  free(colors);  // Free the array of pointers

  // Free the decompressed quadtree
  freeNode(root);

  // Close the files
  fclose(f);
  fclose(fout);
}

int main(int argc, char *argv[]) {
  if (strcmp(argv[1], "-c1") == 0) {
    task1(argv[3], argv[4], argv[2]);
  } else if (strcmp(argv[1], "-c2") == 0) {
    task2(argv[3], argv[4], argv[2]);
  } else if (strcmp(argv[1], "-d") == 0) {
    task3(argv[2], argv[3]);
  }
  return 0;
}
