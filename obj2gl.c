#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CUnit/Basic.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite1(void)
{
   if (NULL == (temp_file = fopen("triangle.obj", "r"))) {
      return -1;
   }
   else {
      return 0;
   }
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
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

void drawObj(FILE* obj) {
    int normalsCapacity = 1;
    int normalsSize = 0;
    struct point* normals = realloc(0, normalsCapacity * sizeof(struct point));

    void pushNormal(struct point normal) {
        if (normalsSize >= normalsCapacity) {
            normalsCapacity = normalsCapacity * 3 / 2 + 1;
            printf("realloc %d\n", normalsCapacity);
            normals = realloc(normals, normalsCapacity * sizeof(struct point));
        }

        normals[normalsSize].x = normal.x;
        normals[normalsSize].y = normal.y;
        normals[normalsSize].z = normal.z;
        ++normalsSize;
    }

    int verticesCapacity = 1;
    int verticesSize = 0;
    struct point* vertices = realloc(0, verticesCapacity * sizeof(struct point));

    void pushVertex(struct point vertex) {
        if (verticesSize >= verticesCapacity) {
            verticesCapacity = verticesCapacity * 3 / 2 + 1;
            printf("realloc %d\n", verticesCapacity);
            vertices = realloc(vertices, verticesCapacity * sizeof(struct point));
        }

        vertices[verticesSize].x = vertex.x;
        vertices[verticesSize].y = vertex.y;
        vertices[verticesSize].z = vertex.z;
        ++verticesSize;
    }

    int n, v;

    void nv(void) {
        struct point normal = normals[n - 1];
        struct point vertex = vertices[v - 1];
        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }

    char *line = 0;
    size_t len;

    while (getline(&line, &len, obj) != -1) {
        float x, y, z;
        int chars;

        if (sscanf(line, "vn %f %f %f", &x, &y, &z) == 3) {
            pushNormal((struct point) {x, y, z});
        } else if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
            pushVertex((struct point) {x, y, z});
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

    free(normals);
    free(vertices);
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

void testObjRead(void)
{
    size_t len = 0;
    char *line;

    if (NULL != temp_file) {
        float x, y, z;

        rewind(temp_file);

        ssize_t nRead = getline(&line, &len, temp_file);

        CU_ASSERT(32 == nRead);
        CU_ASSERT(line[0] == '#');

        nRead = getline(&line, &len, temp_file);
        CU_ASSERT(line[0] == 'm');

        nRead = getline(&line, &len, temp_file);
        CU_ASSERT(line[0] == 'o');

        nRead = getline(&line, &len, temp_file);
        CU_ASSERT(line[0] == '#');

        nRead = getline(&line, &len, temp_file);
        CU_ASSERT(line[0] == 'v');

        sscanf(line, "v %f %f %f", &x, &y, &z);
        CU_ASSERT_DOUBLE_EQUAL(x, 0, 1e-6);
        CU_ASSERT_DOUBLE_EQUAL(y, 1.08866211, 1e-6);
        CU_ASSERT_DOUBLE_EQUAL(x, 0, 1e-6);

        nRead = getline(&line, &len, temp_file);
        CU_ASSERT(line[0] == 'v');

        sscanf(line, "v %f %f %f", &x, &y, &z);
        CU_ASSERT_DOUBLE_EQUAL(x, 0, 1e-6);
        CU_ASSERT_DOUBLE_EQUAL(y, -0.54433105, 1e-6);
        CU_ASSERT_DOUBLE_EQUAL(z, 1.1547, 1e-6);
    }
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if ((NULL == CU_add_test(pSuite, "drawObj", testDrawObj)) ||
       (NULL == CU_add_test(pSuite, "read", testObjRead)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
