// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the MICM class, which represents a multi-component reactive transport model.
// It also includes functions for creating and deleting MICM instances with c bindings.
#pragma once

#include <musica/util.hpp>

#include <micm/configure/solver_config.hpp>
#include <micm/process/process_set.hpp>
#include <micm/solver/rosenbrock.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver.hpp>
#include <micm/solver/solver_builder.hpp>
#include <micm/util/matrix.hpp>
#include <micm/util/sparse_matrix_vector_ordering.hpp>
#include <micm/util/vector_matrix.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifndef MICM_VECTOR_MATRIX_SIZE
  #define MICM_VECTOR_MATRIX_SIZE 1
#endif

namespace musica
{

  class MICM;

#ifdef __cplusplus
  extern "C"
  {
#endif
    /// @brief Types of MICM solver
    enum MICMSolver
    {
      Rosenbrock = 1,              // Vector-ordered Rosenbrock solver
      RosenbrockStandardOrder,     // Standard-ordered Rosenbrock solver
      BackwardEuler,               // Vector-ordered BackwardEuler solver
      BackwardEulerStandardOrder,  // Standard-ordered BackwardEuler solver
    };

    struct SolverResultStats
    {
      /// @brief The number of forcing function calls
      int64_t function_calls_{};
      /// @brief The number of jacobian function calls
      int64_t jacobian_updates_{};
      /// @brief The total number of internal time steps taken
      int64_t number_of_steps_{};
      /// @brief The number of accepted integrations
      int64_t accepted_{};
      /// @brief The number of rejected integrations
      int64_t rejected_{};
      /// @brief The number of LU decompositions
      int64_t decompositions_{};
      /// @brief The number of linear solves
      int64_t solves_{};
      /// @brief The number of times a singular matrix is detected.
      int64_t singular_{};
      /// @brief The final time the solver iterated to
      double final_time_{};
      /// @brief The final state the solver was in

      SolverResultStats()
          : function_calls_(0),
            jacobian_updates_(0),
            number_of_steps_(0),
            accepted_(0),
            rejected_(0),
            decompositions_(0),
            solves_(0),
            singular_(0),
            final_time_(0.0)
      {
      }

      SolverResultStats(
          int64_t func_calls,
          int64_t jacobian,
          int64_t num_steps,
          int64_t accepted,
          int64_t rejected,
          int64_t decompositions,
          int64_t solves,
          int64_t singular,
          double final_time)
          : function_calls_(func_calls),
            jacobian_updates_(jacobian),
            number_of_steps_(num_steps),
            accepted_(accepted),
            rejected_(rejected),
            decompositions_(decompositions),
            solves_(solves),
            singular_(singular),
            final_time_(final_time)
      {
      }
    };

    /// @brief Create a MICM object by specifying solver type to use
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param solver_type Type of MICMSolver
    /// @param num_grid_cells Number of grid cells
    /// @param error Error struct to indicate success or failure
    MICM *CreateMicm(const char *config_path, MICMSolver solver_type, int num_grid_cells, Error *error);

    /// @brief Deletes a MICM object
    /// @param micm Pointer to MICM object
    /// @param error Error struct to indicate success or failure
    void DeleteMicm(const MICM *micm, Error *error);

    /// @brief Solve the system
    /// @param micm Pointer to MICM object
    /// @param time_step Time [s] to advance the state by
    /// @param temperature Temperature [grid cell] (K)
    /// @param pressure Pressure [grid cell] (Pa)
    /// @param air_density Air density [grid cell] (mol m-3)
    /// @param concentrations Array of species' concentrations [grid cell][species] (mol m-3)
    /// @param custom_rate_parameters Array of custom rate parameters [grid cell][parameter] (various units)
    /// @param solver_state State of the solver
    /// @param solver_stats Statistics of the solver
    /// @param error Error struct to indicate success or failure
    void MicmSolve(
        MICM *micm,
        double time_step,
        double *temperature,
        double *pressure,
        double *air_density,
        double *concentrations,
        double *custom_rate_parameters,
        String *solver_state,
        SolverResultStats *solver_stats,
        Error *error);

    /// @brief Get the MICM version
    /// @return MICM version
    String MicmVersion();

    /// @brief Get the ordering of species
    /// @param micm Pointer to MICM object
    /// @param error Error struct to indicate success or failure
    /// @return Array of species' name-index pairs
    Mappings GetSpeciesOrdering(MICM *micm, Error *error);

    /// @brief Get the ordering of user-defined reaction rates
    /// @param micm Pointer to MICM object
    /// @param error Error struct to indicate success or failure
    /// @return Array of reaction rate name-index pairs
    Mappings GetUserDefinedReactionRatesOrdering(MICM *micm, Error *error);

    /// @brief Get a property for a chemical species
    /// @param micm Pointer to MICM object
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @param error Error struct to indicate success or failure
    /// @return Value of the property
    String GetSpeciesPropertyString(MICM *micm, const char *species_name, const char *property_name, Error *error);
    double GetSpeciesPropertyDouble(MICM *micm, const char *species_name, const char *property_name, Error *error);
    int GetSpeciesPropertyInt(MICM *micm, const char *species_name, const char *property_name, Error *error);
    bool GetSpeciesPropertyBool(MICM *micm, const char *species_name, const char *property_name, Error *error);
#ifdef __cplusplus
  }
#endif

  class MICM
  {
   public:
    /// @brief Create a Rosenbrock solver of vector-ordered matrix type by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    void CreateRosenbrock(const std::string &config_path, Error *error);

    /// @brief Create a Rosenbrock solver of standard-ordered matrix type by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    void CreateRosenbrockStandardOrder(const std::string &config_path, Error *error);

    /// @brief Create a BackwardEuler solver of vector-ordered matrix type by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    void CreateBackwardEuler(const std::string &config_path, Error *error);

    /// @brief Create a BackwardEuler solver of standard-ordered matrix type by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    void CreateBackwardEulerStandardOrder(const std::string &config_path, Error *error);

    /// @brief Solve the system
    /// @param solver Pointer to solver
    /// @param time_step Time [s] to advance the state by
    /// @param temperature Temperature [grid cell] (K)
    /// @param pressure Pressure [grid cell] (Pa)
    /// @param air_density Air density [grid cell] (mol m-3)
    /// @param concentrations Array of species' concentrations [grid cell][species] (mol m-3)
    /// @param custom_rate_parameters Array of custom rate parameters [grid cell][parameter] (various units)
    /// @param error Error struct to indicate success or failure
    void Solve(
        auto &solver,
        double time_step,
        double *temperature,
        double *pressure,
        double *air_density,
        double *concentrations,
        double *custom_rate_parameters,
        String *solver_state,
        SolverResultStats *solver_stats,
        Error *error);

    /// @brief Set solver type
    /// @param MICMSolver Type of MICMSolver
    void SetSolverType(MICMSolver solver_type)
    {
      solver_type_ = solver_type;
    }

    /// @brief Set number of grid cells
    /// @param num_grid_cells Number of grid cells
    void SetNumGridCells(int num_grid_cells)
    {
      num_grid_cells_ = num_grid_cells;
    }

    /// @brief Get the ordering of species
    /// @param solver Pointer to solver
    /// @param error Error struct to indicate success or failure
    /// @return Map of species names to their indices
    // std::map<std::string, std::size_t> GetSpeciesOrdering(auto &solver, Error *error);
    template<class T>
    std::map<std::string, std::size_t> GetSpeciesOrdering(T &solver, Error *error);

    /// @brief Get the ordering of user-defined reaction rates
    /// @param solver Pointer to solver
    /// @param error Error struct to indicate success or failure
    /// @return Map of reaction rate names to their indices
    template<class T>
    std::map<std::string, std::size_t> GetUserDefinedReactionRatesOrdering(T &solver, Error *error);

    /// @brief Get a property for a chemical species
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @param error Error struct to indicate success or failure
    /// @return Value of the property
    template<class T>
    T GetSpeciesProperty(const std::string &species_name, const std::string &property_name, Error *error);

   public:
    MICMSolver solver_type_;

    /// @brief Vector-ordered Rosenbrock
    using DenseMatrixVector = micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>;
    using SparseMatrixVector = micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>;
    using RosenbrockVectorType = typename micm::RosenbrockSolverParameters::
        template SolverType<micm::ProcessSet, micm::LinearSolver<SparseMatrixVector, micm::LuDecomposition>>;
    using Rosenbrock = micm::Solver<RosenbrockVectorType, micm::State<DenseMatrixVector, SparseMatrixVector>>;
    using VectorState = micm::State<DenseMatrixVector, SparseMatrixVector>;
    std::unique_ptr<Rosenbrock> rosenbrock_;

    /// @brief Standard-ordered Rosenbrock solver type
    using DenseMatrixStandard = micm::Matrix<double>;
    using SparseMatrixStandard = micm::SparseMatrix<double, micm::SparseMatrixStandardOrdering>;
    using RosenbrockStandardType = typename micm::RosenbrockSolverParameters::
        template SolverType<micm::ProcessSet, micm::LinearSolver<SparseMatrixStandard, micm::LuDecomposition>>;
    using RosenbrockStandard = micm::Solver<RosenbrockStandardType, micm::State<DenseMatrixStandard, SparseMatrixStandard>>;
    using StandardState = micm::State<DenseMatrixStandard, SparseMatrixStandard>;
    std::unique_ptr<RosenbrockStandard> rosenbrock_standard_;

    /// @brief Vector-ordered Backward Euler
    using BackwardEulerVectorType = typename micm::BackwardEulerSolverParameters::
        template SolverType<micm::ProcessSet, micm::LinearSolver<SparseMatrixVector, micm::LuDecomposition>>;
    using BackwardEuler = micm::Solver<BackwardEulerVectorType, micm::State<DenseMatrixVector, SparseMatrixVector>>;
    std::unique_ptr<BackwardEuler> backward_euler_;

    /// @brief Standard-ordered Backward Euler
    using BackwardEulerStandardType = typename micm::BackwardEulerSolverParameters::
        template SolverType<micm::ProcessSet, micm::LinearSolver<SparseMatrixStandard, micm::LuDecomposition>>;
    using BackwardEulerStandard =
        micm::Solver<BackwardEulerStandardType, micm::State<DenseMatrixStandard, SparseMatrixStandard>>;
    std::unique_ptr<BackwardEulerStandard> backward_euler_standard_;

    /// @brief Returns the number of grid cells
    /// @return Number of grid cells
    int NumGridCells() const
    {
      return num_grid_cells_;
    }

   private:
    int num_grid_cells_;
    std::unique_ptr<micm::SolverParameters> solver_parameters_;
  };

  template<class T>
  inline std::map<std::string, std::size_t> MICM::GetSpeciesOrdering(T &solver, Error *error)
  {
    try
    {
      micm::State state = solver->GetState();
      DeleteError(error);
      *error = NoError();
      return state.variable_map_;
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
      return std::map<std::string, std::size_t>();
    }
  }

  template<class T>
  inline std::map<std::string, std::size_t> MICM::GetUserDefinedReactionRatesOrdering(T &solver, Error *error)
  {
    try
    {
      micm::State state = solver->GetState();
      DeleteError(error);
      *error = NoError();
      return state.custom_rate_parameter_map_;
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
      return std::map<std::string, std::size_t>();
    }
  }

  template<class T>
  inline T MICM::GetSpeciesProperty(const std::string &species_name, const std::string &property_name, Error *error)
  {
    *error = NoError();
    for (const auto &species : solver_parameters_->system_.gas_phase_.species_)
    {
      if (species.name_ == species_name)
      {
        try
        {
          return species.GetProperty<T>(property_name);
        }
        catch (const std::system_error &e)
        {
          DeleteError(error);
          *error = ToError(e);
          return T();
        }
      }
    }
    std::string msg = "Species '" + species_name + "' not found";
    DeleteError(error);
    *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SPECIES_NOT_FOUND, msg.c_str());
    return T();
  }
}  // namespace musica
