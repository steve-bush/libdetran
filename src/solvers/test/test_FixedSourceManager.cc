//----------------------------------*-C++-*----------------------------------//
/**
 *  @file   test_FixedSourceManager.cc
 *  @author robertsj
 *  @date   Apr 4, 2012
 *  @brief  test_FixedSourceManager class definition.
 */
//---------------------------------------------------------------------------//

// LIST OF TEST FUNCTIONS
#define TEST_LIST                        \
        FUNC(test_FixedSourceManager_1D) \
        FUNC(test_FixedSourceManager_2D) \
        FUNC(test_FixedSourceManager_3D)

// Detran headers
#include "TestDriver.hh"
#include "FixedSourceManager.hh"
#include "Mesh1D.hh"
#include "Mesh2D.hh"
#include "Mesh3D.hh"
#include "external_source/ConstantSource.hh"
#include "callow/utils/Initialization.hh"
#include "boundary/BoundaryTraits.hh"
#include "boundary/BoundarySN.hh"

// Setup
#include "angle/test/quadrature_fixture.hh"
#include "geometry/test/mesh_fixture.hh"
#include "material/test/material_fixture.hh"
#include "external_source/test/external_source_fixture.hh"

using namespace detran_test;
using namespace detran;
using namespace detran_material;
using namespace detran_external_source;
using namespace detran_geometry;
using namespace detran_utilities;
using namespace std;
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
  callow_initialize(argc, argv);
  RUN(argc, argv);
  callow_finalize();
}

//----------------------------------------------//
// TEST DEFINITIONS
//----------------------------------------------//

SP_material test_FixedSourceManager_material()
{
  // Material (kinf = 5/4 = 1.25)
  SP_material mat(new Material(1, 1));
  mat->set_sigma_t(0, 0, 1.0);
  mat->set_sigma_s(0, 0, 0, 0.6);
  mat->set_sigma_f(0, 0, 0.5);
  mat->set_chi(0, 0, 1.0);
  mat->set_diff_coef(0, 0, 0.33);
  mat->compute_sigma_a();
  mat->finalize();
  return mat;
}

SP_mesh test_FixedSourceManager_mesh(int d)
{
  vec_dbl cm(2, 0.0); cm[1] = 10.0;
  vec_int fm(1, 10);
  vec_int mat_map(1, 0);
  SP_mesh mesh;
  if (d == 1) mesh = new Mesh1D(fm, cm, mat_map);
  if (d == 2) mesh = new Mesh2D(fm, fm, cm, cm, mat_map);
  if (d == 3) mesh = new Mesh3D(fm, fm, fm, cm, cm, cm, mat_map);
  Assert(mesh);
  return mesh;
}

InputDB::SP_input test_FixedSourceManager_input()
{
  InputDB::SP_input inp(new InputDB());
  inp->put<int>("number_groups", 1);
  inp->put<string>("problem_type", "multiply");
  inp->put<double>("inner_tolerance", 1e-17);
  inp->put<int>("quad_number_polar_octant", 2);
  return inp;
}

template <class D>
double compute_leakage(typename BoundarySN<D>::SP_boundary b,
                       SP_quadrature q,
                       SP_material mat,
                       SP_mesh mesh)
{
  typedef detran_utilities::size_t    size_t;
  typedef BoundaryValue<D>            BV_T;
  double leakage = 0;
  // For a given dimension, provide remaining dimensions
  int remdims[3][2] = {{1,2}, {0,2}, {0,1}};
  // Cell indices
  int ijk[3] = {0, 0, 0};
  int &i = ijk[0];
  int &j = ijk[1];
  int &k = ijk[2];
  // Loop over all dimensions
  for (int dim0 = 0; dim0 < D::dimension; ++dim0)
  {
    // Bounding cell indices for this dimension
    int bound[2] = {0, mesh->number_cells(dim0)-1};
    // Other dimensions
    int dim1 = remdims[dim0][0];
    int dim2 = remdims[dim0][1];
    // Loop over directions - and +
    for (int dir = 0; dir < 2; ++dir)
    {
      // Surface index
      int surface = 2 * dim0 + dir;
      // Index and width along this direction
      ijk[dim0] = bound[dir];
      double W  = mesh->width(dim0, ijk[dim0]);
      // Loop over secondaries dimensions on this surface
      for (ijk[dim1] = 0; ijk[dim1] < mesh->number_cells(dim1); ++ijk[dim1])
      {
        for (ijk[dim2] = 0; ijk[dim2] < mesh->number_cells(dim2); ++ijk[dim2])
        {
          // Loop over all groups
          for (int g = 0; g < mat->number_groups(); g++)
          {
            // Loop over angles
            for (int o = 0; o < q->outgoing_octant(surface).size(); ++o)
            {
              for (int a = 0; a < q->number_angles_octant(); ++a)
              {
                cout << " o = " << q->outgoing_octant(surface)[o]
                     << " a = " << a << endl;
                leakage +=
                  BV_T::value((*b)(surface, q->outgoing_octant(surface)[o], a, g),
                               ijk[dim1], ijk[dim2]) *
                  q->cosines(dim0)[a] * q->weight(a) *
                  mesh->width(dim1, ijk[dim1]) * mesh->width(dim2, ijk[dim2]);
              } // end angle loop
            } // end octant loop
          } // end group loop
        } // end dim2 loop
      } // end dim1 loop
    } // end dir loop
  } // end dim0 loop
  return leakage;
}

template <class D>
int test_FixedSourceManager_T()
{
  typedef FixedSourceManager<D>       Manager_T;
  typedef BoundarySN<D>               B_T;
  typedef typename B_T::SP_boundary   SP_boundary;

  // Input, materials, and mesh
  typename Manager_T::SP_input input = test_FixedSourceManager_input();
  SP_material mat = test_FixedSourceManager_material();
  SP_mesh mesh = test_FixedSourceManager_mesh(D::dimension);

  // Manager
  Manager_T manager(input, mat, mesh);
  manager.setup();

  // Build source
  ConstantSource::SP_externalsource q_e(new ConstantSource(1, mesh, 1.0));
  manager.set_source(q_e);

  // Solve.
  bool flag = manager.solve();
  TEST(flag);

  // Get the state and boundary
  SP_boundary boundary;
  boundary = manager.boundary();
  typename Manager_T::SP_state state;
  state = manager.state();

  // compute total source and absorptions
  double gain = 0;
  double absorption = 0;
  for (int i = 0; i < mesh->number_cells(); i++)
  {
    cout << state->phi(0)[i] << " " << endl;
    gain += q_e->source(i, 0);
    absorption += state->phi(0)[i] * mat->sigma_a(0, 0) * mesh->volume(i);
  }

  typename Manager_T::SP_quadrature q;
  q = manager.quadrature();
  TEST(q);

  double leakage = compute_leakage<D>(boundary, q, mat, mesh);

  // compute net balance (should be 0!!)
  double net = gain - (absorption+leakage);
  cout << "        gain = " << gain << endl;
  cout << "  absorption = " << absorption << endl;
  cout << "     leakage = " << leakage << endl;
  cout << " NET BALANCE = " << gain-(absorption+leakage) << endl;
  TEST(soft_equiv(net, 0.0, 1e-12));

  return 0;
}

int test_FixedSourceManager_1D(int argc, char *argv[])
{
  return test_FixedSourceManager_T<_1D>();
}

int test_FixedSourceManager_2D(int argc, char *argv[])
{
  return test_FixedSourceManager_T<_2D>();
}

int test_FixedSourceManager_3D(int argc, char *argv[])
{
  return test_FixedSourceManager_T<_3D>();
}

//---------------------------------------------------------------------------//
//              end of test_FixedSourceManager.cc
//---------------------------------------------------------------------------//
