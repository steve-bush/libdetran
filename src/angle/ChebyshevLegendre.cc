//----------------------------------*-C++-*----------------------------------//
/**
 *  @file   ChebyshevLegendre.cc
 *  @author robertsj
 *  @date   Oct 10, 2012
 *  @brief  ChebyshevLegendre class definition.
 */
//---------------------------------------------------------------------------//

#include "ChebyshevLegendre.hh"
#include "GenerateGaussLegendre.hh"

namespace detran_angle
{

ChebyshevLegendre::ChebyshevLegendre(const size_t dim,
                                     const size_t na,
                                     const size_t np)
  : ProductQuadrature(dim, na, np, "ChebyshevLegendre")
{

  //-------------------------------------------------------------------------//
  // AZIMUTH QUADRATURE
  //-------------------------------------------------------------------------//

  // Chebyshev points are just equally spaced phi's in the first quadrant
  using detran_utilities::pi;
  for (int i = 0; i < na; ++i)
  {
    d_phi[i] = 0.25 * (2*i + 1)*pi / na;
    d_cos_phi[i] = std::cos(d_phi[i]);
    d_sin_phi[i] = std::sin(d_phi[i]);
    d_azimuth_weight[i] = 0.5 * pi / na;
  }

  //-------------------------------------------------------------------------//
  // POLAR QUADRATURE
  //-------------------------------------------------------------------------//

  // temporary arrays
  vec_dbl x(2*np, 0.0);
  vec_dbl w(2*np, 0.0);

  // generate parameters
  generate_gl_parameters(2*np, x, w);

  // fill array
  for (int i = 0; i < np; ++i)
  {
    d_cos_theta[i]    = x[i];
    d_sin_theta[i]    = std::sqrt(1.0 - x[i]*x[i]);
    d_polar_weight[i] = w[i];
  }

  //-------------------------------------------------------------------------//
  // PRODUCT QUADRATURE
  //-------------------------------------------------------------------------//

  double scale = 1.0;
  if (d_dimension == 2) scale = 2.0;
  double weight_tot = 0.0;
  int n = 0;
  for (int a = 0; a < na; ++a)
  {
    for (int p = 0; p < np; ++p, ++n)
    {
      d_mu[n]     = d_cos_theta[p] * d_cos_phi[a];
      d_eta[n]    = d_cos_theta[p] * d_sin_phi[a];
      d_xi[n]     = d_sin_theta[p];
      d_weight[n] = scale * d_polar_weight[p] * d_azimuth_weight[a];
      weight_tot += d_weight[n];
    }
  }

}

//---------------------------------------------------------------------------//
ChebyshevLegendre::SP_quadrature
ChebyshevLegendre::Create(const size_t dim,
                          const size_t na,
                          const size_t np)
{
  ChebyshevLegendre::SP_quadrature p(new ChebyshevLegendre(dim, na, np));
  return p;
}

} // namespace detran_angle


