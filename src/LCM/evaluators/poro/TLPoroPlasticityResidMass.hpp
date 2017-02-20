//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef TLPOROPLASTICITYRESIDMASS_HPP
#define TLPOROPLASTICITYRESIDMASS_HPP

#include "Phalanx_config.hpp"
#include "Phalanx_Evaluator_WithBaseImpl.hpp"
#include "Phalanx_Evaluator_Derived.hpp"
#include "Phalanx_MDField.hpp"

#include "Intrepid2_CellTools.hpp"
#include "Intrepid2_Cubature.hpp"

namespace LCM {
/** \brief
 *   Balance of mass residual for large deformation
 *   poromechanics problem.

*/

template<typename EvalT, typename Traits>
class TLPoroPlasticityResidMass : public PHX::EvaluatorWithBaseImpl<Traits>,
                                  public PHX::EvaluatorDerived<EvalT, Traits>  {

public:

  TLPoroPlasticityResidMass(Teuchos::ParameterList& p);

  void postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& vm);

  void evaluateFields(typename Traits::EvalData d);

private:

  typedef typename EvalT::ScalarT ScalarT;
  typedef typename EvalT::MeshScalarT MeshScalarT;

  // Input:
  PHX::MDField<MeshScalarT,Cell,Node,QuadPoint> wBF;
  PHX::MDField<ScalarT,Cell,QuadPoint> porePressure;
  PHX::MDField<ScalarT,Cell,QuadPoint> Tdot;
  //PHX::MDField<ScalarT,Cell,QuadPoint> stabParameter;
  PHX::MDField<ScalarT,Cell,QuadPoint> ThermalCond;
  PHX::MDField<ScalarT,Cell,QuadPoint> kcPermeability;
  PHX::MDField<ScalarT,Cell,QuadPoint> porosity;
  PHX::MDField<ScalarT,Cell,QuadPoint> biotCoefficient;
  PHX::MDField<ScalarT,Cell,QuadPoint> biotModulus;
  PHX::MDField<MeshScalarT,Cell,Node,QuadPoint,Dim> wGradBF;
  PHX::MDField<ScalarT,Cell,QuadPoint,Dim> TGrad;
  PHX::MDField<ScalarT,Cell,QuadPoint> Source;
  Teuchos::Array<double> convectionVels;
  PHX::MDField<ScalarT,Cell,QuadPoint> rhoCp;
  PHX::MDField<ScalarT,Cell,QuadPoint> Absorption;
  PHX::MDField<ScalarT,Cell,QuadPoint,Dim,Dim> strain;

  PHX::MDField<ScalarT,Cell,QuadPoint,Dim,Dim> defgrad;
  PHX::MDField<ScalarT,Cell,QuadPoint> J;
  PHX::MDField<ScalarT,Cell,QuadPoint> elementLength;

  // stabilization term
  PHX::MDField<MeshScalarT,Cell,Vertex,Dim> coordVec;
  Teuchos::RCP<Intrepid2::Cubature<PHX::Device>> cubature;
  Teuchos::RCP<shards::CellTopology> cellType;
  PHX::MDField<MeshScalarT,Cell,QuadPoint> weights;

  // Time
  PHX::MDField<ScalarT,Dummy> deltaTime;

  //Data from previous time step
  std::string strainName, porePressureName, porosityName,
              JName;



  bool haveSource;
  bool haveConvection;
  bool haveAbsorption;
  bool enableTransient;
  bool haverhoCp;
  bool haveMechanics;
  unsigned int numNodes;
  unsigned int numQPs;
  unsigned int numDims;
  unsigned int worksetSize;

  // Temporary Views
  Kokkos::DynRankView<ScalarT, PHX::Device> flux;
  Kokkos::DynRankView<ScalarT, PHX::Device> fluxdt;
  Kokkos::DynRankView<ScalarT, PHX::Device> pterm;
  Kokkos::DynRankView<ScalarT, PHX::Device> tpterm;
  Kokkos::DynRankView<ScalarT, PHX::Device> aterm;

  // Work space FCs
  Kokkos::DynRankView<ScalarT, PHX::Device> F_inv;
  Kokkos::DynRankView<ScalarT, PHX::Device> F_invT;
  Kokkos::DynRankView<ScalarT, PHX::Device> C;
  Kokkos::DynRankView<ScalarT, PHX::Device> Cinv;
  Kokkos::DynRankView<ScalarT, PHX::Device> JF_invT;
  Kokkos::DynRankView<ScalarT, PHX::Device> KJF_invT;
  Kokkos::DynRankView<ScalarT, PHX::Device> Kref;

  ScalarT porePbar, vol;
  ScalarT trialPbar;




  // Output:
  PHX::MDField<ScalarT,Cell,Node> TResidual;

  RealType stab_param_;

};
}

#endif
