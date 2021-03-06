/****
 BpmDj v4.2-pl4: Free Dj Tools
 Copyright (C) 2001-2012 Werner Van Belle

 http://bpmdj.yellowcouch.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but without any warranty; without even the implied warranty of
 merchantability or fitness for a particular purpose.  See the
 GNU General Public License for more details.

 See the authors.txt for a full list of people involved.
****/
#ifndef __loaded__pca_cpp__
#define __loaded__pca_cpp__
using namespace std;
/*********************************************************************
 * Principal Components Analysis or the Karhunen-Loeve expansion is a
 * classical method for dimensionality reduction or exploratory data
 * analysis.  One reference among many is: F. Murtagh and A. Heck,
 * Multivariate Data Analysis, Kluwer Academic, Dordrecht, 1987.
 *
 * Author: F. Murtagh
 * Phone:  + 49 89 32006298 (work)
 *         + 49 89 965307 (home)
 * Earn/Bitnet:  fionn@dgaeso51,  fim@dgaipp1s,  murtagh@stsci
 * Span:         esomc1::fionn
 * Internet:     murtagh@scivax.stsci.edu
 * F. Murtagh, Munich, 6 June 1989
 *********************************************************************
 * WVB - modified to incorporate it into BpmDj
 * WVB - added DUMP_INFO variable to print out data
 * WVB - modified malloc to allocate
 * WVB - added progress bar support..
 * WVB - added better error handling...
 */

#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "bpmdj.h"
#include "pca.h"
#include "memory.h"
#include "common.h"

#define SIGN(a, b) ( (b) < 0 ? -fabs(a) : fabs(a) )
static void corcol(float4 **data, int n, int m, float4 **symmat);
static float4 *pca_vector(int n);
static void tred2(float4 **a, int n, float4 *d, float4 *e);
static void tqli(float4 d[], float4 e[], int n, float4 ** z, 
const char* &error_msg);
static void free_pca_vector(float4 *v, int n);

// WVB -- this function is a modified version of the original code
// !!! data will be modified !!!
void do_pca(int n, int m, float4** data, const char* &error_msg)
{
  int  i, j, k, k2;
  float4 **symmat, **symmat2, *evals, *interm;
  assert(!error_msg);
  
  symmat = matrix(m, m);  /* Allocation of correlation (etc.) matrix */
  
  /* Look at analysis option; branch in accordance with this. */
  // printf("Analysis of correlations chosen.\n");
  corcol(data, n, m, symmat);
  
  /*********************************************************************
    Eigen-reduction
  **********************************************************************/
  /* Allocate storage for dummy and new pca_vectors. */
  evals = pca_vector(m);     /* Storage alloc. for pca_vector of eigenvalues */
  interm = pca_vector(m);    /* Storage alloc. for 'intermediate' pca_vector */
  symmat2 = matrix(m, m);  /* Duplicate of correlation (etc.) matrix */
  for (i = 1; i <= m; i++) {
    for (j = 1; j <= m; j++) {
      symmat2[i][j] = symmat[i][j]; /* Needed below for col. projections */
    }
  }
  /* Triangular decomposition */
  tred2(symmat, m, evals, interm);  
  /* Reduction of sym. trid. matrix */
  tqli(evals, interm, m, symmat, error_msg);   
  if (error_msg) return;
  
  /* evals now contains the eigenvalues,
     columns of symmat now contain the associated eigenpca_vectors. */

#ifdef PRINT_PCA_RESULTS
  printf("\nEigenvalues:\n");
  for (j = m; j >= 1; j--) {
    printf("%18.5f\n", evals[j]); }
  printf("\n(Eigenvalues should be strictly positive; limited\n");
  printf("precision machine arithmetic may affect this.\n");
  printf("Eigenvalues are often expressed as cumulative\n");
  printf("percentages, representing the 'percentage variance\n");
  printf("explained' by the associated axis or principal component.)\n");
  
  printf("\nEigenpca_vectors:\n");
  printf("(First three; their definition in terms of original vbes.)\n");
  for (j = 1; j <= m; j++) {
    for (i = 1; i <= m; i++)  {
      printf("%12.4f", symmat[j][m-i+1]);  }
    printf("\n");  } 
#endif

  /* Form projections of row-points on first three prin. components. */
  /* Store in 'data', overwriting original data. */
  for (i = 1; i <= n; i++) 
    {
      for (j = 1; j <= m; j++) 
	interm[j] = data[i][j]; // data[i][j] will be overwritten 
      for (k = 1; k <= 3; k++) 
	{
	  data[i][k] = 0.0;
	  for (k2 = 1; k2 <= m; k2++) 
	    data[i][k] += interm[k2] * symmat[k2][m-k+1]; 
#ifdef PRINT_ROWS
	  if (k==1)
	    printf("%g\t",data[i][1]);
	  else if (k==2)
	    printf("%g\n",data[i][2]);
#endif	  
	}
    }

  
  /*
    printf("\nProjections of row-points on first 3 prin. comps.:\n");
    for (i = 1; i <= n; i++) {
    for (j = 1; j <= 3; j++)  {
    printf("%12.4f", data[i][j]);  }
    printf("\n");  }
  */

  free_matrix(symmat, m, m);
  free_matrix(symmat2, m, m);
  free_pca_vector(evals, m);
  free_pca_vector(interm, m);
  
}

#ifdef TEST_PCA
/**  Variance-covariance matrix: creation  *****************************/
/* Create m * m covariance matrix from given n * m data matrix. */
static void covcol(float4 **data, int n, int m, float4 **symmat)
{
  float4 *mean;
  int i, j, j1, j2;
  /* Allocate storage for mean pca_vector */
  mean = pca_vector(m);
  /* Determine mean of column pca_vectors of input data matrix */
  for (j = 1; j <= m; j++)
    {
      mean[j] = 0.0;
      for (i = 1; i <= n; i++)
	mean[j] += data[i][j];
      mean[j] /= (float4)n;
    }
  
  // printf("\nMeans of column pca_vectors:\n");
  // for (j = 1; j <= m; j++)  {
  //    printf("%7.1f",mean[j]);  }   printf("\n");
  
  /* Center the column pca_vectors. */
  
  for (i = 1; i <= n; i++)
    for (j = 1; j <= m; j++)
      data[i][j] -= mean[j];
  
  /* Calculate the m * m covariance matrix. */
  for (j1 = 1; j1 <= m; j1++)
    for (j2 = j1; j2 <= m; j2++)
      {
	symmat[j1][j2] = 0.0;
	for (i = 1; i <= n; i++)
	  symmat[j1][j2] += data[i][j1] * data[i][j2];
	symmat[j2][j1] = symmat[j1][j2];
      }
}

/* Create m * m sums-of-cross-products matrix from n * m data matrix. */
void scpcol(float4 **data, int n, int m, float4 **symmat)
{
  int i, j1, j2;
  /* Calculate the m * m sums-of-squares-and-cross-products matrix. */
  for (j1 = 1; j1 <= m; j1++)
    for (j2 = j1; j2 <= m; j2++)
      {
	symmat[j1][j2] = 0.0;
	for (i = 1; i <= n; i++)
	  symmat[j1][j2] += data[i][j1] * data[i][j2];
	symmat[j2][j1] = symmat[j1][j2];
      }
}

// WVB -- renamed the original main
int old_main(int argc, char *argv[])
{
  FILE *stream;
  int  n, m,  i, j, k, k2;
  float4 **data, **symmat, **symmat2, *evals, *interm;
  float4 in_value;
  char option;
  char * error_msg = NULL;
  
  /*********************************************************************
   Get from command line:
   input data file name, #rows, #cols, option.

   Open input file: fopen opens the file whose name is stored in the
   pointer argv[argc-1]; if unsuccessful, error message is printed to
   stderr.
  *********************************************************************/
  
  if (argc !=  5)
    {
      printf("Syntax help: PCA filename #rows #cols option\n\n");
      printf("(filename -- give full path name,\n");
      printf(" #rows                          \n");
      printf(" #cols    -- integer values,\n");                  
      printf(" option   -- R (recommended) for correlation analysis,\n");
      printf("             V for variance/covariance analysis\n");
      printf("             S for SSCP analysis.)\n");
      fflush(stdout);
      _exit(1);
    }
  
  n = atoi(argv[2]);              /* # rows */
  m = atoi(argv[3]);              /* # columns */
  strncpy(&option,argv[4],1);     /* Analysis option */
  
  printf("No. of rows: %d, no. of columns: %d.\n",n,m);
  printf("Input file: %s.\n",argv[1]);
  
  if ((stream = fopen(argv[1],"r")) == NULL)
    {
      fprintf(stderr, "Program %s : cannot open file %s\n",
	      argv[0], argv[1]);
      fprintf(stderr, "Exiting to system.");
      fflush(stderr);
      _exit(1);
      /* Note: in versions of DOS prior to 3.0, argv[0] contains the
         string "C". */
    }
  
  /* Now read in data. */
  
  data = matrix(n, m);  /* Storage allocation for input data */
  
  for (i = 1; i <= n; i++)
    {
      for (j = 1; j <= m; j++)
	{
	  fscanf(stream, "%f", &in_value);
	  data[i][j] = in_value;
	}
    }
  
  /* Check on (part of) input data.
     for (i = 1; i <= 18; i++) {for (j = 1; j <= 8; j++)  {
     printf("%7.1f", data[i][j]);  }  printf("\n");  }
  */
  
  symmat = matrix(m, m);  /* Allocation of correlation (etc.) matrix */
  
  /* Look at analysis option; branch in accordance with this. */
  
  switch(option)
    {
    case 'R':
    case 'r':
      printf("Analysis of correlations chosen.\n");
      corcol(data, n, m, symmat);
      
      /* Output correlation matrix.
	 for (i = 1; i <= m; i++) {
	 for (j = 1; j <= 8; j++)  {
	 printf("%7.4f", symmat[i][j]);  }
	 printf("\n");  }
      */
      break;
    case 'V':
    case 'v':
      printf("Analysis of variances-covariances chosen.\n");
      covcol(data, n, m, symmat);
      
      /* Output variance-covariance matrix.
	 for (i = 1; i <= m; i++) {
	 for (j = 1; j <= 8; j++)  {
	 printf("%7.1f", symmat[i][j]);  }
	 printf("\n");  }
      */
      break;
    case 'S':
    case 's':
      printf("Analysis of sums-of-squares-cross-products");
      printf(" matrix chosen.\n");
      scpcol(data, n, m, symmat);
      
      /* Output SSCP matrix.
	 for (i = 1; i <= m; i++) {
	 for (j = 1; j <= 8; j++)  {
	 printf("%7.1f", symmat[i][j]);  }
	 printf("\n");  }
      */
      break;
    default:
      printf("Option: %c\n",option);
      printf("For option, please type R, V, or S\n");
      printf("(upper or lower case).\n");
      printf("Exiting to system.\n");
      fflush(stdout);
      _exit(1);
      break;
    }
  
  /*********************************************************************
    Eigen-reduction
  **********************************************************************/
  
  /* Allocate storage for dummy and new pca_vectors. */
  evals = pca_vector(m);     /* Storage alloc. for pca_vector of eigenvalues */
  interm = pca_vector(m);    /* Storage alloc. for 'intermediate' pca_vector */
  symmat2 = matrix(m, m);  /* Duplicate of correlation (etc.) matrix */
  for (i = 1; i <= m; i++) {
    for (j = 1; j <= m; j++) {
      symmat2[i][j] = symmat[i][j]; /* Needed below for col. projections */
    }
  }
  /* Triangular decomposition */
  tred2(symmat, m, evals, interm);  
  /* Reduction of sym. trid. matrix */
  tqli(evals, interm, m, symmat, error_msg);   
  assert(!error_msg);
  /* evals now contains the eigenvalues,
     columns of symmat now contain the associated eigenpca_vectors. */
  
  printf("\nEigenvalues:\n");
  for (j = m; j >= 1; j--) {
    printf("%18.5f\n", evals[j]); }
  printf("\n(Eigenvalues should be strictly positive; limited\n");
  printf("precision machine arithmetic may affect this.\n");
  printf("Eigenvalues are often expressed as cumulative\n");
  printf("percentages, representing the 'percentage variance\n");
  printf("explained' by the associated axis or principal component.)\n");
  
  printf("\nEigenpca_vectors:\n");
  printf("(First three; their definition in terms of original vbes.)\n");
  for (j = 1; j <= m; j++) {
    for (i = 1; i <= 3; i++)  {
      printf("%12.4f", symmat[j][m-i+1]);  }
    printf("\n");  }
  
  /* Form projections of row-points on first three prin. components. */
  /* Store in 'data', overwriting original data. */
  for (i = 1; i <= n; i++) {
    for (j = 1; j <= m; j++) {
      interm[j] = data[i][j]; }   /* data[i][j] will be overwritten */
    for (k = 1; k <= 3; k++) {
      data[i][k] = 0.0;
      for (k2 = 1; k2 <= m; k2++) {
	data[i][k] += interm[k2] * symmat[k2][m-k+1]; }
    }
  }
  
  printf("\nProjections of row-points on first 3 prin. comps.:\n");
  for (i = 1; i <= n; i++) {
    for (j = 1; j <= 3; j++)  {
      printf("%12.4f", data[i][j]);  }
    printf("\n");  }
  
  /* Form projections of col.-points on first three prin. components. */
  /* Store in 'symmat2', overwriting what was stored in this. */
  for (j = 1; j <= m; j++) {
    for (k = 1; k <= m; k++) {
      interm[k] = symmat2[j][k]; }  /*symmat2[j][k] will be overwritten*/
    for (i = 1; i <= 3; i++) {
      symmat2[j][i] = 0.0;
      for (k2 = 1; k2 <= m; k2++) {
	symmat2[j][i] += interm[k2] * symmat[k2][m-i+1]; }
      if (evals[m-i+1] > 0.0005)   /* Guard against zero eigenvalue */
	symmat2[j][i] /= sqrt(evals[m-i+1]);   /* Rescale */
      else
	symmat2[j][i] = 0.0;    /* Standard kludge */
    }
  }
  
  printf("\nProjections of column-points on first 3 prin. comps.:\n");
  for (j = 1; j <= m; j++) {
    for (k = 1; k <= 3; k++)  {
      printf("%12.4f", symmat2[j][k]);  }
    printf("\n");  }
  
  free_matrix(data, n, m);
  free_matrix(symmat, m, m);
  free_matrix(symmat2, m, m);
  free_pca_vector(evals, m);
  free_pca_vector(interm, m);

  return 0; // WVB -- the main must return something to avoid a warning
}
#endif 

/**  Correlation matrix: creation  ***********************************/

/* Create m * m correlation matrix from given n * m data matrix. */
static void corcol(float4 **data, int n, int m, float4 **symmat)
{
  float4 eps = 0.005;
  float4 x, *mean, *stddev;
  int i, j, j1, j2;
  /* Allocate storage for mean and std. dev. pca_vectors */
  mean = pca_vector(m);
  stddev = pca_vector(m);
  /* Determine mean of column pca_vectors of input data matrix */
  for (j = 1; j <= m; j++)
    {
      mean[j] = 0.0;
      for (i = 1; i <= n; i++)
	mean[j] += data[i][j];
      mean[j] /= (float4)n;
    }
  // printf("\nMeans of column pca_vectors:\n");
  // for (j = 1; j <= m; j++)  {
  //  printf("%7.1f",mean[j]);  }   printf("\n");
  /* Determine standard deviations of column pca_vectors of data matrix. */
  for (j = 1; j <= m; j++)
    {
      stddev[j] = 0.0;
      for (i = 1; i <= n; i++)
	stddev[j] += (   ( data[i][j] - mean[j] ) *
			 ( data[i][j] - mean[j] )  );
      stddev[j] /= (float4)n;
      stddev[j] = sqrt(stddev[j]);
      /* The following in an inelegant but usual way to handle
	 near-zero std. dev. values, which below would cause a zero-
	 divide. */
      if (stddev[j] <= eps) stddev[j] = 1.0;
    }
  
  //printf("\nStandard deviations of columns:\n");
  // for (j = 1; j <= m; j++) { printf("%7.1f", stddev[j]); }
  // printf("\n");
  
  /* Center and reduce the column pca_vectors. */
  for (i = 1; i <= n; i++)
    for (j = 1; j <= m; j++)
      {
	data[i][j] -= mean[j];
	x = sqrt((float4)n);
	x *= stddev[j];
	data[i][j] /= x;
      }
  
  /* Calculate the m * m correlation matrix. */
  for (j1 = 1; j1 <= m-1; j1++)
    {
      symmat[j1][j1] = 1.0;
      for (j2 = j1+1; j2 <= m; j2++)
        {
	  symmat[j1][j2] = 0.0;
	  for (i = 1; i <= n; i++)
	    symmat[j1][j2] += ( data[i][j1] * data[i][j2]);
	  symmat[j2][j1] = symmat[j1][j2];
        }
    }
  symmat[m][m] = 1.0;
}

/**  Sums-of-squares-and-cross-products matrix: creation  **************/


/* Allocates a float4 pca_vector with range [1..n]. */
float4 *pca_vector(int n)
{
  float4 * v = bpmdj_allocate(n,float4);
  return v-1;
}

/* Allocate a float4 matrix with range [1..n][1..m]. */
float4 **matrix(int n,int m)
{
  int i;
  float4 **mat;
  /* Allocate pointers to rows. */
  mat = bpmdj_allocate(n, float4*);
  mat -= 1;
  /* Allocate rows and set pointers to them. */
  for (i = 1; i <= n; i++)
    {
      mat[i] = bpmdj_allocate(m,float4);
      mat[i] -= 1;
    }
  /* Return pointer to array of pointers to rows. */
  return mat;
}

/**  Deallocate pca_vector storage  *********************************/
/* Free a float4 pca_vector allocated by pca_vector(). */
static void free_pca_vector(float4 *v, int n)
{
  bpmdj_deallocate((char*) (v+1));
}

/**  Deallocate float4 matrix storage  ***************************/
/* Free a float4 matrix allocated by matrix(). */
void free_matrix(float4 ** mat, int n, int m)
{
  int i;
  for (i = n; i >= 1; i--)
    bpmdj_deallocate ((char*) (mat[i]+1));
  bpmdj_deallocate ((char*) (mat+1));
}

/**  Reduce a real, symmetric matrix to a symmetric, tridiag. matrix. */

/* Householder reduction of matrix a to tridiagonal form.
   Algorithm: Martin et al., Num. Math. 11, 181-195, 1968.
   Ref: Smith et al., Matrix Eigensystem Routines -- EISPACK Guide
   Springer-Verlag, 1976, pp. 489-494.
   W H Press et al., Numerical Recipes in C, Cambridge U P,
   1988, pp. 373-374.  */
static void tred2(float4 **a, int n, float4 *d, float4 *e)
{
  int l, k, j, i;
  float4 scale, hh, h, g, f;
  
  for (i = n; i >= 2; i--)
    {
      l = i - 1;
      h = scale = 0.0;
      if (l > 1)
	{
	  for (k = 1; k <= l; k++)
	    scale += fabs(a[i][k]);
	  if (scale == 0.0)
	    e[i] = a[i][l];
	  else
	    {
	      for (k = 1; k <= l; k++)
		{
		  a[i][k] /= scale;
		  h += a[i][k] * a[i][k];
		}
	      f = a[i][l];
	      g = f>0 ? -sqrt(h) : sqrt(h);
	      e[i] = scale * g;
	      h -= f * g;
	      a[i][l] = f - g;
	      f = 0.0;
	      for (j = 1; j <= l; j++)
		{
		  a[j][i] = a[i][j]/h;
		  g = 0.0;
		  for (k = 1; k <= j; k++)
		    g += a[j][k] * a[i][k];
		  for (k = j+1; k <= l; k++)
		    g += a[k][j] * a[i][k];
		  e[j] = g / h;
		  f += e[j] * a[i][j];
		}
	      hh = f / (h + h);
	      for (j = 1; j <= l; j++)
		{
		  f = a[i][j];
		  e[j] = g = e[j] - hh * f;
		  for (k = 1; k <= j; k++)
		    a[j][k] -= (f * e[k] + g * a[i][k]);
		}
	    }
	}
      else
        e[i] = a[i][l];
      d[i] = h;
    }
  d[1] = 0.0;
  e[1] = 0.0;
  for (i = 1; i <= n; i++)
    {
      l = i - 1;
      if (d[i])
	{
	  for (j = 1; j <= l; j++)
	    {
	      g = 0.0;
	      for (k = 1; k <= l; k++)
		g += a[i][k] * a[k][j];
	      for (k = 1; k <= l; k++)
		a[k][j] -= g * a[k][i];
	    }
	}
      d[i] = a[i][i];
      a[i][i] = 1.0;
      for (j = 1; j <= l; j++)
	a[j][i] = a[i][j] = 0.0;
    }
}

/**  Tridiagonal QL algorithm -- Implicit  **********************/
static void tqli(float4 d[], float4 e[], int n, float4 ** z, 
		 const char* &error_occured)
{
  int m, l, iter, i, k;
  float4 s, r, p, g, f, dd, c, b;
  
  for (i = 2; i <= n; i++)
    e[i-1] = e[i];
  e[n] = 0.0;
  for (l = 1; l <= n; l++)
    {
      iter = 0;
      do
	{
	  for (m = l; m <= n-1; m++)
	    {
	      dd = fabs(d[m]) + fabs(d[m+1]);
	      if (fabs(e[m]) + dd == dd) break;
	    }
          if (m != l)
	    {
	      if (iter++ == 30) 
		{
		  error_occured = "No convergence in TQLI";
		  return;
		}
	      g = (d[l+1] - d[l]) / (2.0 * e[l]);
	      r = sqrt((g * g) + 1.0);
	      g = d[m] - d[l] + e[l] / (g + SIGN(r, g));
	      s = c = 1.0;
	      p = 0.0;
	      for (i = m-1; i >= l; i--)
		{
		  f = s * e[i];
		  b = c * e[i];
		  if (fabs(f) >= fabs(g))
                    {
		      c = g / f;
		      r = sqrt((c * c) + 1.0);
		      e[i+1] = f * r;
		      c *= (s = 1.0/r);
                    }
		  else
                    {
		      s = f / g;
		      r = sqrt((s * s) + 1.0);
		      e[i+1] = g * r;
		      s *= (c = 1.0/r);
                    }
		  g = d[i+1] - p;
		  r = (d[i] - g) * s + 2.0 * c * b;
		  p = s * r;
		  d[i+1] = g + p;
		  g = c * r - b;
		  for (k = 1; k <= n; k++)
		    {
		      f = z[k][i+1];
		      z[k][i+1] = s * z[k][i] + c * f;
		      z[k][i] = c * z[k][i] - s * f;
		    }
		}
	      d[l] = d[l] - p;
	      e[l] = g;
	      e[m] = 0.0;
	    }
	}  while (m != l);
    }
}

/***************************************************************************
    3.0    3.0    3.0    3.0    3.0    3.0   35.0   45.0
   53.0   55.0   58.0  113.0  113.0   86.0   67.0   90.0
    3.5    3.5    4.0    4.0    4.5    4.5   46.0   59.0
   63.0   58.0   58.0  125.0  126.0  110.0   78.0   97.0
    4.0    4.0    4.5    4.5    5.0    5.0   48.0   60.0
   68.0   65.0   65.0  123.0  123.0  117.0   87.0  108.0
    5.0    5.0    5.0    5.5    5.5    5.5   46.0   63.0
   70.0   64.0   63.0  116.0  119.0  115.0   97.0  112.0
    6.0    6.0    6.0    6.0    6.5    6.5   51.0   69.0
   77.0   70.0   71.0  120.0  122.0  122.0   96.0  123.0
   11.0   11.0   11.0   11.0   11.0   11.0   64.0   75.0
   81.0   79.0   79.0  112.0  114.0  113.0   98.0  115.0
   20.0   20.0   20.0   20.0   20.0   20.0   76.0   86.0
   93.0   92.0   91.0  104.0  104.5  107.0   97.5  104.0
   30.0   30.0   30.0   30.0   30.1   30.2   84.0   96.0
   98.0   99.0   96.0  101.0  102.0   99.0   94.0   99.0
   30.0   33.4   36.8   40.0   43.0   45.6  100.0  106.0
  106.0  108.0  101.0   99.0   98.0   99.0   95.0   95.0
   42.0   44.0   46.0   48.0   50.0   51.0  109.0  111.0
  110.0  110.0  103.0   95.5   95.5   95.0   92.5   92.0
   60.0   61.7   63.5   65.5   67.3   69.2  122.0  124.0
  124.0  121.0  103.0   93.2   92.5   92.2   90.0   90.8
   70.0   70.1   70.2   70.3   70.4   70.5  137.0  132.0
  134.0  128.0  101.0   91.7   90.2   88.8   87.3   85.8
   78.0   77.6   77.2   76.8   76.4   76.0  167.0  159.0
  152.0  144.0  103.0   89.8   87.7   85.7   83.7   81.8
   98.9   97.8   96.7   95.5   94.3   93.2  183.0  172.0
  162.0  152.0  102.0   87.5   85.3   83.3   81.3   79.3
  160.0  157.0  155.0  152.0  149.0  147.0  186.0  175.0
  165.0  156.0  120.0   87.0   84.9   82.8   80.8   79.0
  272.0  266.0  260.0  254.0  248.0  242.0  192.0  182.0
  170.0  159.0  131.0   88.0   85.8   83.7   81.6   79.6
  382.0  372.0  362.0  352.0  343.0  333.0  205.0  192.0
  178.0  166.0  138.0   86.2   84.0   82.0   79.8   77.5
  770.0  740.0  710.0  680.0  650.0  618.0  226.0  207.0
  195.0  180.0  160.0   82.9   80.2   77.7   75.2   72.7
***************************************************************************/
#endif // __loaded__pca_cpp__
