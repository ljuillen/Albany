//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Teuchos_TestForException.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Phalanx_TypeStrings.hpp"


namespace Tsunami {


//**********************************************************************
template<typename EvalT, typename Traits>
NavierStokesTau<EvalT, Traits>::
NavierStokesTau(const Teuchos::ParameterList& p,
           const Teuchos::RCP<Albany::Layouts>& dl) :
  V       (p.get<std::string> ("Velocity QP Variable Name"), dl->qp_vector),
  Gc      (p.get<std::string> ("Contravarient Metric Tensor Name"), dl->qp_tensor),
  jacobian_det (p.get<std::string>  ("Jacobian Det Name"), dl->qp_scalar ),
  Tau    (p.get<std::string> ("Tau Name"), dl->qp_scalar)
{

  this->addDependentField(V);
  this->addDependentField(Gc);
  this->addDependentField(jacobian_det);

  this->addEvaluatedField(Tau);

  std::vector<PHX::DataLayout::size_type> dims;
  dl->qp_gradient->dimensions(dims);
  numCells= dims[0];
  numQPs  = dims[1];
  numDims = dims[2];
  mu = p.get<double>("Viscosity"); 
  rho = p.get<double>("Density"); 
  stabType = p.get<std::string>("Stabilization Type"); 
  if (stabType == "Shakib-Hughes") 
    stab_type=SHAKIBHUGHES;
  else if (stabType == "Tsunami")
    stab_type=TSUNAMI; 
  this->setName("NavierStokesTau"+PHX::typeAsString<EvalT>());
}

//**********************************************************************
template<typename EvalT, typename Traits>
void NavierStokesTau<EvalT, Traits>::
postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& fm)
{
  this->utils.setFieldData(V,fm);
  this->utils.setFieldData(Gc,fm);
  this->utils.setFieldData(jacobian_det,fm);

  this->utils.setFieldData(Tau,fm);

  // Allocate workspace
  normGc = Kokkos::createDynRankView(Gc.get_view(), "XXX", numCells, numQPs);
}

//**********************************************************************
template<typename EvalT, typename Traits>
void NavierStokesTau<EvalT, Traits>::
evaluateFields(typename Traits::EvalData workset)
{
  if (stab_type == SHAKIBHUGHES) {
    for (std::size_t cell=0; cell < workset.numCells; ++cell) {
      for (std::size_t qp=0; qp < numQPs; ++qp) {
        Tau(cell,qp) = 0.0;
        normGc(cell,qp) = 0.0;
        for (std::size_t i=0; i < numDims; ++i) {
          for (std::size_t j=0; j < numDims; ++j) {
            Tau(cell,qp) += rho*rho*V(cell,qp,i)*Gc(cell,qp,i,j)*V(cell,qp,j);
            normGc(cell,qp) += Gc(cell,qp,i,j)*Gc(cell,qp,i,j);
          }
        }
        Tau(cell,qp) += 12.*mu*mu*std::sqrt(normGc(cell,qp));
        Tau(cell,qp) = 1./std::sqrt(Tau(cell,qp));
      }
    }
  }
  //IKT, FIXME: Zhiheng & Xiaoshu - please fill in 
  /*else if (stab_type == TSUNAMI) {
    double dt  = workset.time_step; 
    for (std::size_t cell=0; cell < workset.numCells; ++cell) {
      for (std::size_t qp=0; qp < numQPs; ++qp) {
        //Estimate of mesh size h 
        double h = 2.0*pow(jacobian_det(cell,qp), 1.0/numDims);
      }
    }
  }*/
}

//**********************************************************************
}

