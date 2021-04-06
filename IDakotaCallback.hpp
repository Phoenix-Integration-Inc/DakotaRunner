#pragma once

using namespace Phoenix::Optimization;
using namespace System;
using namespace System::Runtime::InteropServices;


namespace Phoenix {
   namespace DakotaRunner
   {
      /**
       * Status of the single evaluation or the whole optimization.
       */
      public enum class Status
      {
         None, Success, Halted, Error
      };

      /**
       * Interface used to call back from Dakota runner to OptTool algorithm.
       */
      public interface class IDakotaCallback
      {
      public:

         /**
          * Evaluates given design point.
          * @param variables design variables values of different types
          * @param results output response values
          * @return true if evaluation succeeded, false if halted
          * @throws RunFailureException if evaluation failed
          */
         bool EvaluateDesign(array<Object^>^ variables, [Out] array<double>^% results);

         /**
         * Updates the list of best designs with the specified one.
         * @param variables design variables values
         * @param results response values
         */
         void UpdateBestDesign(array<Object^>^ variables, array<double>^ results);
      };
   }
}
