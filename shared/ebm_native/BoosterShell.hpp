// Copyright (c) 2018 Microsoft Corporation
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

#ifndef BOOSTER_SHELL_HPP
#define BOOSTER_SHELL_HPP

#include <stdlib.h> // free
#include <stddef.h> // size_t, ptrdiff_t

#include "ebm_native.h"
#include "logging.h"
#include "zones.h"

#include "ebm_internal.hpp"

namespace DEFINED_ZONE_NAME {
#ifndef DEFINED_ZONE_NAME
#error DEFINED_ZONE_NAME must be defined
#endif // DEFINED_ZONE_NAME

struct BinBase;
class BoosterCore;

template<bool bClassification>
struct SplitPosition;

template<bool bClassification>
struct TreeNode;

class BoosterShell final {
   static constexpr size_t k_handleVerificationOk = 10995; // random 15 bit number
   static constexpr size_t k_handleVerificationFreed = 25073; // random 15 bit number
   size_t m_handleVerification; // this needs to be at the top and make it pointer sized to keep best alignment

   BoosterCore * m_pBoosterCore;
   size_t m_iTerm;

   Tensor * m_pTermUpdate;
   Tensor * m_pInnerTermUpdate;

   // TODO: try to merge some of this memory so that we get more CPU cache residency
   BinBase * m_aBinsFastTemp;
   BinBase * m_aBinsBig;

   // TODO: I think this can share memory with m_aBinsFastTemp since the GradientPair always contains a FloatFast, and it always contains enough for the multiclass scores in the first bin, and we always have at least 1 bin, right?
   FloatFast * m_aMulticlassMidwayTemp;

   void * m_aTreeNodesTemp;
   void * m_aSplitPositionsTemp;

#ifndef NDEBUG
   const unsigned char * m_pBinsFastEndDebug;
   const unsigned char * m_pBinsBigEndDebug;
#endif // NDEBUG

public:

   BoosterShell() = default; // preserve our POD status
   ~BoosterShell() = default; // preserve our POD status
   void * operator new(std::size_t) = delete; // we only use malloc/free in this library
   void operator delete (void *) = delete; // we only use malloc/free in this library

   constexpr static size_t k_illegalTermIndex = size_t { static_cast<size_t>(ptrdiff_t { -1 }) };

   INLINE_ALWAYS void InitializeUnfailing() {
      m_handleVerification = k_handleVerificationOk;
      m_pBoosterCore = nullptr;
      m_iTerm = k_illegalTermIndex;
      m_pTermUpdate = nullptr;
      m_pInnerTermUpdate = nullptr;
      m_aBinsFastTemp = nullptr;
      m_aBinsBig = nullptr;
      m_aMulticlassMidwayTemp = nullptr;
      m_aTreeNodesTemp = nullptr;
      m_aSplitPositionsTemp = nullptr;
   }

   static void Free(BoosterShell * const pBoosterShell);
   static BoosterShell * Create();
   ErrorEbm FillAllocations();

   static INLINE_ALWAYS BoosterShell * GetBoosterShellFromHandle(const BoosterHandle boosterHandle) {
      if(nullptr == boosterHandle) {
         LOG_0(Trace_Error, "ERROR GetBoosterShellFromHandle null boosterHandle");
         return nullptr;
      }
      BoosterShell * const pBoosterShell = reinterpret_cast<BoosterShell *>(boosterHandle);
      if(k_handleVerificationOk == pBoosterShell->m_handleVerification) {
         return pBoosterShell;
      }
      if(k_handleVerificationFreed == pBoosterShell->m_handleVerification) {
         LOG_0(Trace_Error, "ERROR GetBoosterShellFromHandle attempt to use freed BoosterHandle");
      } else {
         LOG_0(Trace_Error, "ERROR GetBoosterShellFromHandle attempt to use invalid BoosterHandle");
      }
      return nullptr;
   }
   INLINE_ALWAYS BoosterHandle GetHandle() {
      return reinterpret_cast<BoosterHandle>(this);
   }

   INLINE_ALWAYS BoosterCore * GetBoosterCore() {
      EBM_ASSERT(nullptr != m_pBoosterCore);
      return m_pBoosterCore;
   }

   INLINE_ALWAYS void SetBoosterCore(BoosterCore * const pBoosterCore) {
      EBM_ASSERT(nullptr != pBoosterCore);
      EBM_ASSERT(nullptr == m_pBoosterCore); // only set it once
      m_pBoosterCore = pBoosterCore;
   }

   INLINE_ALWAYS size_t GetTermIndex() {
      return m_iTerm;
   }

   INLINE_ALWAYS void SetTermIndex(const size_t iTerm) {
      m_iTerm = iTerm;
   }

   INLINE_ALWAYS Tensor * GetTermUpdate() {
      return m_pTermUpdate;
   }

   INLINE_ALWAYS Tensor * GetInnerTermUpdate() {
      return m_pInnerTermUpdate;
   }

   INLINE_ALWAYS BinBase * GetBinBaseFast() {
      // call this if the bins were already allocated and we just need the pointer
      return m_aBinsFastTemp;
   }

   INLINE_ALWAYS BinBase * GetBinBaseBig() {
      // call this if the bins were already allocated and we just need the pointer
      return m_aBinsBig;
   }

   INLINE_ALWAYS FloatFast * GetMulticlassMidwayTemp() {
      return m_aMulticlassMidwayTemp;
   }

   template<bool bClassification>
   INLINE_ALWAYS TreeNode<bClassification> * GetTreeNodesTemp() {
      return static_cast<TreeNode<bClassification> *>(m_aTreeNodesTemp);
   }

   template<bool bClassification>
   INLINE_ALWAYS SplitPosition<bClassification> * GetSplitPositionsTemp() {
      return static_cast<SplitPosition<bClassification> *>(m_aSplitPositionsTemp);
   }


#ifndef NDEBUG
   INLINE_ALWAYS const unsigned char * GetBinsFastEndDebug() const {
      return m_pBinsFastEndDebug;
   }

   INLINE_ALWAYS void SetBinsFastEndDebug(const unsigned char * const pBinsFastEndDebug) {
      m_pBinsFastEndDebug = pBinsFastEndDebug;
   }

   INLINE_ALWAYS const unsigned char * GetBinsBigEndDebug() const {
      return m_pBinsBigEndDebug;
   }

   INLINE_ALWAYS void SetBinsBigEndDebug(const unsigned char * const pBinsBigEndDebug) {
      m_pBinsBigEndDebug = pBinsBigEndDebug;
   }
#endif // NDEBUG
};
static_assert(std::is_standard_layout<BoosterShell>::value,
   "We use the struct hack in several places, so disallow non-standard_layout types in general");
static_assert(std::is_trivial<BoosterShell>::value,
   "We use memcpy in several places, so disallow non-trivial types in general");
static_assert(std::is_pod<BoosterShell>::value,
   "We use a lot of C constructs, so disallow non-POD types in general");

} // DEFINED_ZONE_NAME

#endif // BOOSTER_SHELL_HPP
