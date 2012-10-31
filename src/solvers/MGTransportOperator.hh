//----------------------------------*-C++-*----------------------------------//
/**
 *  @file   MGTransportOperator.hh
 *  @brief  MGTransportOperator
 *  @author Jeremy Roberts
 *  @date   Oct 31, 2012
 */
//---------------------------------------------------------------------------//

#ifndef detran_MGTRANSPORTOPERATOR_HH_
#define detran_MGTRANSPORTOPERATOR_HH_

#include "boundary/BoundaryBase.hh"
#include "callow/matrix/MatrixShell.hh"
#include "transport/State.hh"
#include "transport/Sweeper.hh"
#include "transport/SweepSource.hh"
#include "utilities/DBC.hh"
#include "utilities/Definitions.hh"

namespace detran
{

/**
 *  @class MGTransportOperator
 *  @brief Multigroup transport operator
 *
 *  The multigroup transport operator is defined as
 *  ... finish.
 */
template <class D>
class MGTransportOperator: public callow::MatrixShell
{

public:

  //-------------------------------------------------------------------------//
  // TYPEDEFS
  //-------------------------------------------------------------------------//

  typedef MatrixShell                                 Base;
  typedef MGTransportOperator<D>                      Operator_T;
  typedef typename detran_utilities::SP<Operator_T>   SP_operator;
  typedef State::SP_state                             SP_state;
  typedef typename BoundaryBase<D>::SP_boundary       SP_boundary;
  typedef detran_utilities::size_t                    size_t;
  typedef typename Sweeper<D>::SP_sweeper             SP_sweeper;
  typedef typename SweepSource<D>::SP_sweepsource     SP_sweepsource;
  typedef State::moments_type                         moments_type;
  typedef callow::Vector                              Vector;

  //-------------------------------------------------------------------------//
  // CONSTRUCTOR & DESTRUCTOR
  //-------------------------------------------------------------------------//

  // Constructor
  MGTransportOperator(SP_state        state,
                      SP_boundary     boundary,
                      SP_sweeper      sweeper,
                      SP_sweepsource  source);

  // Destructor
  virtual ~MGTransportOperator(){}

  //-------------------------------------------------------------------------//
  // PUBLIC FUNCTIONS
  //-------------------------------------------------------------------------//

  // default shell display gives just the sizes
  virtual void display() const;

  //-------------------------------------------------------------------------//
  // ABSTRACT INTERFACE -- ALL MATRICES MUST IMPLEMENT THESE
  //-------------------------------------------------------------------------//

  // the client must implement the action y <-- A * x
  virtual void multiply(const Vector &x,  Vector &y);

  // the client must implement the action y <-- A' * x
  virtual void multiply_transpose(const Vector &x, Vector &y)
  {
    THROW("Transpose transport operator not implemented");
  }

private:

  //-------------------------------------------------------------------------//
  // DATA
  //-------------------------------------------------------------------------//

  SP_state d_state;
  SP_boundary d_boundary;
  SP_sweeper d_sweeper;
  SP_sweepsource d_sweepsource;
  /// Group index below which the operator is not applicable
  size_t d_upscatter_cutoff;
  /// Number of groups for which operator is applicable
  size_t d_number_groups;
  /// Size of a group moment vector
  size_t d_moments_size;
  /// Size of a group boundary vector
  size_t d_boundary_size;

};

} // end namespace detran

#endif // MGTRANSPORTOPERATOR_HH_ 

//---------------------------------------------------------------------------//
//              end of file MGTransportOperator.hh
//---------------------------------------------------------------------------//
