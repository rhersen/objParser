#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CUnit/Basic.h"

static FILE* temp_file = NULL;

int init_suite1(void)
{
   if (NULL == (temp_file = fopen("triangle.obj", "r"))) {
      return -1;
   }
   else {
      return 0;
   }
}

int clean_suite1(void)
{
   if (0 != fclose(temp_file)) {
      return -1;
   }
   else {
      temp_file = NULL;
      return 0;
   }
}

int GL_POLYGON = 1;

int verifyBegin = 0;
int verifyEnd = 0;

struct point {
    float x, y, z;
} verifyNormal = {-11, -11, -11};

struct point verifyVertex = {-11, -11, -11};

static void glBegin(int mode) {
    verifyBegin += mode == GL_POLYGON;
}

static void glEnd() {
    ++verifyEnd;
}

static void glNormal3f(float x, float y, float z) {
    verifyNormal.x = x;
    verifyNormal.y = y;
    verifyNormal.z = z;
}

static void glVertex3f(float x, float y, float z) {
    verifyVertex.x = x;
    verifyVertex.y = y;
    verifyVertex.z = z;
}

struct pointList {
    int capacity;
    int size;
    struct point* element;
};

struct pointList newPointList(void) {
    int capacity = 1;
    struct pointList r = {capacity, 0, malloc(capacity * sizeof(struct point))};
    return r;
}

deletePointList(struct pointList this) {
    free(this.element);
}

void pushPoint(struct pointList* this, struct point normal) {
    if (this->size >= this->capacity) {
        this->capacity = this->capacity * 3 / 2 + 1;
        this->element = realloc(this->element, this->capacity * sizeof(struct point));
    }

    this->element[this->size].x = normal.x;
    this->element[this->size].y = normal.y;
    this->element[this->size].z = normal.z;
    ++this->size;
}

void drawObj(FILE* obj) {
    struct pointList normals = newPointList();
    struct pointList vertices = newPointList();

    int n, v;

    void nv(void) {
        struct point normal = normals.element[n - 1];
        struct point vertex = vertices.element[v - 1];
        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }

    char *line = 0;
    size_t len;

    while (getline(&line, &len, obj) != -1) {
        float x, y, z;
        int chars;

        if (sscanf(line, "vn %f %f %f", &x, &y, &z) == 3) {
            pushPoint(&normals, (struct point) {x, y, z});
        } else if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
            pushPoint(&vertices, (struct point) {x, y, z});
        } else if (sscanf(line, "f %d//%d %n", &v, &n, &chars) == 2) {
            char* pos = line + chars;

            glBegin(GL_POLYGON);
            nv();

            while (sscanf(pos, "%d//%d %n", &v, &n, &chars) == 2) {
                nv();
                pos += chars;
            }

            glEnd();
        }
    }

    deletePointList(normals);
    deletePointList(vertices);
}

void testDrawObj(void) {
    drawObj(temp_file);
    CU_ASSERT_EQUAL(1, verifyBegin);

    CU_ASSERT_DOUBLE_EQUAL(0, verifyNormal.x, 1e-6);
    CU_ASSERT_DOUBLE_EQUAL(-0.33333333, verifyNormal.y, 1e-6);
    CU_ASSERT_DOUBLE_EQUAL(0.94280904, verifyNormal.z, 1e-6);

    CU_ASSERT_DOUBLE_EQUAL(0.0, verifyVertex.x, 1e-6);
    CU_ASSERT_DOUBLE_EQUAL(-0.54433105, verifyVertex.y, 1e-6);
    CU_ASSERT_DOUBLE_EQUAL(1.15470054, verifyVertex.z, 1e-6);

    CU_ASSERT_EQUAL(1, verifyEnd);
}

int main()
{
   CU_pSuite pSuite = NULL;

   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);

   if (NULL == pSuite) {CU_cleanup_registry();
      return CU_get_error();
   }

   if ((NULL == CU_add_test(pSuite, "drawObj", testDrawObj))) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
