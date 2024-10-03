// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME srcdIrootdict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "src/NanoAODAnalyzerrdframe.h"
#include "src/BaseAnalyser.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *NanoAODAnalyzerrdframe_Dictionary();
   static void NanoAODAnalyzerrdframe_TClassManip(TClass*);
   static void delete_NanoAODAnalyzerrdframe(void *p);
   static void deleteArray_NanoAODAnalyzerrdframe(void *p);
   static void destruct_NanoAODAnalyzerrdframe(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::NanoAODAnalyzerrdframe*)
   {
      ::NanoAODAnalyzerrdframe *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::NanoAODAnalyzerrdframe));
      static ::ROOT::TGenericClassInfo 
         instance("NanoAODAnalyzerrdframe", "NanoAODAnalyzerrdframe.h", 40,
                  typeid(::NanoAODAnalyzerrdframe), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &NanoAODAnalyzerrdframe_Dictionary, isa_proxy, 4,
                  sizeof(::NanoAODAnalyzerrdframe) );
      instance.SetDelete(&delete_NanoAODAnalyzerrdframe);
      instance.SetDeleteArray(&deleteArray_NanoAODAnalyzerrdframe);
      instance.SetDestructor(&destruct_NanoAODAnalyzerrdframe);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::NanoAODAnalyzerrdframe*)
   {
      return GenerateInitInstanceLocal((::NanoAODAnalyzerrdframe*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::NanoAODAnalyzerrdframe*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *NanoAODAnalyzerrdframe_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::NanoAODAnalyzerrdframe*)0x0)->GetClass();
      NanoAODAnalyzerrdframe_TClassManip(theClass);
   return theClass;
   }

   static void NanoAODAnalyzerrdframe_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *BaseAnalyser_Dictionary();
   static void BaseAnalyser_TClassManip(TClass*);
   static void delete_BaseAnalyser(void *p);
   static void deleteArray_BaseAnalyser(void *p);
   static void destruct_BaseAnalyser(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::BaseAnalyser*)
   {
      ::BaseAnalyser *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::BaseAnalyser));
      static ::ROOT::TGenericClassInfo 
         instance("BaseAnalyser", "BaseAnalyser.h", 14,
                  typeid(::BaseAnalyser), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &BaseAnalyser_Dictionary, isa_proxy, 4,
                  sizeof(::BaseAnalyser) );
      instance.SetDelete(&delete_BaseAnalyser);
      instance.SetDeleteArray(&deleteArray_BaseAnalyser);
      instance.SetDestructor(&destruct_BaseAnalyser);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::BaseAnalyser*)
   {
      return GenerateInitInstanceLocal((::BaseAnalyser*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::BaseAnalyser*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *BaseAnalyser_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::BaseAnalyser*)0x0)->GetClass();
      BaseAnalyser_TClassManip(theClass);
   return theClass;
   }

   static void BaseAnalyser_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrapper around operator delete
   static void delete_NanoAODAnalyzerrdframe(void *p) {
      delete ((::NanoAODAnalyzerrdframe*)p);
   }
   static void deleteArray_NanoAODAnalyzerrdframe(void *p) {
      delete [] ((::NanoAODAnalyzerrdframe*)p);
   }
   static void destruct_NanoAODAnalyzerrdframe(void *p) {
      typedef ::NanoAODAnalyzerrdframe current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::NanoAODAnalyzerrdframe

namespace ROOT {
   // Wrapper around operator delete
   static void delete_BaseAnalyser(void *p) {
      delete ((::BaseAnalyser*)p);
   }
   static void deleteArray_BaseAnalyser(void *p) {
      delete [] ((::BaseAnalyser*)p);
   }
   static void destruct_BaseAnalyser(void *p) {
      typedef ::BaseAnalyser current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::BaseAnalyser

namespace ROOT {
   static TClass *vectorlEvarinfogR_Dictionary();
   static void vectorlEvarinfogR_TClassManip(TClass*);
   static void *new_vectorlEvarinfogR(void *p = 0);
   static void *newArray_vectorlEvarinfogR(Long_t size, void *p);
   static void delete_vectorlEvarinfogR(void *p);
   static void deleteArray_vectorlEvarinfogR(void *p);
   static void destruct_vectorlEvarinfogR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<varinfo>*)
   {
      vector<varinfo> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<varinfo>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<varinfo>", -2, "vector", 389,
                  typeid(vector<varinfo>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEvarinfogR_Dictionary, isa_proxy, 0,
                  sizeof(vector<varinfo>) );
      instance.SetNew(&new_vectorlEvarinfogR);
      instance.SetNewArray(&newArray_vectorlEvarinfogR);
      instance.SetDelete(&delete_vectorlEvarinfogR);
      instance.SetDeleteArray(&deleteArray_vectorlEvarinfogR);
      instance.SetDestructor(&destruct_vectorlEvarinfogR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<varinfo> >()));

      ::ROOT::AddClassAlternate("vector<varinfo>","std::vector<varinfo, std::allocator<varinfo> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<varinfo>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEvarinfogR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<varinfo>*)0x0)->GetClass();
      vectorlEvarinfogR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEvarinfogR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEvarinfogR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<varinfo> : new vector<varinfo>;
   }
   static void *newArray_vectorlEvarinfogR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<varinfo>[nElements] : new vector<varinfo>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEvarinfogR(void *p) {
      delete ((vector<varinfo>*)p);
   }
   static void deleteArray_vectorlEvarinfogR(void *p) {
      delete [] ((vector<varinfo>*)p);
   }
   static void destruct_vectorlEvarinfogR(void *p) {
      typedef vector<varinfo> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<varinfo>

namespace ROOT {
   static TClass *vectorlEstringgR_Dictionary();
   static void vectorlEstringgR_TClassManip(TClass*);
   static void *new_vectorlEstringgR(void *p = 0);
   static void *newArray_vectorlEstringgR(Long_t size, void *p);
   static void delete_vectorlEstringgR(void *p);
   static void deleteArray_vectorlEstringgR(void *p);
   static void destruct_vectorlEstringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<string>*)
   {
      vector<string> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<string>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<string>", -2, "vector", 389,
                  typeid(vector<string>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEstringgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<string>) );
      instance.SetNew(&new_vectorlEstringgR);
      instance.SetNewArray(&newArray_vectorlEstringgR);
      instance.SetDelete(&delete_vectorlEstringgR);
      instance.SetDeleteArray(&deleteArray_vectorlEstringgR);
      instance.SetDestructor(&destruct_vectorlEstringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<string> >()));

      ::ROOT::AddClassAlternate("vector<string>","std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<string>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEstringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<string>*)0x0)->GetClass();
      vectorlEstringgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEstringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEstringgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<string> : new vector<string>;
   }
   static void *newArray_vectorlEstringgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<string>[nElements] : new vector<string>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEstringgR(void *p) {
      delete ((vector<string>*)p);
   }
   static void deleteArray_vectorlEstringgR(void *p) {
      delete [] ((vector<string>*)p);
   }
   static void destruct_vectorlEstringgR(void *p) {
      typedef vector<string> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<string>

namespace ROOT {
   static TClass *vectorlEhist2dinfogR_Dictionary();
   static void vectorlEhist2dinfogR_TClassManip(TClass*);
   static void *new_vectorlEhist2dinfogR(void *p = 0);
   static void *newArray_vectorlEhist2dinfogR(Long_t size, void *p);
   static void delete_vectorlEhist2dinfogR(void *p);
   static void deleteArray_vectorlEhist2dinfogR(void *p);
   static void destruct_vectorlEhist2dinfogR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<hist2dinfo>*)
   {
      vector<hist2dinfo> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<hist2dinfo>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<hist2dinfo>", -2, "vector", 389,
                  typeid(vector<hist2dinfo>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEhist2dinfogR_Dictionary, isa_proxy, 0,
                  sizeof(vector<hist2dinfo>) );
      instance.SetNew(&new_vectorlEhist2dinfogR);
      instance.SetNewArray(&newArray_vectorlEhist2dinfogR);
      instance.SetDelete(&delete_vectorlEhist2dinfogR);
      instance.SetDeleteArray(&deleteArray_vectorlEhist2dinfogR);
      instance.SetDestructor(&destruct_vectorlEhist2dinfogR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<hist2dinfo> >()));

      ::ROOT::AddClassAlternate("vector<hist2dinfo>","std::vector<hist2dinfo, std::allocator<hist2dinfo> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<hist2dinfo>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEhist2dinfogR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<hist2dinfo>*)0x0)->GetClass();
      vectorlEhist2dinfogR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEhist2dinfogR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEhist2dinfogR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<hist2dinfo> : new vector<hist2dinfo>;
   }
   static void *newArray_vectorlEhist2dinfogR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<hist2dinfo>[nElements] : new vector<hist2dinfo>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEhist2dinfogR(void *p) {
      delete ((vector<hist2dinfo>*)p);
   }
   static void deleteArray_vectorlEhist2dinfogR(void *p) {
      delete [] ((vector<hist2dinfo>*)p);
   }
   static void destruct_vectorlEhist2dinfogR(void *p) {
      typedef vector<hist2dinfo> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<hist2dinfo>

namespace ROOT {
   static TClass *vectorlEhist1dinfogR_Dictionary();
   static void vectorlEhist1dinfogR_TClassManip(TClass*);
   static void *new_vectorlEhist1dinfogR(void *p = 0);
   static void *newArray_vectorlEhist1dinfogR(Long_t size, void *p);
   static void delete_vectorlEhist1dinfogR(void *p);
   static void deleteArray_vectorlEhist1dinfogR(void *p);
   static void destruct_vectorlEhist1dinfogR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<hist1dinfo>*)
   {
      vector<hist1dinfo> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<hist1dinfo>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<hist1dinfo>", -2, "vector", 389,
                  typeid(vector<hist1dinfo>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEhist1dinfogR_Dictionary, isa_proxy, 0,
                  sizeof(vector<hist1dinfo>) );
      instance.SetNew(&new_vectorlEhist1dinfogR);
      instance.SetNewArray(&newArray_vectorlEhist1dinfogR);
      instance.SetDelete(&delete_vectorlEhist1dinfogR);
      instance.SetDeleteArray(&deleteArray_vectorlEhist1dinfogR);
      instance.SetDestructor(&destruct_vectorlEhist1dinfogR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<hist1dinfo> >()));

      ::ROOT::AddClassAlternate("vector<hist1dinfo>","std::vector<hist1dinfo, std::allocator<hist1dinfo> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<hist1dinfo>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEhist1dinfogR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<hist1dinfo>*)0x0)->GetClass();
      vectorlEhist1dinfogR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEhist1dinfogR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEhist1dinfogR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<hist1dinfo> : new vector<hist1dinfo>;
   }
   static void *newArray_vectorlEhist1dinfogR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<hist1dinfo>[nElements] : new vector<hist1dinfo>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEhist1dinfogR(void *p) {
      delete ((vector<hist1dinfo>*)p);
   }
   static void deleteArray_vectorlEhist1dinfogR(void *p) {
      delete [] ((vector<hist1dinfo>*)p);
   }
   static void destruct_vectorlEhist1dinfogR(void *p) {
      typedef vector<hist1dinfo> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<hist1dinfo>

namespace ROOT {
   static TClass *vectorlEcutinfogR_Dictionary();
   static void vectorlEcutinfogR_TClassManip(TClass*);
   static void *new_vectorlEcutinfogR(void *p = 0);
   static void *newArray_vectorlEcutinfogR(Long_t size, void *p);
   static void delete_vectorlEcutinfogR(void *p);
   static void deleteArray_vectorlEcutinfogR(void *p);
   static void destruct_vectorlEcutinfogR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<cutinfo>*)
   {
      vector<cutinfo> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<cutinfo>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<cutinfo>", -2, "vector", 389,
                  typeid(vector<cutinfo>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEcutinfogR_Dictionary, isa_proxy, 0,
                  sizeof(vector<cutinfo>) );
      instance.SetNew(&new_vectorlEcutinfogR);
      instance.SetNewArray(&newArray_vectorlEcutinfogR);
      instance.SetDelete(&delete_vectorlEcutinfogR);
      instance.SetDeleteArray(&deleteArray_vectorlEcutinfogR);
      instance.SetDestructor(&destruct_vectorlEcutinfogR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<cutinfo> >()));

      ::ROOT::AddClassAlternate("vector<cutinfo>","std::vector<cutinfo, std::allocator<cutinfo> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<cutinfo>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEcutinfogR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<cutinfo>*)0x0)->GetClass();
      vectorlEcutinfogR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEcutinfogR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEcutinfogR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<cutinfo> : new vector<cutinfo>;
   }
   static void *newArray_vectorlEcutinfogR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<cutinfo>[nElements] : new vector<cutinfo>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEcutinfogR(void *p) {
      delete ((vector<cutinfo>*)p);
   }
   static void deleteArray_vectorlEcutinfogR(void *p) {
      delete [] ((vector<cutinfo>*)p);
   }
   static void destruct_vectorlEcutinfogR(void *p) {
      typedef vector<cutinfo> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<cutinfo>

namespace ROOT {
   static TClass *vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR_Dictionary();
   static void vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR_TClassManip(TClass*);
   static void *new_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p = 0);
   static void *newArray_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(Long_t size, void *p);
   static void delete_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p);
   static void deleteArray_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p);
   static void destruct_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >*)
   {
      vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >));
      static ::ROOT::TGenericClassInfo 
         instance("vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >", -2, "vector", 389,
                  typeid(vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >) );
      instance.SetNew(&new_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR);
      instance.SetNewArray(&newArray_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR);
      instance.SetDelete(&delete_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR);
      instance.SetDeleteArray(&deleteArray_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR);
      instance.SetDestructor(&destruct_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > > >()));

      ::ROOT::AddClassAlternate("vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >","std::vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> >, std::allocator<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >*)0x0)->GetClass();
      vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > > : new vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >;
   }
   static void *newArray_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >[nElements] : new vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p) {
      delete ((vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >*)p);
   }
   static void deleteArray_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p) {
      delete [] ((vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >*)p);
   }
   static void destruct_vectorlEROOTcLcLMathcLcLLorentzVectorlEROOTcLcLMathcLcLPtEtaPhiM4DlEdoublegRsPgRsPgR(void *p) {
      typedef vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > >

namespace ROOT {
   static TClass *maplEstringcOvectorlEstringgRsPgR_Dictionary();
   static void maplEstringcOvectorlEstringgRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOvectorlEstringgRsPgR(void *p = 0);
   static void *newArray_maplEstringcOvectorlEstringgRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOvectorlEstringgRsPgR(void *p);
   static void deleteArray_maplEstringcOvectorlEstringgRsPgR(void *p);
   static void destruct_maplEstringcOvectorlEstringgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,vector<string> >*)
   {
      map<string,vector<string> > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,vector<string> >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,vector<string> >", -2, "map", 100,
                  typeid(map<string,vector<string> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOvectorlEstringgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,vector<string> >) );
      instance.SetNew(&new_maplEstringcOvectorlEstringgRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOvectorlEstringgRsPgR);
      instance.SetDelete(&delete_maplEstringcOvectorlEstringgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOvectorlEstringgRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOvectorlEstringgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,vector<string> > >()));

      ::ROOT::AddClassAlternate("map<string,vector<string> >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,vector<string> >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOvectorlEstringgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,vector<string> >*)0x0)->GetClass();
      maplEstringcOvectorlEstringgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOvectorlEstringgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOvectorlEstringgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,vector<string> > : new map<string,vector<string> >;
   }
   static void *newArray_maplEstringcOvectorlEstringgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,vector<string> >[nElements] : new map<string,vector<string> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOvectorlEstringgRsPgR(void *p) {
      delete ((map<string,vector<string> >*)p);
   }
   static void deleteArray_maplEstringcOvectorlEstringgRsPgR(void *p) {
      delete [] ((map<string,vector<string> >*)p);
   }
   static void destruct_maplEstringcOvectorlEstringgRsPgR(void *p) {
      typedef map<string,vector<string> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,vector<string> >

namespace ROOT {
   static TClass *maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR_Dictionary();
   static void maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p = 0);
   static void *newArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p);
   static void deleteArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p);
   static void destruct_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,ROOT::RDF::RResultPtr<TH2D> >*)
   {
      map<string,ROOT::RDF::RResultPtr<TH2D> > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,ROOT::RDF::RResultPtr<TH2D> >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,ROOT::RDF::RResultPtr<TH2D> >", -2, "map", 100,
                  typeid(map<string,ROOT::RDF::RResultPtr<TH2D> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,ROOT::RDF::RResultPtr<TH2D> >) );
      instance.SetNew(&new_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR);
      instance.SetDelete(&delete_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,ROOT::RDF::RResultPtr<TH2D> > >()));

      ::ROOT::AddClassAlternate("map<string,ROOT::RDF::RResultPtr<TH2D> >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, ROOT::RDF::RResultPtr<TH2D>, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, ROOT::RDF::RResultPtr<TH2D> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,ROOT::RDF::RResultPtr<TH2D> >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,ROOT::RDF::RResultPtr<TH2D> >*)0x0)->GetClass();
      maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,ROOT::RDF::RResultPtr<TH2D> > : new map<string,ROOT::RDF::RResultPtr<TH2D> >;
   }
   static void *newArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,ROOT::RDF::RResultPtr<TH2D> >[nElements] : new map<string,ROOT::RDF::RResultPtr<TH2D> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p) {
      delete ((map<string,ROOT::RDF::RResultPtr<TH2D> >*)p);
   }
   static void deleteArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p) {
      delete [] ((map<string,ROOT::RDF::RResultPtr<TH2D> >*)p);
   }
   static void destruct_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH2DgRsPgR(void *p) {
      typedef map<string,ROOT::RDF::RResultPtr<TH2D> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,ROOT::RDF::RResultPtr<TH2D> >

namespace ROOT {
   static TClass *maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR_Dictionary();
   static void maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p = 0);
   static void *newArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p);
   static void deleteArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p);
   static void destruct_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,ROOT::RDF::RResultPtr<TH1D> >*)
   {
      map<string,ROOT::RDF::RResultPtr<TH1D> > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,ROOT::RDF::RResultPtr<TH1D> >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,ROOT::RDF::RResultPtr<TH1D> >", -2, "map", 100,
                  typeid(map<string,ROOT::RDF::RResultPtr<TH1D> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,ROOT::RDF::RResultPtr<TH1D> >) );
      instance.SetNew(&new_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR);
      instance.SetDelete(&delete_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,ROOT::RDF::RResultPtr<TH1D> > >()));

      ::ROOT::AddClassAlternate("map<string,ROOT::RDF::RResultPtr<TH1D> >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, ROOT::RDF::RResultPtr<TH1D>, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, ROOT::RDF::RResultPtr<TH1D> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,ROOT::RDF::RResultPtr<TH1D> >*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,ROOT::RDF::RResultPtr<TH1D> >*)0x0)->GetClass();
      maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,ROOT::RDF::RResultPtr<TH1D> > : new map<string,ROOT::RDF::RResultPtr<TH1D> >;
   }
   static void *newArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,ROOT::RDF::RResultPtr<TH1D> >[nElements] : new map<string,ROOT::RDF::RResultPtr<TH1D> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p) {
      delete ((map<string,ROOT::RDF::RResultPtr<TH1D> >*)p);
   }
   static void deleteArray_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p) {
      delete [] ((map<string,ROOT::RDF::RResultPtr<TH1D> >*)p);
   }
   static void destruct_maplEstringcOROOTcLcLRDFcLcLRResultPtrlETH1DgRsPgR(void *p) {
      typedef map<string,ROOT::RDF::RResultPtr<TH1D> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,ROOT::RDF::RResultPtr<TH1D> >

namespace {
  void TriggerDictionaryInitialization_rootdict_Impl() {
    static const char* headers[] = {
"src/NanoAODAnalyzerrdframe.h",
"src/BaseAnalyser.h",
0
    };
    static const char* includePaths[] = {
"/cvmfs/cms.cern.ch/el8_amd64_gcc10/external/py3-correctionlib/2.1.0-bc1f1a24613552be911056d8eb959866/lib/python3.9/site-packages/correctionlib/include",
"src",
"/cvmfs/cms.cern.ch/el8_amd64_gcc10/lcg/root/6.24.07-6b24df5a7040a677b8f0d27957c7cb74/include/",
"/uscms_data/d3/vsinha/CMSSW_12_3_4/src/fly/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "rootdict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
namespace ROOT{namespace Math{template <typename T> class __attribute__((annotate("$clingAutoload$Math/Vector4Dfwd.h")))  __attribute__((annotate("$clingAutoload$src/NanoAODAnalyzerrdframe.h")))  PtEtaPhiM4D;
}}
namespace ROOT{namespace Math{template <class CoordSystem> class __attribute__((annotate("$clingAutoload$Math/Vector4Dfwd.h")))  __attribute__((annotate("$clingAutoload$src/NanoAODAnalyzerrdframe.h")))  LorentzVector;
}}
namespace std{template <typename _Tp> class __attribute__((annotate("$clingAutoload$bits/allocator.h")))  __attribute__((annotate("$clingAutoload$string")))  allocator;
}
class __attribute__((annotate("$clingAutoload$src/NanoAODAnalyzerrdframe.h")))  NanoAODAnalyzerrdframe;
class __attribute__((annotate("$clingAutoload$src/BaseAnalyser.h")))  BaseAnalyser;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "rootdict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "src/NanoAODAnalyzerrdframe.h"
#include "src/BaseAnalyser.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"BaseAnalyser", payloadCode, "@",
"NanoAODAnalyzerrdframe", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("rootdict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_rootdict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_rootdict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_rootdict() {
  TriggerDictionaryInitialization_rootdict_Impl();
}
