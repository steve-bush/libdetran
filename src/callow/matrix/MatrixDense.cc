//----------------------------------*-C++-*----------------------------------//
/**
 *  @file   MatrixDense.cc
 *  @brief  MatrixDense
 *  @author Jeremy Roberts
 *  @date   Jan 7, 2013
 */
//---------------------------------------------------------------------------//

#include "MatrixDense.hh"

namespace callow
{

//---------------------------------------------------------------------------//
MatrixDense::MatrixDense(const int m, const int n, const double v)
  : MatrixBase(m, n)
{
  // Preconditions
  Require(d_m > 0);
  Require(d_n > 0);

  // Allocate the values and initialize
  int N = m * n;
  d_values = new double[N];
  for (int i = 0; i < N; ++i) d_values[i] = v;

#ifdef CALLOW_ENABLE_PETSC
  PetscErrorCode ierr;
  ierr = MatCreateSeqDense(PETSC_COMM_SELF, d_n, d_m,
                           d_values, &d_petsc_matrix);
  Assert(!ierr);
#endif

  d_is_ready = true;
}

//---------------------------------------------------------------------------//
MatrixDense::MatrixDense(const MatrixDense &A)
  : MatrixBase(A.number_rows(), A.number_columns())
  , d_values(NULL)
{
  // Preconditions
  Require(d_m > 0);
  Require(d_n > 0);

  // Allocate the values and initialize
  for (int i = 0; i < d_m; ++i)
    for (int j = 0; j < d_n; ++j)
      d_values[j + i * d_n] = A(i, j);

  d_is_ready = true;

#ifdef CALLOW_ENABLE_PETSC
  PetscErrorCode ierr;
  ierr = MatCreateSeqDense(PETSC_COMM_SELF, d_n, d_m,
                           d_values, &d_petsc_matrix);
  Assert(!ierr);
#endif
}

//---------------------------------------------------------------------------//
MatrixDense::~MatrixDense()
{
  if (d_is_ready)
  {
#ifdef CALLOW_ENABLE_PETSC
  PetscErrorCode ierr;
  ierr = MatDestroy(&d_petsc_matrix);
  Assert(!ierr);
#endif
    delete [] d_values;
  }
}

//---------------------------------------------------------------------------//
MatrixDense::SP_matrix
MatrixDense::Create(const int m, const int n, const double v)
{
  SP_matrix p(new MatrixDense(m, n, v));
  return p;
}


//---------------------------------------------------------------------------//
void MatrixDense::display() const
{
  Require(d_is_ready);
  printf(" Dense matrix \n");
  printf(" ---------------------------\n");
  printf("      number rows = %5i \n",   d_m);
  printf("   number columns = %5i \n",   d_n);
  printf("\n");
  if (d_m > 20 || d_n > 20)
  {
    printf("  *** matrix not printed for m or n > 20 *** \n");
    return;
  }
  for (int i = 0; i < d_m; i++)
  {
    printf(" row  %3i | ", i);
    for (int j = 0; j < d_n; j++)
    {
      double v = d_values[j + i * d_n];
      printf(" %3i (%13.6e)", j, v);
    }
    printf("\n");
  }
  printf("\n");
}

//---------------------------------------------------------------------------//
inline void MatrixDense::print_matlab(std::string filename) const
{
  Require(d_is_ready);
  FILE * f;
  f = fopen (filename.c_str(), "w");
  for (int i = 0; i < d_m; ++i)
  {
    for (int j = 0; j < d_n; ++j)
    {
      double v = d_values[j + i * d_n];
      fprintf(f, "%8i   %8i    %23.16e \n", i+1, j+1, v);
    }
  }
  fprintf(f, "\n");
  fclose (f);
}


} // end namespace callow

//---------------------------------------------------------------------------//
//              end of file MatrixDense.cc
//---------------------------------------------------------------------------//
