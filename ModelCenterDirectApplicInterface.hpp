#pragma once

#include <functional>
#pragma warning(push, 0)
#include <DirectApplicInterface.hpp>
#pragma warning(pop)
#include "DakotaRunner.hpp"

namespace Phoenix { namespace DakotaRunner
{
   /**
    * This class makes calls to OptTool algorithms on each design evaluation request from Dakota.
    */
   class ModelCenterDirectApplicInterface : public Dakota::DirectApplicInterface
   {
   public:

      /**
       * Constructor
       * @param problem_db the database containing the problem definintion
       * @param evalMethodPtr callback to the method used to evaluate designs within ModelCenter
       */
      ModelCenterDirectApplicInterface(const Dakota::ProblemDescDB& problem_db, EvaluateModelCb evalMethodPtr);

      /**
       * Destructor
       */
      ~ModelCenterDirectApplicInterface();

      /**
       * Gets the map of OptTool variable index to [internal Dakota structure id, index in structure].
       * The Dakota structure ids are numbered as:  [Continuous vars - 0, Integer vars - 1, Real vars - 2].
       * @return the map of variables to internal dakota data structures.
       */
      std::vector<std::tuple<int, size_t>> getVariableMappings();

   protected:

      /**
       * execute an analysis code portion of a direct evaluation invocation
       */
      int derived_map_ac(const Dakota::String& ac_name);

      /**
       * Copy of base implementation to avoid linker error.
       */
      void wait_local_evaluations(Dakota::PRPQueue& prp_queue);
      
      /**
       * Copy of base implementation to avoid linker error.
       */
      void test_local_evaluations(Dakota::PRPQueue& prp_queue);

   private:

      /**
       * The method used to evaluate designs within ModelCenter.
       */
      EvaluateModelCb _evaluateMethod;

      /**
       * The map of OptTool variable index to [internal Dakota structure id, index in structure].
       * The Dakota structure ids are numbered as:  [Continuous vars - 0, Integer vars - 1, Real vars - 2].
       * These arrays are defined in DirectApplicInterface.h (xC, xDI, xDR)
       */
      std::vector<std::tuple<int, size_t>> _variableMappings;
   };
}}
