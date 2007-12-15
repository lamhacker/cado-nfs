/* 
 * Program: merge
 * Author : F. Morain
 * Purpose: merging relations
 * 
 * Algorithm: digest and interpolation from Cavallar.
 *
 */

// TODO: use unified compact lists...!
// TODO: reintroduce mat->weight

#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

#define WANT_ASSERT

#include "utils/utils.h"

#define DEBUG 0
#define TEX 0

#define USE_TAB 0

#if USE_TAB == 0
#define isRowNull(mat, i) ((mat)->data[(i)].val == NULL)
#define lengthRow(mat, i) (mat)->data[(i)].len
#define cell(mat, i, k) (mat)->data[(i)].val[(k)]
#else
#define isRowNull(mat, i) ((mat)->rows[(i)] == NULL)
#define lengthRow(mat, i) (mat)->rows[(i)][0]
#define cell(mat, i, k) (mat)->rows[(i)][(k)]
#endif

#define USE_USED 0
#if USE_USED >= 1
char *used;
#endif

#define MERGE_LEVEL_MAX 30

// 0 means dummy filtering using slow methods
// 1 means:
//  * fast with optimized-though-memory-consuming data structure;
//  * heavy columns are killed.
// 2 means:
//  * fast as above;
//  * heavy columns are present, but not in S[]; this yields more acurate
//    weights.
#define USE_MERGE_FAST 2

typedef struct {
  int len;    /* number of non-zero entries */
  int *val;   /* array of size len containing column indices of non-zero
                 entries */
} rel_t;

// doubly chained lists
typedef struct dclist{
    int j;
    struct dclist *prev, *next;
} *dclist;

typedef struct {
  int nrows;
  int ncols;
  int rem_nrows;     /* number of remaining rows */
  int rem_ncols;     /* number of remaining columns */
  rel_t *data;
  int **rows;
  int *wt; /* weight of prime j, <= 1 for a deleted prime */
  int weight;
  int cwmax;         /* bound on weight of j to enter the SWAR structure */
  int rwmax;         /* if a weight(row) > rwmax, kill that row */
  int delta;         /* bound for nrows-ncols */
  int mergelevelmax; /* says it */
  dclist *S, *A;
  int **R;
} sparse_mat_t;

static int 
cmp(const void *p, const void *q) {
    int x = *((int *)p);
    int y = *((int *)q);
    return (x <= y ? -1 : 1);
}

dclist
dclistCreate(int j)
{
    dclist dcl = (dclist)malloc(sizeof(struct dclist));

    dcl->j = j;
    dcl->prev = NULL;
    dcl->next = NULL;
    return dcl;
}

void
dclistPrint(FILE *file, dclist dcl)
{
    while(dcl != NULL){
	fprintf(file, " -> %d", dcl->j);
	dcl = dcl->next;
    }
}

int
dclistLength(dclist dcl)
{
    int l = 0;

    while(dcl != NULL){
	l++;
	dcl = dcl->next;
    }
    return l;
}

void
dclistTex(FILE *file, dclist dcl)
{
    fprintf(stderr, "\\begin{array}{r}\n");
    while(dcl != NULL){
	fprintf(file, "%d\\\\\n", dcl->j);
	dcl = dcl->next;
    }
    fprintf(stderr, "\\end{array}\n");
}

dclist
dclistInsert(dclist dcl, int j)
{
    dclist newdcl = dclistCreate(j);

    newdcl->next = dcl->next;
    newdcl->prev = dcl;
    if(newdcl->next != NULL)
	newdcl->next->prev = newdcl;
    dcl->next = newdcl;
    return newdcl;
}

// TODO: ncols could be a new renumbered ncols...
void
initSWAR(sparse_mat_t *mat, int cwmax, int rwmax, int mergelevelmax)
{
#if USE_MERGE_FAST
    // cwmax+2 to prevent bangs
    dclist *S = (dclist *)malloc((cwmax+2) * sizeof(dclist));
    dclist *A = (dclist *)malloc(mat->ncols * sizeof(dclist));
    int **R;
    int j;
#endif
#if USE_TAB == 0
    mat->data = (rel_t*) malloc (mat->nrows * sizeof (rel_t));
#else
    mat->rows = (int **)malloc(mat->nrows * sizeof (int *));
#endif
    mat->wt = (int*) malloc (mat->ncols * sizeof (int));
    memset(mat->wt, 0, mat->ncols * sizeof (int));
    mat->rwmax = rwmax;
    mat->cwmax = cwmax;
    mat->mergelevelmax = mergelevelmax;
#if USE_MERGE_FAST
    // S[0] has a meaning at least for temporary reasons
    for(j = 0; j <= cwmax+1; j++)
	S[j] = dclistCreate(-1);
    R = (int **)malloc(mat->ncols * sizeof(int *));
    mat->S = S;
    mat->A = A;
    mat->R = R;
#endif
}

void
fillSWAR(sparse_mat_t *mat)
{
    int j, *Rj;

    for(j = 0; j < mat->ncols; j++){
	if(mat->wt[j] <= mat->cwmax){
	    mat->A[j] = dclistInsert(mat->S[mat->wt[j]], j);
#  if DEBUG >= 1
	    fprintf(stderr, "Inserting %d in S[%d]:", j, mat->wt[j]);
	    dclistPrint(stderr, mat->S[mat->wt[j]]->next);
	    fprintf(stderr, "\n");
#  endif
	    Rj = (int *)malloc((mat->wt[j]+1) * sizeof(int));
	    Rj[0] = 0; // last index used
	    mat->R[j] = Rj;
	}
	else{
#if USE_MERGE_FAST <= 1
	    mat->wt[j] = -1;
#else
	    mat->wt[j] = -mat->wt[j]; // trick!!!
#endif
	    mat->A[j] = NULL; // TODO: renumber j's?????
	    mat->R[j] = NULL;
	}
    }
}

// TODO
void
closeSWAR(sparse_mat_t *mat)
{
}

void
printStats(sparse_mat_t *mat)
{
    int w;

    for(w = 0; w <= mat->cwmax; w++)
	fprintf(stderr, "I found %d primes of weight %d\n",
		dclistLength(mat->S[w]->next), w);
}

void
checkData(sparse_mat_t *mat)
{
    int j;

    fprintf(stderr, "Checking final data\n");
    for(j = 0; j < mat->ncols; j++)
	if(mat->wt[j] > 0)
	    fprintf(stderr, "Wj[%d] = %d\n", j, mat->wt[j]);
}

// dump for debugging reasons
void
printSWAR(sparse_mat_t *mat, int ncols)
{
    int j, k, w;

    fprintf(stderr, "===== S is\n");
    for(w = 0; w <= mat->cwmax; w++){
	fprintf(stderr, "  S[%d] ", w);
	dclistPrint(stderr, mat->S[w]->next);
	fprintf(stderr, "\n");
    }
    fprintf(stderr, "===== Wj is\n");
    for(j = 0; j < ncols; j++)
	fprintf(stderr, "  Wj[%d]=%d\n", j, mat->wt[j]);
    fprintf(stderr, "===== R is\n");
    for(j = 0; j < ncols; j++){
	if(mat->R[j] == NULL)
	    continue;
	fprintf(stderr, "  R[%d]=", j);
	for(k = 1; k <= mat->R[j][0]; k++)
	    fprintf(stderr, " %d", mat->R[j][k]);
	fprintf(stderr, "\n");
    }
}

void
matrix2tex(sparse_mat_t *mat)
{
    int i, j, k, *tab;

    tab = (int *)malloc(mat->ncols * sizeof(int));
    fprintf(stderr, "$$\\begin{array}{l|");
    for(j = 0; j < mat->ncols; j++)
	fprintf(stderr, "c");
    fprintf(stderr, "}\n");
    for(i = 0; i < mat->nrows; i++){
	memset(tab, 0, mat->ncols * sizeof(int));
	for(k = 0; k < lengthRow(mat, i); k++)
	    tab[cell(mat, i, k)] = 1;
	fprintf(stderr, "R_{%d}", i);
	for(j = 0; j < mat->ncols; j++)
	    fprintf(stderr, "& %d", tab[j]);
	fprintf(stderr, "\\\\\n");
    }
    fprintf(stderr, "\\hline\n");
    fprintf(stderr, "\\text{weight}");
    for(j = 0; j < mat->ncols; j++)
	fprintf(stderr, "& %d", mat->wt[j]);
    fprintf(stderr, "\\\\\n");
    fprintf(stderr, "\\end{array}$$\n");
    free(tab);
}

void
texSWAR(sparse_mat_t *mat)
{
    int j, k, w;

    fprintf(stderr, "\\end{verbatim}\n");
    matrix2tex(mat);
    fprintf(stderr, "$$\\begin{array}{|");
    for(w = 0; w <= mat->cwmax; w++)
	fprintf(stderr, "r|");
    fprintf(stderr, "|r|}\\hline\n");
    for(w = 0; w <= mat->cwmax+1; w++){
	fprintf(stderr, "S[%d]", w);
	if(w < mat->cwmax+1)
	    fprintf(stderr, "&");
    }
    fprintf(stderr, "\\\\\\hline\n");
    for(w = 0; w <= mat->cwmax+1; w++){
	dclistTex(stderr, mat->S[w]->next);
	if(w < mat->cwmax+1)
            fprintf(stderr, "&");
    }
    fprintf(stderr, "\\\\\\hline\n");
    fprintf(stderr, "\\end{array}$$\n");
    fprintf(stderr, "$$\\begin{array}{|c|");
    for(j = 0; j < mat->ncols; j++)
	fprintf(stderr, "r|");
    fprintf(stderr, "}\\hline\n");
    fprintf(stderr, "j");
    for(j = 0; j < mat->ncols; j++)
	fprintf(stderr, "& %d", j);
    fprintf(stderr, "\\\\\\hline\n");
    fprintf(stderr, "W[j]");
    for(j = 0; j < mat->ncols; j++)
	fprintf(stderr, "& %d", mat->wt[j]);
    fprintf(stderr, "\\\\\\hline\n\\end{array}$$\n");
    fprintf(stderr, "\\begin{verbatim}\n");
    for(j = 0; j < mat->ncols; j++){
	if(mat->R[j] == NULL)
	    continue;
	fprintf(stderr, "  R[%d]=", j);
	for(k = 1; k <= mat->R[j][0]; k++)
	    fprintf(stderr, " %d", mat->R[j][k]);
	fprintf(stderr, "\n");
    }
}

// terrific hack: everybody on the same line
// the first is to be destroyed in replay!!!
void
reportn(int *ind, int n)
{
    int i;

#if DEBUG >= 1
    fprintf(stderr, "Reporting for n=%d\n", n);
#endif
    for(i = 0; i < n; i++){
	printf("%d", ind[i]);
	if(i < n-1)
	    printf(" ");
    }
    printf("\n");
}

void
report1(int i)
{
    reportn(&i, 1);
}

void
report2(int i1, int i2)
{
    int tmp[2];
    tmp[0] = i1;
    tmp[1] = i2;
    reportn(tmp, 2);
}

void
inspectWeight(sparse_mat_t *mat, FILE *purgedfile)
{
    int i, j, ret, x, nc;

    memset(mat->wt, 0, mat->ncols * sizeof(int));
    for(i = 0; i < mat->nrows; i++){
	ret = fscanf(purgedfile, "%d", &j); // unused index to rels file
	ASSERT (ret == 1);
	ret = fscanf (purgedfile, "%d", &nc);
	ASSERT (ret == 1);
	for(j = 0; j < nc; j++){
	    ret = fscanf(purgedfile, "%d", &x);
	    ASSERT (ret == 1);
	    mat->wt[x]++;
	}
    }
}

#define BUF_LEN 100

/* Reads a matrix file, and puts in mat->wt[j], 0 <= j < ncols, the
   weight of column j (adapted from matsort.c).

   We skip columns that are too heavy.

   mat->wt is already filled and put to - values when needed. So don't touch.
*/
void
readmat(sparse_mat_t *mat, FILE *file)
{
    int ret;
    int i, j;
    int nc, x, buf[BUF_LEN], ibuf;

    ret = fscanf (file, "%d %d", &(mat->nrows), &(mat->ncols));
    ASSERT (ret == 2);
    
    fprintf(stderr, "Reading matrix of %d rows and %d columns: excess is %d\n",
	    mat->nrows, mat->ncols, mat->nrows - mat->ncols);
    mat->rem_nrows = mat->nrows;
    mat->weight = 0;

    for (i = 0; i < mat->nrows; i++){
	ret = fscanf(file, "%d", &j); // unused index to rels file
	ASSERT (ret == 1);
	ret = fscanf (file, "%d", &nc);
	ASSERT (ret == 1);
	if(nc == 0){
#if USE_TAB == 0
	    lengthRow(mat, i) = 0;
	    mat->data[i].val = NULL;
#else
	    mat->rows[i] = NULL;
#endif
	    mat->rem_nrows--;
	}
	else{
	    for(j = 0, ibuf = 0; j < nc; j++){
		ret = fscanf(file, "%d", &x);
#if DEBUG >= 1
		fprintf(stderr, "i = %d, j = %d, x = %d\n", i, j, x);
#endif
		ASSERT (ret == 1);
		ASSERT (0 <= x && x < mat->ncols);
#if USE_MERGE_FAST <= 1
		if(mat->wt[x] > 0)
		    buf[ibuf++] = x;
#else
		// always store x
		buf[ibuf++] = x;
		mat->weight++;
#endif
		if(mat->wt[x] > 0){
		    mat->R[x][0]++;
		    mat->R[x][mat->R[x][0]] = i;
		}
	    }
	    ASSERT(ibuf <= BUF_LEN);
#if USE_TAB == 0
	    lengthRow(mat, i) = ibuf;
	    mat->data[i].val = (int*) malloc (ibuf * sizeof (int));
	    memcpy(mat->data[i].val, buf, ibuf * sizeof(int));
	    // sort indices in val to ease row merges
	    qsort(mat->data[i].val, ibuf, sizeof(int), cmp);
#else
	    mat->rows[i] = (int*) malloc ((ibuf+1) * sizeof (int));
	    mat->rows[i][0] = ibuf;
	    memcpy(mat->rows[i]+1, buf, ibuf * sizeof(int));
	    // sort indices in val to ease row merges
            qsort(mat->rows[i]+1, ibuf, sizeof(int), cmp);
#endif
	}
    }
    // we need to keep informed of what really happens; this will be an upper
    // bound on the number of active columns, I guess
    mat->rem_ncols = mat->ncols;
    printStats(mat);
}

void
print_row(sparse_mat_t *mat, int i)
{
    int k;
    
#if USE_TAB == 0
    for(k = 0; k < lengthRow(mat, i); k++)
	fprintf(stderr, " %d", cell(mat, i, k));
#else
    fprintRow(stderr, mat->rows[i]);
#endif
}

void
destroyRow(sparse_mat_t *mat, int i)
{
#if USE_TAB == 0
    free(mat->data[i].val);
    mat->data[i].val = NULL;
    lengthRow(mat, i) = 0;
#else
    free(mat->rows[i]);
    mat->rows[i] = NULL;
#endif
}

void
removeWeightFromRow(sparse_mat_t *mat, int i)
{
    int k;

    mat->weight -= lengthRow(mat, i);
#if USE_TAB == 0
    for(k = 0; k < lengthRow(mat, i); k++)
	mat->wt[cell(mat, i, k)]--;
#else
    removeWeight(mat->rows, mat->wt, i);
#endif
}

void
addWeightFromRow(sparse_mat_t *mat, int i)
{
    int k;

    mat->weight += lengthRow(mat, i);
#if USE_TAB == 0
    for(k = 0; k < lengthRow(mat, i); k++)
	mat->wt[cell(mat, i, k)]++;
#else
    addWeight(mat->rows, mat->wt, i);
#endif
}

int
hasCol(sparse_mat_t *mat, int i, int j)
{
    int k;

#if USE_TAB == 0
    for(k = 0; k < lengthRow(mat, i); k++)
	if(cell(mat, i, k) == j)
	    return 1;
#else
    for(k = 1; k <= mat->rows[i][0]; k++)
	if(cell(mat, i, k) == j)
	    return 1;
#endif
    return 0;
}

// i1 += i2; weights are taken care of outside...
void
addRowsData(rel_t *data, int i1, int i2)
{
    int k1, k2, k, len, *tmp, *tmp2;

#if DEBUG >= 2
    fprintf(stderr, "row[%d] =", i1); print_row(mat, i1);
    fprintf(stderr, "\n");
    fprintf(stderr, "row[%d] =", i2); print_row(mat, i2);
    fprintf(stderr, "\n");
#endif
    // merge row[i1] and row[i2] in i1...
    len = data[i1].len + data[i2].len;
    tmp = (int *)malloc(len * sizeof(tmp));
    k = k1 = k2 = 0;

    // loop while everybody is here
    while((k1 < data[i1].len) && (k2 < data[i2].len)){
	if(data[i1].val[k1] < data[i2].val[k2])
	    tmp[k++] = data[i1].val[k1++];
	else if(data[i1].val[k1] > data[i2].val[k2])
            tmp[k++] = data[i2].val[k2++];
	else{
#if DEBUG >= 1
	    fprintf(stderr, "ADD[%d=(%ld,%lu), %d=(%ld,%lu)]: new w[%d]=%lu\n", 
		    i1, data[i1].a, data[i1].b,
		    i2, data[i2].a, data[i2].b,
		    data[i1].val[k1], 
		    wt[data[i1].val[k1]]);
#endif
	    k1++;
	    k2++;
	}
    }
    // finish with k1
    for( ; k1 < data[i1].len; k1++)
	tmp[k++] = data[i1].val[k1];
    // finish with k2
    for( ; k2 < data[i2].len; k2++)
	tmp[k++] = data[i2].val[k2];
    // destroy and copy back
    free(data[i1].val);
    tmp2 = (int *)malloc(k * sizeof(int));
    memcpy(tmp2, tmp, k * sizeof(int));
    data[i1].len = k;
    data[i1].val = tmp2;
    free(tmp);
#if DEBUG >= 2
    fprintf(stderr, "row[%d]+row[%d] =", i1, i2);
    print_row(mat, i1); fprintf(stderr, "\n");
#endif
}

// i1 += i2, mat->wt is updated at the same time.
void
addRows(sparse_mat_t *mat, int i1, int i2)
{
    // i1 is to disappear, replaced by a new one
    removeWeightFromRow(mat, i1);
    addRowsData(mat->data, i1, i2);
    // new row i1 has to contribute to the weight
    addWeightFromRow(mat, i1);
}

// what is the weight of the sum of Ra and Rb?
int
weightSum(sparse_mat_t *mat, int i1, int i2)
{
    int k1, k2, w = 0;

#if USE_TAB == 0
    k1 = k2 = 0;
    while((k1 < lengthRow(mat, i1)) && (k2 < lengthRow(mat, i2))){
#else
    k1 = k2 = 1;
    while((k1 <= lengthRow(mat, i1)) && (k2 <= lengthRow(mat, i2))){
#endif
	if(cell(mat, i1, k1) < cell(mat, i2, k2)){
	    k1++;
	    w++;
	}
	else if(cell(mat, i1, k1) > cell(mat, i2, k2)){
	    k2++;
	    w++;
	}
	else{
	    k1++; k2++;
	}
    }
    w += (k1 > lengthRow(mat, i1) ? 0 : lengthRow(mat, i1)-k1+1);
    w += (k2 > lengthRow(mat, i2) ? 0 : lengthRow(mat, i2)-k2+1);
    return w;
}

int
findAllRowsWithGivenj(int *ind, sparse_mat_t *mat, int j, int nb)
{
    int i, k, r = 0;

    // TODO: special hack for nb==2???
    for(i = 0; i < mat->nrows; i++){
	if(isRowNull(mat, i))
	    continue;
#if USE_TAB == 0
	for(k = 0; k < lengthRow(mat, i)-1; k++) // trick!
	    if(cell(mat, i, k) >= j)
		break;
	if(cell(mat, i, k) == j){
	    ind[r++] = i;
	    if(r == nb)
		return 1;
	}
#else
	for(k = 1; k <= mat->rows[i][0]-1; k++) // trick!
	    if(mat->rows[i][k] >= j)
		break;
	if(mat->rows[i][k] == j){
	    ind[r++] = i;
	    if(r == nb)
		return 1;
	}
#endif
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////
// Prim

void
popQueue(int *s, int *t, int **Q, int *fQ, int *lQ)
{
    *s = Q[*fQ][0];
    *t = Q[*fQ][1];
    *fQ += 1;
}

void
printQueue(int **Q, int fQ, int lQ)
{
    int i;

    for(i = fQ; i < lQ; i++)
	fprintf(stderr, "Q[%d] = [%d, %d, %d]\n", i,Q[i][0], Q[i][1], Q[i][2]);
}

void
addEdge(int **Q, int *fQ, int *lQ, int u, int v, int Auv)
{
    int i, j;

    for(i = *fQ; i < *lQ; i++)
	if(Auv < Q[i][2])
	    break;
    // shift everybody: Auv >= Q[i][2]
    for(j = *lQ; j > i; j--){
	Q[j][0] = Q[j-1][0];
	Q[j][1] = Q[j-1][1];
	Q[j][2] = Q[j-1][2];
    }
    Q[i][0] = u;
    Q[i][1] = v;
    Q[i][2] = Auv;
    *lQ += 1;
}

// Add all neighbors of u, which is already in T; all edges with v in T
// are not useful anymore.
void
addAllEdges(int **Q, int *fQ, int *lQ, int u, int *father, int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX], int m)
{
    int v;

    for(v = 0; v < m; v++)
	if((v != u) && (father[v] < 0))
	    addEdge(Q, fQ, lQ, u, v, A[u][v]);
}

// height[0] = 0, etc. Returns hmax.
int
minimalSpanningTreeWithPrim(int *father, int *height, int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX], int m)
{
    int **Q, m2 = m*m, u, fQ = 0, lQ = 0, s, t, i, nV, hmax = 0;

    // over-conservative
    Q = (int **)malloc(m2 * sizeof(int *));
    for(i = 0; i < m2; i++)
	Q[i] = (int *)malloc(3 * sizeof(int));
    // nodes of T
    for(i = 0; i < m; i++){
	father[i] = -1;
	height[i] = 0;
    }
    u = 0;
    father[u] = u; // for the root, isn't it?
    nV = m-1;
    addAllEdges(Q, &fQ, &lQ, u, father, A, m);
#if DEBUG >= 1
    printQueue(Q, fQ, lQ);
#endif
    // active part of Q is Q[fQ..lQ[
    ASSERT(fQ == 0);
    ASSERT(lQ == (m-1));
    while(fQ != lQ){
	// while queue is non empty
	// pop queue
	popQueue(&s, &t, Q, &fQ, &lQ);
#if DEBUG >= 1
	fprintf(stderr, "Popping a = (%d, %d)\n", s, t);
#endif
	if(father[t] == -1){
	    // t does not close a cycle
	    // T[u] <- T[u] union (s, t)
#if DEBUG >= 1
	    fprintf(stderr, "new edge: (%d, %d)\n", s, t);
#endif
	    father[t] = s;
	    height[t] = height[s]+1;
	    if(height[t] > hmax)
		hmax = height[t];
	    nV--;
	    if(nV == 0)
		break;
	    addAllEdges(Q, &fQ, &lQ, t, father, A, m);
#if DEBUG >= 1
	    printQueue(Q, fQ, lQ);
#endif
	}
    }
    for(i = 0; i < m2; i++)
	free(Q[i]);
    free(Q);
    return hmax;
}

void
minimalSpanningTreeWithKruskal(int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX], int m)
{
#if 0
    int *E, m2 = m*(m-1), i, j, iE = 0, lE = 3*m2;
    int nV = 0, *ancester, *father;

    E = (int *)malloc(lE * sizeof(int));
    for(i = 0; i < m; i++)
	for(j = 0; j < m; j++){
	    if(j != i){
		E[iE++] = A[i][j];
		E[iE++] = i;
		E[iE++] = j;
	    }
	}
    // sort edges
    qsort(E, m2, 3 * sizeof(int), cmp);
#if DEBUG >= 1
    for(i = 0; i < m2; i++)
	fprintf(stderr, "w(%d, %d)=%d\n", E[3*i+1], E[3*i+2], E[3*i]);
#endif
    ancester = (int *)malloc(m * sizeof(int));
    father = (int *)malloc(m * sizeof(int));
    for(i = 0; i < m; i++){
	ancester[i] = -1;
	father[i] = -1; // everybody is a root
    }
    nV = 0;
    iE = -3;
    while(1){
	iE += 3;
	if((ancester[E[iE+1]] == -1) && (ancester[E[iE+2]] == -1)){
	    // new cc
	    ancester[E[iE+1]] = E[iE+1];
	    ancester[E[iE+2]] = father[E[iE+2]] = E[iE+1];
	    nV += 2;
	}
	else if(ancester[E[iE+1]] == -1){
	    // new edge to cc of E[iE+2]
	    father[E[iE+1]] = E[iE+2];
	    ancester[E[iE+1]] = ancester[E[iE+2]];
	    nV++;
	}
	else if(ancester[E[iE+2]] == -1){
            // new edge to cc of E[iE+1]
            father[E[iE+2]] = E[iE+1];
            ancester[E[iE+2]] = ancester[E[iE+1]];
	    nV++;
        }
	else{
	    // E[iE+1] and E[iE+2] are already known, check ancesters
	    int anc1 = findAncester(E[iE+1], ancester);
	    int anc2 = findAncester(E[iE+2], ancester);
	    // if anc1 == anc2, then this would create a cycle
	    if(anc1 != anc2){
		// we have to merge the two cc's
		ancester[E[iE+2]] = ancester[E[iE+1]];
		// but how do we deal with the father's...?
	    }
	}
	if(nV == m)
	    break;
    }
    free(E);
    free(ancester);
    free(father);
#endif
}

int
minimalSpanningTree(int *father, int *height, sparse_mat_t *mat, int m, int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX])
{
    return minimalSpanningTreeWithPrim(father, height, A, m);
    //    minimalSpanningTreeWithKruskal(A, m);
}

void
fillRowAddMatrix(int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX], sparse_mat_t *mat, int m, int *ind)
{
    int i, j;

    for(i = 0; i < m; i++)
	A[i][i] = 0;
    // A[i][j] <- Weight(R[ind[i]]+R[ind[j]]);
    for(i = 0; i < m; i++)
	for(j = i+1; j < m; j++){
	    A[i][j] = weightSum(mat, ind[i], ind[j]);
	    A[j][i] = A[i][j];
#if DEBUG >= 1
	    fprintf(stderr, "A[%d][%d] = A[%d][%d] = %d\n",i,j,j,i,A[i][j]);
#endif
	}
}

// not mallocing to speed up(?).
int
findBestIndex(sparse_mat_t *mat, int m, int *ind)
{
    int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX], i, j, imin, wmin, w;

    ASSERT(m <= MERGE_LEVEL_MAX);
    if(m == 2)
	return 0;
    fillRowAddMatrix(A, mat, m, ind);
    // iterate over all vertices
    imin = -1;
    wmin = 0;
    for(i = 0; i < m; i++){
	// compute the new total weight if i is used as pivot
	w = 0;
	for(j = 0; j < m; j++)
	    // note A[i][i] = 0
	    w += A[i][j];
#if DEBUG >= 1
	fprintf(stderr, "W[%d]=%d\n", i, w);
#endif
	if((imin == -1) || (w < wmin)){
	    imin = i;
	    wmin = w;
	}
    }
    return imin;
}

void
merge_m_slow(sparse_mat_t *mat, int m)
{
    int totfindrows = 0, totfindbest = 0, totadd = 0, tt;
    int *ind, j, k, i, njproc = 0;

#if DEBUG >= 1
    fprintf(stderr, "Weight %d:", m);
#endif
    ind = (int *)malloc(m * sizeof(int));
    for(j = 0; j < mat->ncols; j++){
	if(mat->wt[j] != m)
	    continue;
	njproc++;
	if(!(njproc % 1000))
	    fprintf(stderr, "# %d columns of weight %d processed\n",njproc,m);
	// we need to find the m rows and then
	tt = cputime();
	if(!findAllRowsWithGivenj(ind, mat, j, m))
	    fprintf(stderr, "Could not find the %d required rows\n", m);
	totfindrows += (cputime()-tt);
#if DEBUG >= 1
	fprintf(stderr, " %d", j);
	fprintf(stderr, "=> the %d rows are:\n", j, m);
	for(k = 0; k < m; k++){
	    fprintf(stderr, "row[%d]=", ind[k]);
	    print_row(mat, ind[k]);
	    fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
#endif
	// try all combinations to find the smaller one
	tt = cputime();
	i = findBestIndex(mat, m, ind);
	totfindbest += (cputime()-tt);
	if(i == -1){
	    fprintf(stderr, "Sorry, could not find best index\n");
	    // or do some more clever stuff?
	    continue;
	}
#if DEBUG >= 1
	fprintf(stderr, "Minimal is i=%d\n", i);
#endif
	tt = cputime();
	for(k = 0; k < m; k++)
	    if(k != i){
		addRows(mat, ind[k], ind[i]);
#if DEBUG >= 1
		fprintf(stderr, "new row[%d]=", ind[k]);
		print_row(mat, ind[k]);
		fprintf(stderr, "\n");
#endif
	    }
	if(i > 0){
	    // put ind[i] in front
	    int itmp = ind[i];
            for(k = i; k > 0; k--)
                ind[k] = ind[k-1];
	    ind[0] = itmp;
	}
	reportn(ind, m);
	totadd += (cputime()-tt);
	removeWeightFromRow(mat, ind[0]);
	destroyRow(mat, ind[0]);
	mat->rem_nrows--;
	mat->rem_ncols--;
	mat->wt[j] = 0;
    }
    fprintf(stderr, "TIME: findrows=%d findbest=%d add=%d\n",
	    totfindrows, totfindbest, totadd);
    free(ind);
}

// We should have mat->wt[j] == m == nchk
void
checkCoherence(sparse_mat_t *mat, int m, int j)
{
    int nchk = 0, k;

    for(k = 1; k <= mat->R[j][0]; k++)
	if(mat->R[j][k] != -1)
	    nchk++;
    ASSERT(nchk == (mat->wt[j] >= 0 ? mat->wt[j] : -mat->wt[j]));
    if(m != -1){
	if(nchk != m){
	    fprintf(stderr, "HYPERCHECK:");
	    fprintf(stderr, "mat->R[%d][0]=%d, m=%d\n", j, mat->R[j][0], m);
	    fprintf(stderr, "Gasp: nchk=%d\n", nchk);
	}
	ASSERT(nchk == m);
    }
}

// w(j) has decreased in such a way that it can be incorporated in the
// SWAR structure. We need find all the info we need to do that. Note this
// can be costly. We must not store row index i0, since j was discovered
// when row i0 was treated.
void
incorporateColumn(sparse_mat_t *mat, int j, int i0)
{
    int i, ni, *Rj, wj;

    wj = -mat->wt[j];
    Rj = (int *)malloc((wj+1) * sizeof(int));
    // find all rows in which j appears
    for(i = 0, ni = 1; i < mat->nrows; i++)
	if(!isRowNull(mat, i))
	    if((i != i0) && hasCol(mat, i, j))
#if 1
		Rj[ni++] = i;
#else
                ni++;
    fprintf(stderr, "iC: %d %d\n", j, ni);
#endif
    Rj[0] = ni-1;
    mat->R[j] = Rj;
    mat->wt[j] = wj;
    ASSERT(wj == Rj[0]);
    mat->A[j] = dclistInsert(mat->S[wj], j);
}

int
getNextj(dclist dcl)
{
    int j;
    dclist foo;

    foo = dcl->next;
    j = foo->j;
    dcl->next = foo->next;
    if(foo->next != NULL)
	foo->next->prev = dcl;
    free(foo);
    return j;
}

void
remove_j_from_S(sparse_mat_t *mat, int j)
{
    dclist dcl = mat->A[j], foo;

    if(dcl == NULL){
	fprintf(stderr, "Already removed? %d\n", j);
	return;
    }
#if DEBUG >= 2
    int ind = mat->wt[j];
    if(ind > mat->cwmax)
        ind = mat->cwmax+1;
    fprintf(stderr, "S[%d]_b=", ind);
    dclistPrint(stderr, mat->S[ind]->next); fprintf(stderr, "\n");
    fprintf(stderr, "dcl="); dclistPrint(stderr, dcl); fprintf(stderr, "\n");
#endif
    foo = dcl->prev;
    foo->next = dcl->next;
    if(dcl->next != NULL)
	dcl->next->prev = foo;
#if DEBUG >= 2
    fprintf(stderr, "S[%d]_a=", ind);
    dclistPrint(stderr, mat->S[ind]->next); fprintf(stderr, "\n");
#endif
    free(dcl);
}

void
destroyRj(sparse_mat_t *mat, int j)
{
    free(mat->R[j]);
    mat->R[j] = NULL;
}

void
remove_j_from_SWAR(sparse_mat_t *mat, int j)
{
    remove_j_from_S(mat, j);
    mat->A[j] = NULL;
    mat->wt[j] = 0; // minus 1
    destroyRj(mat, j);
}

// Don't touch to R[j][0]!!!!
void
remove_i_from_Rj(sparse_mat_t *mat, int i, int j)
{
    // be dumb for a while
    int k;

    if(mat->R[j] == NULL){
	fprintf(stderr, "Row %d already empty\n", j);
	return;
    }
    for(k = 1; k <= mat->R[j][0]; k++)
	if(mat->R[j][k] == i){
#if DEBUG >= 1
	    fprintf(stderr, "Removing row %d from R[%d]\n", i, j);
#endif
	    mat->R[j][k] = -1;
	    break;
	}
}

void
add_i_to_Rj(sparse_mat_t *mat, int i, int j)
{
    // be semi-dumb for a while
    int k;
    
#if DEBUG >= 1
    fprintf(stderr, "Adding row %d to R[%d]\n", i, j);
#endif
    for(k = 1; k <= mat->R[j][0]; k++)
	if(mat->R[j][k] == -1)
	    break;
    if(k <= mat->R[j][0]){
	// we have found a place where it is -1
	mat->R[j][k] = i;
    }
    else{
#if DEBUG >= 1
	fprintf(stderr, "WARNING: reallocing things in add_i_to_Rj for R[%d]\n", j);
#endif
	int l = mat->R[j][0]+2;
	mat->R[j] = (int *)realloc(mat->R[j], l * sizeof(int));
	mat->R[j][l-1] = i;
	mat->R[j][0] = l-1;
    }
}

int
decrS(int w)
{
#if USE_MERGE_FAST <= 1
    return w-1;
#else
    return (w >= 0 ? w-1 : w+1);
#endif
}

int
incrS(int w)
{
#if USE_MERGE_FAST <= 1
    return w+1;
#else
    return (w >= 0 ? w+1 : w-1);
#endif
}

// A[j] contains the address where j is stored
void
removeCellSWAR(sparse_mat_t *mat, int i, int j)
{
    int ind;

    // update weight
#if DEBUG >= 1
    fprintf(stderr, "removeCellSWAR: moving j=%d from S[%d] to S[%d]\n",
	    j, mat->wt[j], decrS(mat->wt[j]));
#endif
    ind = mat->wt[j] = decrS(mat->wt[j]);
#if USE_MERGE_FAST > 1
    if(ind < 0){
	// what if abs(weight) becomes below cwmax?
	// we should incorporate the column and update the data structure, no?
	if(abs(ind) <= mat->mergelevelmax){
#if DEBUG >= 1
	    fprintf(stderr, "WARNING: column %d becomes light at %d...!\n",
		    j, abs(ind));
#endif
	    incorporateColumn(mat, j, i);
	}
	return;
    }
#endif
    remove_j_from_S(mat, j);
    if(mat->wt[j] > mat->cwmax)
	ind = mat->cwmax+1;
    // update A[j]
#if DEBUG >= 2
    fprintf(stderr, "S[%d]_b=", ind);
    dclistPrint(stderr, mat->S[ind]->next); fprintf(stderr, "\n");
#endif
    mat->A[j] = dclistInsert(mat->S[ind], j);
#if DEBUG >= 2
    fprintf(stderr, "S[%d]_a=", ind);
    dclistPrint(stderr, mat->S[ind]->next); fprintf(stderr, "\n");
#endif
    // update R[j] by removing i
    remove_i_from_Rj(mat, i, j);
}

// for all j in row[i], removes j and update data
void
removeRowSWAR(sparse_mat_t *mat, int i)
{
    int k;

#if USE_TAB == 0
    mat->weight -= lengthRow(mat, i);
    for(k = 0; k < lengthRow(mat, i); k++)
	removeCellSWAR(mat, i, cell(mat, i, k));
#else
    mat->weight -= mat->rows[i][0];
    for(k = 1; k <= mat->rows[i][0]; k++)
	removeCellSWAR(mat, i, mat->rows[i][k]);
#endif
}

void
addCellSWAR(sparse_mat_t *mat, int i, int j)
{
    int ind;

    // update weight
#if DEBUG >= 1
    fprintf(stderr, "addCellSWAR: moving j=%d from S[%d] to S[%d]\n",
	    j, mat->wt[j], incrS(mat->wt[j]));
#endif
    ind = mat->wt[j] = incrS(mat->wt[j]);
#if USE_MERGE_FAST > 1
    if(ind < 0)
	return;
#endif
    remove_j_from_S(mat, j);
    if(mat->wt[j] > mat->cwmax){
#if DEBUG >= 1
	fprintf(stderr, "WARNING: column %d is too heavy (%d)\n", j,
		mat->wt[j]);
#endif
	ind = mat->cwmax+1; // trick
    }
    // update A[j]
    mat->A[j] = dclistInsert(mat->S[ind], j);
    // update R[j] by adding i
    add_i_to_Rj(mat, i, j);
}

void
addRowSWAR(sparse_mat_t *mat, int i)
{
    int k;

#if USE_TAB == 0
    mat->weight += lengthRow(mat, i);
    for(k = 0; k < lengthRow(mat, i); k++)
	addCellSWAR(mat, i, cell(mat, i, k));
#else
    mat->weight += mat->rows[i][0];
    for(k = 1; k <= mat->rows[i][0]; k++)
	addCellSWAR(mat, i, mat->rows[i][k]);
#endif
}

// i1 += i2.
void
addRowsSWAR(sparse_mat_t *mat, int i1, int i2)
{
#if 0 // original version
    int k1, k2, k, len, *tmp, *tmp2;

#if TEX
    fprintf(stderr, "row[%d] += row[%d]\n", i1, i2);
#endif
#if DEBUG >= 1
    fprintf(stderr, "row[%d] =", i1); print_row(mat, i1);
    fprintf(stderr, "\n");
    fprintf(stderr, "row[%d] =", i2); print_row(mat, i2);
    fprintf(stderr, "\n");
#endif
    // merge row[i1] and row[i2] in i1...
    len = mat->data[i1].len + mat->data[i2].len;
    tmp = (int *)malloc(len * sizeof(tmp));
    k = k1 = k2 = 0;

    // i1 is to disappear, replaced by a new one
    removeRowSWAR(mat, i1);
    // loop while everybody is here
    while((k1 < mat->data[i1].len) && (k2 < mat->data[i2].len)){
	if(mat->data[i1].val[k1] < mat->data[i2].val[k2]){
	    tmp[k++] = mat->data[i1].val[k1++];
	    mat->weight++;
	}
	else if(mat->data[i1].val[k1] > mat->data[i2].val[k2]){
            tmp[k++] = mat->data[i2].val[k2++];
	    mat->weight++;
        }
	else{
	    k1++;
	    k2++;
	}
    }
    // finish with k1
    for( ; k1 < mat->data[i1].len; k1++){
	tmp[k++] = mat->data[i1].val[k1];
	mat->weight++;
    }
    // finish with k2
    for( ; k2 < mat->data[i2].len; k2++){
	tmp[k++] = mat->data[i2].val[k2];
	mat->weight++;
    }
    // destroy and copy back
    free(mat->data[i1].val);
    tmp2 = (int *)malloc(k * sizeof(int));
    memcpy(tmp2, tmp, k * sizeof(int));
    mat->data[i1].len = k;
    mat->data[i1].val = tmp2;
    free(tmp);
    addRowSWAR(mat, i1);
#if DEBUG >= 1
    fprintf(stderr, "row[%d]+row[%d] =", i1, i2);
    print_row(mat, i1); fprintf(stderr, "\n");
#endif
#else // cleaner one, that shares addRowsData() to prepare the next move...!
    // i1 is to disappear, replaced by a new one
    removeRowSWAR(mat, i1);
    addRowsData(mat->data, i1, i2);
    addRowSWAR(mat, i1);
#endif
}

void
remove_j_from_row(sparse_mat_t *mat, int i, int j)
{
    int k;

#if DEBUG >= 2
    fprintf(stderr, "row[%d]_b=", i);
    print_row(mat, i);
    fprintf(stderr, "\n");
#endif
#if USE_TAB == 0
    for(k = 0; k < lengthRow(mat, i); k++)
	if(cell(mat, i, k) == j)
	    break;
    ASSERT(k < lengthRow(mat, i));
    // crunch
    for(++k; k < lengthRow(mat, i); k++)
	cell(mat, i, k-1) = cell(mat, i, k);
#else
    for(k = 1; k <= lengthRow(mat, i); k++)
	if(cell(mat, i, k) == j)
	    break;
    ASSERT(k <= lenghtRow(mat, i));
    // crunch
    for(++k; k <= lengthRow(mat, i); k++)
	cell(mat, i, k-1) = cell(mat, i, k);
#endif
    lengthRow(mat, i) -= 1;
    mat->weight--;
#if DEBUG >= 2
    fprintf(stderr, "row[%d]_a=", i);
    print_row(mat, i);
    fprintf(stderr, "\n");
#endif
}

// These columns are simply removed from the current squeleton matrix, but
// not from the small matrix.
int
deleteAllColsFromStack(sparse_mat_t *mat, int iS)
{
    dclist dcl;
    int j, k, njrem = 0;

    while(1){
	dcl = mat->S[iS]->next;
	if(dcl == NULL)
	    break;
	j = dcl->j;
	njrem++;
#if DEBUG >= 1
	fprintf(stderr, "Removing column %d from S[%d]\n", j, iS);
#endif
	// we destroy column j
	// TODO: do faster?
#if USE_MERGE_FAST > 1
	if(iS == 1)
#endif
	for(k = 1; k <= mat->R[j][0]; k++)
	    if(mat->R[j][k] != -1)
		remove_j_from_row(mat, mat->R[j][k], j);
#if USE_MERGE_FAST > 1
	k = mat->wt[j]; // make a copy of the weight
#endif
	remove_j_from_SWAR(mat, j);
	// mat->wt[j] is put to 0...
#if USE_MERGE_FAST > 1
	mat->wt[j] = -k; // restore and update
#endif
    }
#if DEBUG >= 1
    if(njrem > 0)
	fprintf(stderr, "deleteAllColsFromStack[%d]: %d\n", iS, njrem);
#endif
    return njrem;
}

int
deleteHeavyColumns(sparse_mat_t *mat)
{
    return deleteAllColsFromStack(mat, mat->cwmax+1);
}

int
removeSingletons(sparse_mat_t *mat)
{
#if USE_MERGE_FAST == 0
    int i, j;

    for(j = 0; j < mat->ncols; j++)
	if(mat->wt[j] == 1){
	    // find row...
	    for(i = 0; i < mat->nrows; i++)
		if(hasCol(mat, i, j))
		    break;
	    ASSERT(i < mat->nrows);
	    removeWeightFromRow(mat, i);
	    destroyRow(mat, i);
	    report1(i); // signal to replay...!
	}
#else
    return deleteAllColsFromStack(mat, 1);
#endif
}

// try all combinations to find the smaller one
void
tryAllCombinations(sparse_mat_t *mat, int m, int *ind)
{
    int i, k;
    
    i = findBestIndex(mat, m, ind);
#if DEBUG >= 1
    fprintf(stderr, "Minimal is i=%d\n", i);
#endif
    for(k = 0; k < m; k++)
	if(k != i){
	    addRowsSWAR(mat, ind[k], ind[i]);
#if DEBUG >= 1
	    fprintf(stderr, "new row[%d]=", ind[k]);
	    print_row(mat, ind[k]);
	    fprintf(stderr, "\n");
#endif
	}
    if(i > 0){
	// put ind[i] in front
	int itmp = ind[i];
	for(k = i; k > 0; k--)
	    ind[k] = ind[k-1];
	ind[0] = itmp;
    }
    reportn(ind, m);
    removeRowSWAR(mat, ind[0]);
    destroyRow(mat, ind[0]);
}

void
useMinimalSpanningTree(sparse_mat_t *mat, int m, int *ind, int *tfill, int *tMST)
{
    int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX];
    int i, father[MERGE_LEVEL_MAX], height[MERGE_LEVEL_MAX], hmax, nV = m, h;
    int adds[MERGE_LEVEL_MAX << 1], nadds, tab[MERGE_LEVEL_MAX << 1], itab;

    *tfill = cputime();
    fillRowAddMatrix(A, mat, m, ind);
    *tfill = cputime()-*tfill;
    *tMST = cputime();
    hmax = minimalSpanningTree(father, height, mat, m, A);
    *tMST = cputime()-*tMST;
#if DEBUG >= 1
    for(i = 0; i < m; i++)
	fprintf(stderr, "father[%d] = %d\n", i, father[i]);
#endif
    // add each father to each of his sons, but from bottom to top...!
    for(h = hmax; h > 0; h--){
	// we are currently at height h
	if(nV == 1)
	    // only the root remains!
	    break;
	nadds = 0;
	for(i = 1; i < m; i++)
	    if(height[i] == h){
		int i1 = ind[i], i2 = ind[father[i]];
#if DEBUG >= 1
		fprintf(stderr, "h = %d, i = %d: ", h, i);
		fprintf(stderr, "row[%d] += row[%d]\n", i1, i2);
#endif
		nV--;

		addRowsSWAR(mat, i1, i2);

		// TODO: use the fact we know w(i1, i2) = A[][] to compute
		// the length of row[i1]+row[i2] directly

		// since we want to add without destroying, we report
		// -(i2+1) i1 // hack!!!!
#if 0 // for non compact reporting + comment reportn for adds[]
		report2(-(i2+1), i1);
#endif
		adds[nadds++] = -(i2+1);
		adds[nadds++] = i1;
	    }
	qsort(adds, nadds>>1, 2 * sizeof(int), cmp);
#if DEBUG >= 1
	fprintf(stderr, "all adds[%d]:", h);
	for(i = 0; i < nadds; i++)
	    fprintf(stderr, " %d", adds[i]);
	fprintf(stderr, "\n");
#endif
	// compress history
	tab[0] = adds[0];
	tab[1] = adds[1];
	itab = 2;
	for(i = 2; i < nadds; i += 2){
	    if(adds[i] == tab[0])
		// same father
		tab[itab++] = adds[i+1];
	    else{
		// new father
		reportn(tab, itab);
		tab[0] = adds[i];
		tab[1] = adds[i+1];
		itab = 2;
	    }
	}
	if(h == 1)
	    // we are at the root, actually
	    tab[0] = -tab[0]-1; // what a trick!
	reportn(tab, itab);
    }
#if 0 // for non compact reporting
    // delete root!
    report1(ind[0]);
#endif
    removeRowSWAR(mat, ind[0]);
    destroyRow(mat, ind[0]);
}

void
findOptimalCombination(sparse_mat_t *mat, int m, int *ind, int *tfill, int *tMST)
{
    if(m <= 3){
	*tfill = *tMST = 0;
	tryAllCombinations(mat, m, ind);
    }
    else{
#if 0
	tryAllCombinations(mat, m, ind);
#else
	useMinimalSpanningTree(mat, m, ind, tfill, tMST);
#endif
    }
}

void
checkWeight(sparse_mat_t *mat, int j)
{
    int i, w = 0;

    fprintf(stderr, "Rows containing %d:", j);
    for(i = 0; i < mat->nrows; i++)
	if(!isRowNull(mat, i))
	    if(hasCol(mat, i, j)){
		fprintf(stderr, " %d", i);
		w++;
	    }
    fprintf(stderr, "\n");
    ASSERT(w == (mat->wt[j] >= 0 ? mat->wt[j] : -mat->wt[j]));
}

void
checkMatrixWeight(sparse_mat_t *mat)
{
    int i, w = 0;

    for(i = 0; i < mat->nrows; i++)
	if(!isRowNull(mat, i))
	    w += lengthRow(mat, i);
    ASSERT(w == mat->weight);
}

int
merge_m_fast(sparse_mat_t *mat, int m)
{
    int totopt = 0, tot = cputime(), tt, totfill = 0, totMST = 0, tfill, tMST;
    int *ind, j, k, njproc = 0, ni, njrem = 0;
    dclist dcl = mat->S[m];

#if DEBUG >= 1
    fprintf(stderr, "Weight %d:", m);
#endif
    ind = (int *)malloc(m * sizeof(int));

    //    checkWeight(mat, 132461);

    while(1){
#if DEBUG >= 1
	checkMatrixWeight(mat);
#endif
	// get next j
	dcl = mat->S[m]->next;
	if(dcl == NULL)
	    break;
	j = dcl->j;
	njproc++;
	if(!(njproc % 1000))
	    fprintf(stderr, "# %d columns of weight %d processed\n",njproc,m);
#if DEBUG >= 1
	fprintf(stderr, "Treating %d-th column %d of weight %d\n", njproc, j,
		mat->wt[j]);
#endif
#if (DEBUG >= 1) || TEX
	fprintf(stderr, "Status before next j=%d to start\n", j);
# if TEX
	texSWAR(mat);
# else
	printSWAR(mat, mat->ncols);
# endif
#endif
	// the corresponding rows are in R[j], skipping 1st cell and -1's
#if DEBUG >= 2
	checkCoherence(mat, m, j);
#endif
	ni = 0;
	for(k = 1; k <= mat->R[j][0]; k++){
	    if(mat->R[j][k] != -1){
		ind[ni++] = mat->R[j][k];
		if(ni == m)
		    break;
	    }
	}
#if DEBUG >= 1
	fprintf(stderr, " %d", j);
	fprintf(stderr, "=> the %d rows are:\n", j);
	for(k = 0; k < m; k++){
	    fprintf(stderr, "row[%d]=", ind[k]);
	    print_row(mat, ind[k]);
	    fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
#endif
	tt = cputime();
	findOptimalCombination(mat, m, ind, &tfill, &tMST);
	totopt += (cputime()-tt);
	totfill += tfill;
	totMST += tMST;
	mat->rem_nrows--;
	mat->rem_ncols--;
	remove_j_from_SWAR(mat, j);
	njrem += deleteHeavyColumns(mat);
    }
    tot = cputime()-tot;
    fprintf(stderr, "TIME: m=%d nj=%d findopt=%d (%d/%d) tot=%d\n", 
	    m, njproc, totopt, totfill, totMST, tot);
    free(ind);
#if DEBUG >= 1
    fprintf(stderr, "Status at the end of the process\n");
    texSWAR(mat);
#endif
    return njrem;
}

int
merge_m(sparse_mat_t *mat, int m)
{
#if USE_MERGE_FAST >= 1
    return merge_m_fast(mat, m);
#else
    return merge_m_slow(mat, m);
#endif
}

// TODO: use mergemax?
int
mergeGe2(sparse_mat_t *mat, int m, int nb_merge_max)
{
#if USE_MERGE_FAST == 0
    int j, nbm;

    // TODO: remove this and start directly merge_m?
    for(j = 0, nbm = 0; j < mat->ncols; j++)
	if(mat->wt[j] == m){
# if DEBUG >= 0
	    fprintf(stderr, "# wt[%d] = %d\n", j, m);
# endif
	    nbm++;
	}
    fprintf(stderr, "There are %d column(s) of weight %d\n", nbm, m);
    if(nbm)
#endif
	return merge_m(mat, m);
#if DEBUG >= 1
    matrix2tex(mat);
#endif
}

int
minColWeight(sparse_mat_t *mat)
{
#if USE_MERGE_FAST == 0
    int j, minw = mat->nrows;

    for(j = 0; j < mat->ncols; j++)
	if((mat->wt[j] > 0) && (mat->wt[j] < minw))
	    minw = mat->wt[j];
    return minw;
#else
    // in this case, locating minw is easy: inspect the stacks!
    int m;

    for(m = 1; m <= mat->cwmax; m++)
	if(mat->S[m]->next != NULL)
	    return m;
    return -1;
#endif
}

void
inspectRowWeight(sparse_mat_t *mat)
{
    int i, maxw = 0;

    for(i = 0; i < mat->nrows; i++)
	if(!isRowNull(mat, i)){
	    if(lengthRow(mat, i) > mat->rwmax){
		if((mat->rem_nrows - mat->rem_ncols) > mat->delta){
#if DEBUG >= 1
		    fprintf(stderr, "Too heavy row[%d]: %d\n", 
			    i, lengthRow(mat, i));
#endif
		    removeRowSWAR(mat, i);
		    destroyRow(mat, i);
		    report1(i);
		    mat->rem_nrows--;
		}
	    }
	    if(lengthRow(mat, i) > maxw)
		maxw = lengthRow(mat, i);
	}
    fprintf(stderr, "nrows=%d; max row weight is %d\n", mat->rem_nrows, maxw);
}

void
merge(sparse_mat_t *mat, int nb_merge_max, int maxlevel)
{
    int old_nrows, old_ncols, m, mm, njrem = 0;

    report2(mat->nrows, mat->ncols);
    m = 2;
    while(1){
	fprintf(stderr, "Performing merge %d; w(M)=%d, ncols*w(M)=%ld\n",
		m, mat->weight, ((long)mat->rem_ncols) * ((long)mat->weight));
	old_nrows = mat->rem_nrows;
	old_ncols = mat->rem_ncols;
	if(m == 1)
	    njrem += removeSingletons(mat);
	else
	    njrem += mergeGe2(mat, m, nb_merge_max);
#if 0
    {
	int i, ni = 0, j0 = 191342;

	for(i = 0; i < mat->nrows; i++)
	    if((mat->data[i].val != NULL) && hasCol(mat, i, j0))
		ni++;
	printf("CHECK: %d %d\n", ni, mat->wt[j0]);
    }
#endif
	fprintf(stderr, "=> nrows=%d ncols=%d njrem=%d\n",
		mat->rem_nrows, mat->rem_ncols, njrem);
	inspectRowWeight(mat);
	mm = minColWeight(mat);
	fprintf(stderr, "Min col weight = %d\n", mm);
#if 1
	if(mm < m)
	    // something new happened, anyway
	    m = mm;
	else
#endif
	    if((old_nrows == mat->rem_nrows) && (old_ncols == mat->rem_ncols)){
		// nothing happened this time and mm > m
		m = mm;
		if((m > maxlevel) || (m <= 0))
		    break;
	    }
    }
}

void
checkmat(sparse_mat_t *mat)
{
    int i;

    for(i = 0; i < mat->nrows; i++){
	fprintf(stderr, "Row %d =", i);
	print_row(mat, i);
	fprintf(stderr, "\n");
    }
    exit(-1);
}

#if 0 // not activated anymore...!
void
dumpSparse(FILE *ofile, sparse_mat_t *mat, int *code)
{
    int i, j, k, buf[1000], ibuf, new_nrows = 0;

    fprintf(ofile, "%d %d\n", mat->rem_nrows, mat->rem_ncols);
    for(i = 0; i < mat->nrows; i++){
	if(mat->data[i].val == NULL)
	    continue;
	new_nrows++;
	ibuf = 0;
	for(k = 0; k < lengthRow(mat, i); k++){
	    j = cell(mat, i, k);
	    if(code[j])
		buf[ibuf++] = code[j]-1;
	}
	fprintf(ofile, "%d", ibuf);
	for(k = 0; k < ibuf; k++)
	    fprintf(ofile, " %d", buf[k]);
	fprintf(ofile, "\n");
    }
    ASSERT(new_nrows == mat->rem_nrows);
}
#endif

int
main(int argc, char *argv[])
{
    FILE *purgedfile;
    sparse_mat_t mat;
    char *purgedname = NULL, *hisname = NULL;
    int nb_merge_max = 0, nrows, ncols;
    int cwmax = 20, rwmax = 1000000, maxlevel = 2;
    
#if TEX
    fprintf(stderr, "\\begin{verbatim}\n");
#endif    
    fprintf (stderr, "%s rev. %s\n", argv[0], REV);
    
    while(argc > 1 && argv[1][0] == '-'){
	if(argc > 2 && strcmp (argv[1], "-merge") == 0){
	    nb_merge_max = atoi(argv[2]);
	    argc -= 2;
	    argv += 2;
	}
	else if (argc > 2 && strcmp (argv[1], "-mat") == 0){
	    purgedname = argv[2];
	    argc -= 2;
	    argv += 2;
	}
	else if (argc > 2 && strcmp (argv[1], "-rebuild") == 0){
	    hisname = argv[2];
	    argc -= 2;
	    argv += 2;
	}
	else if (argc > 2 && strcmp (argv[1], "-cwmax") == 0){
	    cwmax = atoi(argv[2]);
	    argc -= 2;
	    argv += 2;
	}
	else if (argc > 2 && strcmp (argv[1], "-rwmax") == 0){
	    rwmax = atoi(argv[2]);
	    argc -= 2;
	    argv += 2;
	}
	else if (argc > 2 && strcmp (argv[1], "-maxlevel") == 0){
	    maxlevel = atoi(argv[2]);
	    argc -= 2;
	    argv += 2;
	}
	else 
	    break;
    }
    
    purgedfile = fopen(purgedname, "r");
    ASSERT(purgedfile != NULL);
    fscanf(purgedfile, "%d %d", &nrows, &ncols);

    mat.nrows = nrows;
    mat.ncols = ncols;
    mat.delta = 128;
    initSWAR(&mat, cwmax, rwmax, maxlevel);

    inspectWeight(&mat, purgedfile);
    
    fillSWAR(&mat);
    
    rewind(purgedfile);
    readmat(&mat, purgedfile);
    fclose(purgedfile);
#if DEBUG >= 3
    checkmat(&mat);
#endif
#if USE_USED >= 1
    used = (char *)malloc(mat.nrows * sizeof(char));
    memset(used, 0, mat.nrows * sizeof(char));
#endif
    merge(&mat, nb_merge_max, maxlevel);
    fprintf(stderr, "Final matrix has w(M)=%d, ncols*w(M)=%ld\n",
	    mat.weight, ((long)mat.rem_ncols) * ((long)mat.weight));
#if TEX
    fprintf(stderr, "\\end{verbatim}\n");
#endif
    //  checkData(&mat);
    closeSWAR(&mat);
    return 0;
}
