// ---------------------------------------------------------------------
//
// Copyright (c) 2017 The Regents of the University of Michigan and DFT-FE authors.
//
// This file is part of the DFT-FE code.
//
// The DFT-FE code is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the DFT-FE distribution.
//
// ---------------------------------------------------------------------
//
// @author Sambit Das (2017)
//
#include "../../../include/eshelbyTensor.h"

namespace eshelbyTensor
{
Tensor<2,C_DIM,VectorizedArray<double> >  getPhiExtEshelbyTensor(const VectorizedArray<double> & phiExt,const Tensor<1,C_DIM,VectorizedArray<double> > & gradPhiExt)
{

   Tensor<2,C_DIM,VectorizedArray<double> > identityTensor;
   identityTensor[0][0]=make_vectorized_array (1.0);
   identityTensor[1][1]=make_vectorized_array (1.0);
   identityTensor[2][2]=make_vectorized_array (1.0);



   Tensor<2,C_DIM,VectorizedArray<double> > eshelbyTensor= make_vectorized_array(1.0/(4.0*M_PI))*outer_product(gradPhiExt,gradPhiExt)-make_vectorized_array(1.0/(8.0*M_PI))*scalar_product(gradPhiExt,gradPhiExt)*identityTensor;

  return eshelbyTensor;
}

/////
Tensor<2,C_DIM,VectorizedArray<double> >  getVselfBallEshelbyTensor(const Tensor<1,C_DIM,VectorizedArray<double> > & gradVself)
{

   Tensor<2,C_DIM,VectorizedArray<double> > identityTensor;
   identityTensor[0][0]=make_vectorized_array (1.0);
   identityTensor[1][1]=make_vectorized_array (1.0);
   identityTensor[2][2]=make_vectorized_array (1.0);



   Tensor<2,C_DIM,VectorizedArray<double> > eshelbyTensor= make_vectorized_array(1.0/(8.0*M_PI))*scalar_product(gradVself,gradVself)*identityTensor-make_vectorized_array(1.0/(4.0*M_PI))*outer_product(gradVself,gradVself);

  return eshelbyTensor;
}


Tensor<2,C_DIM,double >  getVselfBallEshelbyTensor(const Tensor<1,C_DIM,double> & gradVself)
{

   double identityTensorFactor=1.0/(8.0*M_PI)*scalar_product(gradVself,gradVself);
   Tensor<2,C_DIM,double > eshelbyTensor= -1.0/(4.0*M_PI)*outer_product(gradVself,gradVself);

   eshelbyTensor[0][0]+=identityTensorFactor;
   eshelbyTensor[1][1]+=identityTensorFactor;
   eshelbyTensor[2][2]+=identityTensorFactor;
  
   return eshelbyTensor;
}


Tensor<2,C_DIM,VectorizedArray<double> >  getELocEshelbyTensorPeriodic(const VectorizedArray<double> & phiTot,
		                                                       const Tensor<1,C_DIM,VectorizedArray<double> > & gradPhiTot,
								       const VectorizedArray<double> & rho,
								       const Tensor<1,C_DIM,VectorizedArray<double> > & gradRho,
								       const VectorizedArray<double> & exc,
								       const Tensor<1,C_DIM,VectorizedArray<double> > & gradRhoExc,
								       std::vector<Tensor<1,2,VectorizedArray<double> > >::const_iterator psiBegin,
                                                                       std::vector<Tensor<1,2,Tensor<1,C_DIM,VectorizedArray<double> > > >::const_iterator gradPsiBegin,
								       const std::vector<double> & kPointCoordinates,
                                                                       const std::vector<double> & kPointWeights,							
								       const std::vector<std::vector<double> > & eigenValues_,
								       const double fermiEnergy_,
								       const double tVal)
{


   Tensor<2,C_DIM,VectorizedArray<double> > eshelbyTensor= make_vectorized_array(1.0/(4.0*M_PI))*outer_product(gradPhiTot,gradPhiTot);
   VectorizedArray<double> identityTensorFactor=make_vectorized_array(-1.0/(8.0*M_PI))*scalar_product(gradPhiTot,gradPhiTot)+rho*phiTot+exc*rho;

   std::vector<Tensor<1,2,VectorizedArray<double> > >::const_iterator it1=psiBegin;
   std::vector<Tensor<1,2,Tensor<1,C_DIM,VectorizedArray<double> > > >::const_iterator it2=gradPsiBegin;

   Tensor<1,C_DIM,VectorizedArray<double> > kPointCoord;
   for (unsigned int ik=0; ik<eigenValues_.size(); ++ik){
     kPointCoord[0]=make_vectorized_array(kPointCoordinates[ik*C_DIM+0]);
     kPointCoord[1]=make_vectorized_array(kPointCoordinates[ik*C_DIM+1]);
     kPointCoord[2]=make_vectorized_array(kPointCoordinates[ik*C_DIM+2]);
     for (unsigned int eigenIndex=0; eigenIndex<eigenValues_[0].size(); ++it1, ++it2, ++ eigenIndex){
        const Tensor<1,2,VectorizedArray<double> > & psi= *it1;
        const Tensor<1,2,Tensor<1,C_DIM,VectorizedArray<double> > >  & gradPsi=*it2;

        double factor=(eigenValues_[ik][eigenIndex]-fermiEnergy_)/(C_kb*tVal);
        double partOcc = (factor >= 0)?std::exp(-factor)/(1.0 + std::exp(-factor)) : 1.0/(1.0 + std::exp(factor));

	VectorizedArray<double> identityTensorFactorContribution=make_vectorized_array(0.0);
	VectorizedArray<double> fnk=make_vectorized_array(partOcc*kPointWeights[ik]);
        identityTensorFactorContribution+=(scalar_product(gradPsi[0],gradPsi[0])+scalar_product(gradPsi[1],gradPsi[1]));
	identityTensorFactorContribution+=make_vectorized_array(2.0)*(psi[0]*scalar_product(kPointCoord,gradPsi[1])-psi[1]*scalar_product(kPointCoord,gradPsi[0]));
	identityTensorFactorContribution+=(scalar_product(kPointCoord,kPointCoord)-make_vectorized_array(2.0*eigenValues_[ik][eigenIndex]))*(psi[0]*psi[0]+psi[1]*psi[1]);
        identityTensorFactorContribution*=fnk;
	identityTensorFactor+=identityTensorFactorContribution;

        eshelbyTensor-=make_vectorized_array(2.0)*fnk*(outer_product(gradPsi[0],gradPsi[0])+outer_product(gradPsi[1],gradPsi[1])+psi[0]*outer_product(gradPsi[1],kPointCoord)-psi[1]*outer_product(gradPsi[0],kPointCoord));
     }
   }

   eshelbyTensor[0][0]+=identityTensorFactor;
   eshelbyTensor[1][1]+=identityTensorFactor;
   eshelbyTensor[2][2]+=identityTensorFactor;
   return eshelbyTensor;
}

Tensor<2,C_DIM,VectorizedArray<double> >  getELocEshelbyTensorNonPeriodic(const VectorizedArray<double> & phiTot,
		                                                          const Tensor<1,C_DIM,VectorizedArray<double> > & gradPhiTot,
									  const VectorizedArray<double> & rho,
									  const Tensor<1,C_DIM,VectorizedArray<double> > & gradRho,
									  const VectorizedArray<double> & exc,
									  const Tensor<1,C_DIM,VectorizedArray<double> > & gradRhoExc,
									  std::vector<VectorizedArray<double> >::const_iterator psiBegin,
                                                                          std::vector<Tensor<1,C_DIM,VectorizedArray<double> > >::const_iterator gradPsiBegin,
									  const std::vector<double> & eigenValues_,
									  const double fermiEnergy_,
									  const double tVal)
{

   Tensor<2,C_DIM,VectorizedArray<double> > eshelbyTensor= make_vectorized_array(1.0/(4.0*M_PI))*outer_product(gradPhiTot,gradPhiTot);
   VectorizedArray<double> identityTensorFactor=make_vectorized_array(-1.0/(8.0*M_PI))*scalar_product(gradPhiTot,gradPhiTot)+rho*phiTot+exc*rho;

   std::vector<VectorizedArray<double> >::const_iterator it1=psiBegin;   
   std::vector<Tensor<1,C_DIM,VectorizedArray<double> > >::const_iterator it2=gradPsiBegin;
   for (unsigned int eigenIndex=0; eigenIndex < eigenValues_.size(); ++it1, ++it2, ++ eigenIndex){
      const VectorizedArray<double> & psi= *it1;
      const Tensor<1,C_DIM,VectorizedArray<double> > & gradPsi=*it2;
      double factor=(eigenValues_[eigenIndex]-fermiEnergy_)/(C_kb*tVal);
      double partOcc = (factor >= 0)?std::exp(-factor)/(1.0 + std::exp(-factor)) : 1.0/(1.0 + std::exp(factor));	   
      identityTensorFactor+=make_vectorized_array(partOcc)*scalar_product(gradPsi,gradPsi)-make_vectorized_array(2*partOcc*eigenValues_[eigenIndex])*psi*psi;
      eshelbyTensor-=make_vectorized_array(2.0*partOcc)*outer_product(gradPsi,gradPsi);
   }
   
   eshelbyTensor[0][0]+=identityTensorFactor;
   eshelbyTensor[1][1]+=identityTensorFactor;
   eshelbyTensor[2][2]+=identityTensorFactor;
   return eshelbyTensor;
}

/*
Tensor<2,C_DIM,VectorizedArray<double> >  getENonLocEshelbyTensor(const VectorizedArray<double> & phiTot,
		                                                  const Tensor<1,C_DIM,VectorizedArray<double> > & gradPhiTot,
							          const VectorizedArray<double> & rho,
							          const Tensor<1,C_DIM,VectorizedArray<double> > & gradRho,
								  const VectorizedArray<double> & exc,
								  const Tensor<1,C_DIM,VectorizedArray<double> > & gradRhoExc,
								  std::vector<VectorizedArray<double> >::const_iterator psiBegin,
                                                                  std::vector<Tensor<1,C_DIM,VectorizedArray<double> > >::const_iterator gradPsiBegin,
								  const std::vector<double> & eigenValues_,
								  const double fermiEnergy_,
								  const double tVal)
{

   Tensor<2,C_DIM,VectorizedArray<double> > eshelbyTensor= make_vectorized_array(1.0/(4.0*M_PI))*outer_product(gradPhiTot,gradPhiTot);
   VectorizedArray<double> identityTensorFactor=make_vectorized_array(-1.0/(8.0*M_PI))*scalar_product(gradPhiTot,gradPhiTot)+rho*phiTot+exc*rho;

   
   eshelbyTensor[0][0]+=identityTensorFactor;
   eshelbyTensor[1][1]+=identityTensorFactor;
   eshelbyTensor[2][2]+=identityTensorFactor;
   return eshelbyTensor;
}
*/

Tensor<1,C_DIM,VectorizedArray<double> >  getFPSPLocal(const VectorizedArray<double> rho,
		                                       const Tensor<1,C_DIM,VectorizedArray<double> > & gradVPseudoLocal,
						       const Tensor<1,C_DIM,VectorizedArray<double> > & gradSumVself)

{

   return rho*(gradVPseudoLocal-gradSumVself);
}

//Tensor<1,C_DIM,VectorizedArray<double> >  getFPSPNonLocal()

//{


//}

}
