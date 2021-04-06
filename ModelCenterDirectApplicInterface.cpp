#include "Stdafx.h"
#include "ModelCenterDirectApplicInterface.hpp"
#include <string>


namespace Phoenix { namespace DakotaRunner
{

   ModelCenterDirectApplicInterface::ModelCenterDirectApplicInterface(
      const Dakota::ProblemDescDB& problem_db, EvaluateModelCb evalMethodPtr) 
      : Dakota::DirectApplicInterface(problem_db)
   {
      _evaluateMethod = evalMethodPtr;
      _variableMappings = {};
   }

   ModelCenterDirectApplicInterface::~ModelCenterDirectApplicInterface()
   {
   }

   std::vector<std::tuple<int, size_t>> ModelCenterDirectApplicInterface::getVariableMappings()
   {
      return _variableMappings;
   }

   int ModelCenterDirectApplicInterface::derived_map_ac(const Dakota::String& ac_name)
   {
      if (multiProcAnalysisFlag)
      {
         throw Dakota::FunctionEvalFailure("Plug-in does not support multiprocessor analyses.");
      }

      if (ac_name != "modelcenter")
      {
         std::string err_msg("Error evaluating analysis_driver ");
         err_msg += ac_name;
         throw Dakota::FunctionEvalFailure(err_msg);
      }

      // TODO: Do we care about string variables? (xDS, xDSLabels, numADSV)
      size_t varSize = numACV + numADIV + numADRV;

      // Make an array, where each element corresponds to the variable at that index in MC, and each element is
      // a tuple of <array to look in, index in array>, with the array indexes being:
      //    [xC - 0, xDI - 1, xDR - 2]
      // The labels are in the form x0, x1, etc. So we trim off the 'x'.
      if (_variableMappings.size() == 0)
      {
         std::vector<std::tuple<int, size_t>> map(varSize);
         for (size_t i = 0; i < xCLabels.size(); ++i)
         {
            long index = std::stol(xCLabels[i].substr(1));
            map[index] = std::make_tuple(0, i);
         }
         for (size_t i = 0; i < xDILabels.size(); ++i)
         {
            long index = std::stol(xDILabels[i].substr(1));
            map[index] = std::make_tuple(1, i);
         }
         for (size_t i = 0; i < xDRLabels.size(); ++i)
         {
            long index = std::stol(xDRLabels[i].substr(1));
            map[index] = std::make_tuple(2, i);
         }
         _variableMappings = map;
      }
         
      // Combine all variable values into one array.
      double* variables = new double[varSize];
      double* results = nullptr;

      try
      {
         for (size_t i = 0; i < varSize; ++i)
         {
            switch (std::get<0>(_variableMappings[i]))
            {
            case 0:
               variables[i] = xC[static_cast<int>(std::get<1>(_variableMappings[i]))];
               break;
            case 1:
               // Implicit cast from int to real here
               variables[i] = xDI[static_cast<int>(std::get<1>(_variableMappings[i]))];
               break;
            case 2:
               variables[i] = xDR[static_cast<int>(std::get<1>(_variableMappings[i]))];
               break;
            default:
               throw Dakota::FunctionEvalFailure("Unknown variable type.");
            }
         }

         // Evaulate the design
         results = _evaluateMethod(variables, varSize);
         if (!results)
         {
            throw Dakota::FunctionEvalFailure("Error evaluating plugin analysis_driver.");
         }

         // Set the result into Dakota
         size_t out_var_act_len = fnLabels.size();
         if (out_var_act_len != numFns)
         {
            throw Dakota::FunctionEvalFailure("Mismatch in the number of responses.");
         }

         for (int i = 0; i < static_cast<int>(out_var_act_len); ++i)
         {
            // f
            if (directFnASV[i] & 1)
            {
               fnVals[i] = results[i];
            }
            // df/dx
            if (directFnASV[i] & 2)
            {
               throw Dakota::FunctionEvalFailure("Analytic gradients not supported.");
            }
            // d^2f/dx^2
            if (directFnASV[i] & 4)
            {
               throw Dakota::FunctionEvalFailure("Analytic Hessians not supported.");
            }
         }
      }
      finally
      {
         delete []results;
         delete []variables;
      }

      return 0;
   }

   void ModelCenterDirectApplicInterface::wait_local_evaluations(Dakota::PRPQueue& prp_queue)
   {
      UNREFERENCED_PARAMETER(prp_queue);
      throw Dakota::FunctionEvalFailure("Asynchronous capability (multiple threads) not installed in DirectApplicInterface.");
   }

   void ModelCenterDirectApplicInterface::test_local_evaluations(Dakota::PRPQueue& prp_queue)
   {
      UNREFERENCED_PARAMETER(prp_queue);
      throw Dakota::FunctionEvalFailure("Asynchronous capability (multiple threads) not installed in DirectApplicInterface.");
   }
}}
