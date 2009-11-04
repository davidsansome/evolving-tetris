#ifndef ENGINE_H
#define ENGINE_H

#include <QSize>

#include "population.h"

class Engine {
 public:
  Engine();

  void Run();

 private:
  void UpdateFitness();
  static const Individual& FittestOf(const Individual& one, const Individual& two);

  Population pop_;

  static const QSize kBoardSize;
  static const int kPopulationSize;
  static const int kGamesToRun;
  static const int kMaxGenerations;

  // Functor for using QtConcurrentMap with object pointers
  template <typename T, typename C>
  class PointerMemberFunctionWrapper
  {
   public:
    typedef T (C::*FunctionPointerType)();
    typedef T result_type;
    inline PointerMemberFunctionWrapper(FunctionPointerType _functionPointer)
    :functionPointer(_functionPointer) { }

    inline T operator()(C* c)
    {
        return (c->*functionPointer)();
    }

   private:
    FunctionPointerType functionPointer;
  };
};

#endif // ENGINE_H
