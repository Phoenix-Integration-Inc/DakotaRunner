#include "stdafx.h"
#include "DakotaRunner.hpp"

#pragma unmanaged
#include "ModelCenterDirectApplicInterface.hpp"
#include <memory>
#pragma warning(push, 0)
#include <dakota_dll_api.h>
#include <dakota_global_defs.hpp>
#include <DakotaInterface.hpp>
#include <DakotaModel.hpp>
#include <LibraryEnvironment.hpp>
#include <ProblemDescDB.hpp>
#include <ProgramOptions.hpp>
#pragma warning(pop)
#pragma managed

#include <msclr\marshal_cppstd.h>

using namespace Runtime::InteropServices;

namespace Phoenix { namespace DakotaRunner
{
   DakotaRunner::DakotaRunner(IDakotaCallback^ algorithm)
   {
      _algorithm = algorithm;
      _evaluateDelegate = nullptr;
      _halted = false;
   }
   
   bool DakotaRunner::Run(
      String^ inputFileName, String^ writeRestartFile, String^ readRestartFile,
      String^ logFile, String^ errFile)
   {
      std::string input = msclr::interop::marshal_as<std::string>(inputFileName);
      // NOTE: Must use forward slashes in path
      std::replace(input.begin(), input.end(), '\\', '/');

      _halted = false;

      // Initialize the input, ouput, and restart files
      Dakota::ProgramOptions opts;

      // Do not abort/exit the application on fatal errors - throw an exception.
      opts.exit_mode("throw");
      //  Do not echo input file into log.
      opts.echo_input(false);

      opts.input_file(input);

      if (!String::IsNullOrEmpty(logFile))
      {
         std::string path = msclr::interop::marshal_as<std::string>(logFile);
         std::replace(path.begin(), path.end(), '\\', '/');
         opts.output_file(path);
      }

      if (!String::IsNullOrEmpty(errFile))
      {
         std::string path = msclr::interop::marshal_as<std::string>(errFile);
         std::replace(path.begin(), path.end(), '\\', '/');
         opts.error_file(path);
      }

      if (!String::IsNullOrEmpty(writeRestartFile))
      {
         std::string path = msclr::interop::marshal_as<std::string>(writeRestartFile);
         std::replace(path.begin(), path.end(), '\\', '/');
         opts.write_restart_file(path);
      }

      if (!String::IsNullOrEmpty(readRestartFile))
      {
         std::string path = msclr::interop::marshal_as<std::string>(readRestartFile);
         std::replace(path.begin(), path.end(), '\\', '/');
         opts.read_restart_file(path);
      }

      // Initialize the environment. Will crash if the input file has invalid parameters.
      Dakota::LibraryEnvironment env(opts);

      std::string modelType(""); // empty string will match any model type
      std::string interfaceType("direct");
      std::string analysisDriver("modelcenter");

      Dakota::ProblemDescDB& problem_db = env.problem_description_db();

      _evaluateDelegate = gcnew EvaluateModelDelegate(this, &DakotaRunner::_evaluateModel);
      IntPtr evaluatePtr = Marshal::GetFunctionPointerForDelegate(_evaluateDelegate);
      EvaluateModelCb cb = static_cast<EvaluateModelCb>(evaluatePtr.ToPointer());

      // Create the interface. Deletion is handled by Dakota.
      auto applicationInterface = new ModelCenterDirectApplicInterface(problem_db, cb);

      bool plugged_in = env.plugin_interface(
         modelType, interfaceType, analysisDriver, applicationInterface);
      if (!plugged_in)
      {
         throw gcnew AlgorithmException(-1, "Failed to plug in ModelCenterDirectApplicInterface.");
      }
      
      try
      {
         // Execute the optimization using the set up environment
         env.execute();
      }
      catch (std::exception& e)
      {
         if (! _halted)
         {
            throw gcnew AlgorithmException(-1, gcnew String(e.what()));
         }
      }

      if (_halted)
      {
         return false;
      }

      // Fill an array with the values of the design variables at the best design.
      // See documentation on ModelCenterDirectApplicInterface::_variableMappings.
      auto mappings = applicationInterface->getVariableMappings();
      if (mappings.size() > 0)
      {
         auto variables = gcnew array<Object^>(static_cast<int>(mappings.size()));
         auto variablesResults = env.variables_results();

         for (int i = 0; i < static_cast<int>(mappings.size()); ++i)
         {
            int index = static_cast<int>(std::get<1>(mappings[i]));
            switch (std::get<0>(mappings[i]))
            {
            case 0:
               variables[i] = Convert::ToDouble(variablesResults.all_continuous_variables()[index]);
               break;
            case 1:
               variables[i] = Convert::ToInt32(variablesResults.all_discrete_int_variables()[index]);
               break;
            case 2:
               variables[i] = Convert::ToDouble(variablesResults.all_discrete_real_variables()[index]);
               break;
            default:
               throw gcnew AlgorithmException(-1, "Unknown variable type.");
            }
         }

         auto functionValues = env.response_results().function_values();
         auto results = gcnew array<double>(functionValues.length());

         for (int i = 0; i < results->Length; ++i)
         {
            results[i] = functionValues[i];
         }

         // Update the best design within the OptTool.
         _algorithm->UpdateBestDesign(variables, results);
      }
      else
      {
         throw gcnew AlgorithmException(-1, "Algorithm returned no results.");
      }

      // TODO: Can we determine if the algorithm failed to converge/failed in some other way from here?

      return true;
   }

   double* DakotaRunner::_evaluateModel(double* designVariables, size_t length)
   {
      // Copy the unmanaged array to a managed array
      auto managedArray = gcnew array<Object^>(static_cast<int>(length));
      array<double>^ managedResults = nullptr;

      for (int i = 0; i < static_cast<int>(length); ++i)
      {
         managedArray[i] = designVariables[i];
      }

      // Run the evaluation
      if (_algorithm->EvaluateDesign(managedArray, managedResults))
      {
         double* results = new double[managedResults->Length];
         for (int i = 0; i < managedResults->Length; ++i)
         {
            results[i] = managedResults[i];
         }

         return results;
      }
      else
      {
         _halted = true;
         throw Dakota::FunctionEvalFailure("Evaluation halted.");
      }
   }
}}
