#ifndef TFHE_TEST_ENVIRONMENT
#include <cstdlib>
#include <iostream>
#include <random>
#include <cassert>
#include "tfhe_core.h"
#include "numeric_functions.h"
#include "tlwe_functions.h"
#include "tgsw_functions.h"
#include "polynomials_arithmetic.h"
#include "lagrangehalfc_arithmetic.h"
#define INCLUDE_ALL
#else
#undef EXPORT
#define EXPORT
#endif


#if defined INCLUDE_ALL || defined INCLUDE_TGSW_KEYGEN
#undef INCLUDE_TGSW_KEYGEN
// TGsw
/** generate a tgsw key (in fact, a tlwe key) */
template<typename TORUS>
void TGswFunctions<TORUS>::KeyGen(TGswKey<TORUS>* result){
  tLweKeyGen(&result->tlwe_key);
}
#endif


#if defined INCLUDE_ALL || defined INCLUDE_TGSW_CLEAR
#undef INCLUDE_TGSW_CLEAR
// support Functions for TGsw
// Result = 0
template<typename TORUS>
void TGswFunctions<TORUS>::Clear(TGswSample<TORUS>* result, const TGswParams<TORUS>* params){
  const int kpl = params->kpl;

  for (int p = 0; p < kpl; ++p)
    tLweClear(&result->all_sample[p], params->tlwe_params);
}
#endif

#if defined INCLUDE_ALL || defined INCLUDE_TGSW_ADD_H
#undef INCLUDE_TGSW_ADD_H
// Result += H
template<typename TORUS>
void TGswFunctions<TORUS>::AddH(TGswSample<TORUS>* result, const TGswParams<TORUS>* params){
  const int k = params->tlwe_params->k;
  const int l = params->l;
  const TORUS* h = params->h;

  // compute result += H
  for (int bloc = 0; bloc <= k; ++bloc)
    for (int i=0; i<l; i++)
      result->bloc_sample[bloc][i].a[bloc].coefsT[0]+=h[i];
}
#endif

#if defined INCLUDE_ALL || defined INCLUDE_TGSW_ADD_MU_H
#undef INCLUDE_TGSW_ADD_MU_H
// Result += mu*H
template<typename TORUS>
void TGswFunctions<TORUS>::AddMuH(TGswSample<TORUS>* result, const IntPolynomial* message, const TGswParams<TORUS>* params) {
  const int k = params->tlwe_params->k;
  const int N = params->tlwe_params->N;
  const int l = params->l;
  const TORUS* h = params->h;
  const int* mu = message->coefs;

  // compute result += H
  for (int bloc = 0; bloc <= k; ++bloc)
  for (int i=0; i<l; i++) {
    TORUS* target = result->bloc_sample[bloc][i].a[bloc].coefsT;
    const TORUS hi=h[i];
    for (int j=0; j<N; j++) {
      target[j]+=mu[j]*hi;
    }
  }
}
#endif

#if defined INCLUDE_ALL || defined INCLUDE_TGSW_ADD_MU_INT_H
#undef INCLUDE_TGSW_ADD_MU_INT_H
// Result += mu*H, mu integer
template<typename TORUS>
void TGswFunctions<TORUS>::AddMuIntH(TGswSample<TORUS>* result, const int message, const TGswParams<TORUS>* params)
{
  const int k = params->tlwe_params->k;
  const int l = params->l;
  const TORUS* h = params->h;

  // compute result += H
  for (int bloc = 0; bloc <= k; ++bloc)
    for (int i=0; i<l; i++)
      result->bloc_sample[bloc][i].a[bloc].coefsT[0]+=message*h[i];
}
#endif

#if defined INCLUDE_ALL || defined INCLUDE_TGSW_ENCRYPT_ZERO
#undef INCLUDE_TGSW_ENCRYPT_ZERO
// Result = tGsw(0)
template<typename TORUS>
void TGswFunctions<TORUS>::EncryptZero(TGswSample<TORUS>* result, double alpha, const TGswKey<TORUS>* key){
  const TLweKey<TORUS>* rlkey = &key->tlwe_key;
  const int kpl = key->params->kpl;

  for (int p = 0; p < kpl; ++p)
    tLweSymEncryptZero(&result->all_sample[p], alpha, rlkey);
}
#endif






#if defined INCLUDE_ALL || defined INCLUDE_TGSW_MUL_BY_XAI_MINUS_ONE
#undef INCLUDE_TGSW_MUL_BY_XAI_MINUS_ONE
//mult externe de X^{a_i} par bki
template<typename TORUS>
void TGswFunctions<TORUS>::MulByXaiMinusOne(TGswSample<TORUS>* result, int ai, const TGswSample<TORUS>* bk, const TGswParams<TORUS>* params){
  const TLweParams<TORUS>* par=params->tlwe_params;
  const int kpl=params->kpl;
  for (int i=0;i<kpl;i++)
    tLweMulByXaiMinusOne(&result->all_sample[i],ai,&bk->all_sample[i],par);
}
#endif

#if defined INCLUDE_ALL || defined INCLUDE_TGSW_EXTERN_MUL_TO_TLWE
#undef INCLUDE_TGSW_EXTERN_MUL_TO_TLWE
//Update l'accumulateur ligne 5 de l'algo toujours
//void tGswTLweDecompH(IntPolynomial* result, const TLweSample<TORUS>* sample,const TGswParams<TORUS>* params);
//accum *= sample
template<typename TORUS>
void TGswFunctions<TORUS>::ExternMulToTLwe(TLweSample<TORUS>* accum, const TGswSample<TORUS>* sample, const TGswParams<TORUS>* params){
  const TLweParams<TORUS>* par=params->tlwe_params;
  const int N=par->N;
  const int kpl=params->kpl;
  //TODO: improve this new/delete
  IntPolynomial* dec =new_IntPolynomial_array(kpl,N);

  tGswTLweDecompH(dec,accum,params);
  tLweClear(accum,par);
  for (int i=0; i<kpl;i++) {
    tLweAddMulRTo(accum,&dec[i],&sample->all_sample[i],par);
  }

  delete_IntPolynomial_array(kpl, dec);
}
#endif


#if defined INCLUDE_ALL || defined INCLUDE_TGSW_SYM_ENCRYPT
#undef INCLUDE_TGSW_SYM_ENCRYPT
/**
 * encrypts a poly message
 */
template<typename TORUS>
void TGswFunctions<TORUS>::SymEncrypt(TGswSample<TORUS>* result, const IntPolynomial* message, double alpha, const TGswKey<TORUS>* key){
  tGswEncryptZero(result, alpha, key);
  tGswAddMuH(result, message, key->params);
}
#endif


#if defined INCLUDE_ALL || defined INCLUDE_TGSW_SYM_ENCRYPT_INT
#undef INCLUDE_TGSW_SYM_ENCRYPT_INT
/**
 * encrypts a constant message
 */
template<typename TORUS>
void TGswFunctions<TORUS>::SymEncryptInt(TGswSample<TORUS>* result, const int message, double alpha, const TGswKey<TORUS>* key){
  tGswEncryptZero(result, alpha, key);
  tGswAddMuIntH(result, message, key->params);
}
#endif


// #if defined INCLUDE_ALL || defined INCLUDE_TGSW_ENCRYPT_B
// #undef INCLUDE_TGSW_ENCRYPT_B
// /**
//  * encrypts a message = 0 ou 1
//  */
// template<typename TORUS>
// void TGswFunctions<TORUS>::EncryptB(TGswSample<TORUS>* result, const int message, double alpha, const TGswKey<TORUS>* key){
//   tGswEncryptZero(result, alpha, key);
//   if (message == 1)
//     tGswAddH(result, key->params);
// }
// #endif




#if defined INCLUDE_ALL || defined INCLUDE_TGSW_DECRYPT
#undef INCLUDE_TGSW_DECRYPT
// à revoir
template<typename TORUS>
void TGswFunctions<TORUS>::SymDecrypt(IntPolynomial* result, const TGswSample<TORUS>* sample, const TGswKey<TORUS>* key, const int Msize){
  const TGswParams<TORUS>* params = key->params;
  const TLweParams<TORUS>* rlwe_params = params->tlwe_params;
  const int N = rlwe_params->N;
  const int l = params->l;
  const int k = rlwe_params->k;
  TorusPolynomial<TORUS>* testvec = new_TorusPolynomial<TORUS>(N);
  TorusPolynomial<TORUS>* tmp = new_TorusPolynomial<TORUS>(N);
  IntPolynomial* decomp = new_IntPolynomial_array(l,N);

  const TORUS indic = TorusUtils<TORUS>::modSwitchToTorus(1, Msize);
  torusPolynomialClear(testvec);
  testvec->coefsT[0]=indic;
  tGswTorusPolynomialDecompH(decomp, testvec, params);

  torusPolynomialClear(testvec);
  for (int i=0; i<l; i++) {
    for (int j=1; j<N; j++) assert(decomp[i].coefs[j]==0);
    tLwePhase(tmp, &sample->bloc_sample[k][i], &key->tlwe_key);
    torusPolynomialAddMulR(testvec, decomp+i, tmp);
  }
  for (int i=0; i<N; i++)
    result->coefs[i]=TorusUtils<TORUS>::modSwitchFromTorus(testvec->coefsT[i], Msize);

  delete_TorusPolynomial(testvec);
  delete_TorusPolynomial(tmp);
  delete_IntPolynomial_array(l,decomp);
}
#endif

/*
// à revoir
template<typename TORUS>
int TGswFunctions<TORUS>::SymDecryptInt(const TGswSample<TORUS>* sample, const TGswKey<TORUS>* key){
  TorusPolynomial<TORUS>* phase = new_TorusPolynomial<TORUS>(key->params->tlwe_params->N);

  tGswPhase(phase, sample, key);
  int result = TorusUtils<TORUS>::modSwitchFromTorus(phase->coefsT[0], Msize);

  delete_TorusPolynomial(phase);
  return result;
}
*/
//do we really decrypt Gsw samples?
// template<typename TORUS>
// EXPORT void TGswFunctions<TORUS>::MulByXaiMinusOne(Gsw* result, int ai, const Gsw* bk);
// EXPORT void tLweExternMulRLweTo(RLwe* accum, Gsw* a); //  accum = a \odot accum


#if defined INCLUDE_ALL || defined INCLUDE_TGSW_TLWE_DECOMP_H
#undef INCLUDE_TGSW_TLWE_DECOMP_H
//fonction de decomposition
template<typename TORUS>
void TGswFunctions<TORUS>::TLweDecompH(IntPolynomial* result, const TLweSample<TORUS>* sample, const TGswParams<TORUS>* params){
  const int k = params->tlwe_params->k;
  const int l = params->l;

  for (int i = 0; i <= k; ++i) // b=a[k]
    tGswTorusPolynomialDecompH(result+(i*l), &sample->a[i], params);
}
#endif


// #if defined INCLUDE_ALL || defined INCLUDE_TGSW_TORUS32POLYNOMIAL_DECOMP_H_OLD
// #undef INCLUDE_TGSW_TORUS32POLYNOMIAL_DECOMP_H_OLD
// EXPORT void Torus32PolynomialDecompH_old(IntPolynomial* result, const TorusPolynomial<TORUS>* sample, const TGswParams<TORUS>* params){
//   const int N = params->tlwe_params->N;
//   const int l = params->l;
//   const int Bgbit = params->Bgbit;
//   const uint32_t maskMod = params->maskMod;
//   const int32_t halfBg = params->halfBg;
//   const uint32_t offset = params->offset;

//   for (int j = 0; j < N; ++j)
//   {
//     uint32_t temp0 = sample->coefsT[j] + offset;
//     for (int p = 0; p < l; ++p)
//     {
//       uint32_t temp1 = (temp0 >> (32-(p+1)*Bgbit)) &maskMod; // doute
//       result[p].coefs[j] = temp1 - halfBg;
//     }
//   }
// }
// #endif

#if defined INCLUDE_ALL || defined INCLUDE_TGSW_TORUS32POLYNOMIAL_DECOMP_H
#undef INCLUDE_TGSW_TORUS32POLYNOMIAL_DECOMP_H

template<typename TORUS>
void TGswFunctions<TORUS>::TorusPolynomialDecompH(IntPolynomial* result, const TorusPolynomial<TORUS>* sample, const TGswParams<TORUS>* params){
  typedef typename TorusUtils<TORUS>::UTORUS UTORUS;

  const int N = params->tlwe_params->N;
  const int l = params->l;
  const int Bgbit = params->Bgbit;
  UTORUS* buf = (UTORUS*) sample->coefsT;
  const UTORUS maskMod = params->maskMod;
  const TORUS halfBg = params->halfBg;
  const UTORUS offset = params->offset;

  //First, add offset to everyone
  for (int j = 0; j < N; ++j) buf[j]+=offset;

  //then, do the decomposition (in parallel)
  for (int p = 0; p < l; ++p)
  {
    const int decal = (TorusUtils<TORUS>::bit_cnt-(p+1)*Bgbit);
    int32_t* res_p = result[p].coefs;
    for (int j = 0; j < N; ++j)
    {
      UTORUS temp1 = (buf[j] >> decal) & maskMod;
      res_p[j] = (int32_t)(temp1 - halfBg);
    }
  }

  //finally, remove offset to everyone
  for (int j = 0; j < N; ++j) buf[j]-=offset;
}

#ifdef __AVX2__

template<>
void TGswFunctions<Torus32>::TorusPolynomialDecompH(IntPolynomial* result, const TorusPolynomial<Torus32>* sample, const TGswParams<Torus32>* params){
  const int N = params->tlwe_params->N;
  const int l = params->l;
  const int Bgbit = params->Bgbit;
  uint32_t* buf = (uint32_t*) sample->coefsT;

  const uint32_t* maskMod_addr = &params->maskMod;
  const int32_t* halfBg_addr = &params->halfBg;
  const uint32_t* offset_addr = &params->offset;

  //First, add offset to everyone
  {
    const uint32_t* sit = buf;
    const uint32_t* send = buf+N;
    __asm__ __volatile__ (
      "vpbroadcastd (%2),%%ymm0\n"
      "0:\n"
      "vmovdqu (%0),%%ymm3\n"
      "vpaddd %%ymm0,%%ymm3,%%ymm3\n" // add offset
      "vmovdqu %%ymm3,(%0)\n"
      "addq $32,%0\n"
      "cmpq %1,%0\n"
      "jb 0b\n"
      : "=r"(sit),"=r"(send),"=r"(offset_addr)
      :  "0"(sit), "1"(send), "2"(offset_addr)
      : "%ymm0","%ymm3","memory"
      );
  }

  //then, do the decomposition (in parallel)
  for (int p = 0; p < l; ++p)
  {
    const int decal = (32-(p+1)*Bgbit);
    int32_t* dst = result[p].coefs;
    const uint32_t* sit = buf;
    const uint32_t* send = buf+N;
    const int32_t* decal_addr = &decal;
    __asm__ __volatile__ (
      "vpbroadcastd (%4),%%ymm0\n"
      "vpbroadcastd (%5),%%ymm1\n"
      "vmovd (%3),%%xmm2\n"
      "1:\n"
      "vmovdqu (%1),%%ymm3\n"
      "VPSRLD %%xmm2,%%ymm3,%%ymm3\n" // shift by decal
      "VPAND %%ymm1,%%ymm3,%%ymm3\n"  // and maskMod
      "VPSUBD %%ymm0,%%ymm3,%%ymm3\n" // sub halfBg
      "vmovdqu %%ymm3,(%0)\n"
      "addq $32,%0\n"
      "addq $32,%1\n"
      "cmpq %2,%1\n"
      "jb 1b\n"
      : "=r"(dst),"=r"(sit),"=r"(send),"=r"(decal_addr),"=r"(halfBg_addr),"=r"(maskMod_addr)
      :  "0"(dst), "1"(sit), "2"(send), "3"(decal_addr), "4"(halfBg_addr) ,"5"(maskMod_addr)
      : "%ymm0","%ymm1","%ymm2","%ymm3","memory"
      );
    /* // verify that the assembly block was ok
    int32_t* res_p = result[p].coefs;
    for (int j = 0; j < N; ++j)
    {
      uint32_t temp1 = (buf[j] >> decal) & maskMod;
      if (res_p[j] != int32_t(temp1 - halfBg)) {
      fprintf(stderr, "j=%d,buf[j]=%u,decal=%u,mask=%u,halfbg=%d,res_p[j]=%d\n",j,buf[j],decal,maskMod,halfBg,res_p[j]);
      abort();
      }
    }*/
  }

  //finally, remove offset to everyone
  {
    const uint32_t* sit = buf;
    const uint32_t* send = buf+N;
    __asm__ __volatile__ (
      "vpbroadcastd (%2),%%ymm0\n"
      "2:\n"
      "vmovdqu (%0),%%ymm3\n"
      "vpsubd %%ymm0,%%ymm3,%%ymm3\n" // add offset
      "vmovdqu %%ymm3,(%0)\n"
      "addq $32,%0\n"
      "cmpq %1,%0\n"
      "jb 2b\n"
      "vzeroall\n"
      : "=r"(sit),"=r"(send),"=r"(offset_addr)
      :  "0"(sit), "1"(send), "2"(offset_addr)
      : "%ymm0","%ymm3","memory"
      );
  }
}

#endif

#endif


#if defined INCLUDE_ALL || defined INCLUDE_TGSW_EXTERN_PRODUCT
#undef INCLUDE_TGSW_EXTERN_PRODUCT
//result = a*b
template<typename TORUS>
void TGswFunctions<TORUS>::ExternProduct(TLweSample<TORUS>* result, const TGswSample<TORUS>* a, const TLweSample<TORUS>* b, const TGswParams<TORUS>* params){
  const TLweParams<TORUS>* parlwe = params->tlwe_params;
  const int N = parlwe->N;
  const int kpl = params->kpl;
  IntPolynomial* dec = new_IntPolynomial_array(kpl,N);

  tGswTLweDecompH(dec, b, params);

  tLweClear(result, parlwe);
  for (int i = 0; i < kpl; i++)
  tLweAddMulRTo(result, &dec[i], &a->all_sample[i], parlwe);

  result->current_variance += b->current_variance; //todo + the error term?

  delete_IntPolynomial_array(kpl, dec);
}
#endif






#if defined INCLUDE_ALL || defined INCLUDE_TGSW_NOISELESS_TRIVIAL
#undef INCLUDE_TGSW_NOISELESS_TRIVIAL
/**
 * result = (0,mu)
 */
template<typename TORUS>
void TGswFunctions<TORUS>::NoiselessTrivial(TGswSample<TORUS>* result, const IntPolynomial* mu, const TGswParams<TORUS>* params){
  tGswClear(result, params);
  tGswAddMuH(result, mu, params);
}
#endif

#undef INCLUDE_ALL