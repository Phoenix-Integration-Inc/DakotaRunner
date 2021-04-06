#pragma once

#include "IDakotaCallback.hpp"

using namespace Phoenix::Optimization;
using namespace System;

#pragma unmanaged
/**
 * Signature of the callback used to evaluate a design within ModelCenter.
 */
typedef double*(__stdcall *EvaluateModelCb) (double*, size_t);
#pragma managed

namespace Phoenix { namespace DakotaRunner
{
   /**
    * Class that initializes and runs Dakota.
    */
   public ref class DakotaRunner
   {
   public:

      /**
       * Constructor. Initializes the algorithm host used to run the model.
       */
      DakotaRunner(IDakotaCallback^ algorithm);

      /**
       * Starts Dakota with the given input files and runs the optimization.
       * @param inputFileName path to input (.in) configuration file
       * @param writeRestartFile path to restart file to be written
       * @parma readRestartFile path to restart file to be read
       * @param logFile path to a log file
       * @param errFile path to error log file
       * @return true if run succeeded, false if halted
       * @throws std::exception if evaluation failed
       */
      bool virtual Run(String^ inputFileName, String^ writeRestartFile, String^ readRestartFile,
               String^ logFile, String^ errFile);
      
      /**
       * Delegate (callback) for evaluateModel.
       */
      delegate double* EvaluateModelDelegate(double* designVariables, size_t length);

   private:

      /**
       * The callback interface to Optimization Tool algorithm.
       */
      IDakotaCallback^ _algorithm;

      /**
       * Reference to the delegate passed to ModelCenterDirectInterface to ensure it stays in scope throughout its
       * lifetime.
       */
      EvaluateModelDelegate^ _evaluateDelegate;

      /**
       * Indicates if run has been halted.
       */
      bool _halted;

      /**
       * Method that evaluates the given designs within ModelCenter.
       * @param designVariables the values of all design variables, in the same order as within the OptTool
       * @param length the length of the designVariables array
       * @return array of the computed response, which must be deleted by the caller
       */
      double* _evaluateModel(double* designVariables, size_t length);
   };
}}
