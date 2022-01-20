#ifndef MP_GUROBI_BACKEND_H_
#define MP_GUROBI_BACKEND_H_

#if __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-parameter"
# pragma clang diagnostic ignored "-Wunused-private-field"
#elif _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4244)
#endif

extern "C" {
  #include "gurobi_c.h"
}

#if __clang__
# pragma clang diagnostic pop
#elif _MSC_VER
# pragma warning(pop)
#endif

#include <string>

#include "mp/flat/MIP/backend.h"
#include "mp/flat/std_constr.h"

namespace mp {

class GurobiBackend :
    public MIPBackend<GurobiBackend>
{
  using BaseBackend = MIPBackend<GurobiBackend>;

public:
  GurobiBackend();
  ~GurobiBackend();

  ////////////////////////////////////////////////////////////
  //////////////////// PART 1. Accessor API //////////////////
  /// Standard and optional methods to provide or retrieve ///
  /// information to/from or manipulate the solver. Most  ////
  /// of them override placeholders from base classes.   /////
  ////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////
  //////////////////////// Metadata //////////////////////////
  ////////////////////////////////////////////////////////////
  static const char* GetSolverName() { return "Gurobi"; }
  static std::string GetSolverVersion();
  static const char* GetSolverInvocationName();
  static const char* GetSolverLongName() { return "AMPLGurobi"; }
  static const char* GetBackendName();
  static const char* GetBackendLongName() { return nullptr; }

  ////////////////////////////////////////////////////////////
  /////////////// OPTIONAL STANDARD FEATURES /////////////////
  ////////////////////////////////////////////////////////////
  USING_STD_FEATURES;
  /**
   * MULTIOBJ
  **/
  ALLOW_STD_FEATURE( MULTIOBJ, true )
  ArrayRef<double> ObjectiveValues() const;
  void ObjPriorities(ArrayRef<int>);
  void ObjWeights(ArrayRef<double>);
  void ObjAbsTol(ArrayRef<double>);
  void ObjRelTol(ArrayRef<double>);
  /**
   * MULTISOL support
   * No API, use ReportIntermediateSolution()
  **/
  ALLOW_STD_FEATURE( MULTISOL, true )
  /**
  * Set lazy/user cut attributes
  * Negative suffix values are "user cuts"
  * Check lazy_/user_cuts() to see which kinds are allowed
  **/
  ALLOW_STD_FEATURE( LAZY_USER_CUTS, true )
  void MarkLazyOrUserCuts(ArrayRef<int> );
  /**
  * Get/Set AMPL var/con statii
  **/
  ALLOW_STD_FEATURE( BASIS, true )
  SolutionBasis GetBasis();
  void SetBasis(SolutionBasis );
  /**
  * General warm start, e.g.,
  * set primal/dual initial guesses for continuous case
  **/
  ALLOW_STD_FEATURE( WARMSTART, true )
  void InputPrimalDualStart(ArrayRef<double> x0,
                         ArrayRef<double> pi0);
  /**
  * Specifically, MIP warm start
  **/
  ALLOW_STD_FEATURE( MIPSTART, true )
  void AddMIPStart(ArrayRef<double> x0);
  /**
  * Obtain inf/unbounded rays
  **/
  ALLOW_STD_FEATURE( RAYS, true )
  ArrayRef<double> Ray();
  ArrayRef<double> DRay();
  /**
  * Compute the IIS and obtain relevant values
  **/
  ALLOW_STD_FEATURE( IIS, true )
  /// Compute IIS
  void ComputeIIS();
  /// Retrieve IIS. Elements correspond to IISStatus
  IIS GetIIS();
  /**
  * Get MIP Gap
  **/
  ALLOW_STD_FEATURE( RETURN_MIP_GAP, true )
  double MIPGap() const;
  /**
  * Get MIP dual bound
  **/
  ALLOW_STD_FEATURE( RETURN_BEST_DUAL_BOUND, true )
  double BestDualBound() const;
  /**
  * Set branch and bound priorities
  **/
  ALLOW_STD_FEATURE( VAR_PRIORITIES, true )
  void VarPriorities(ArrayRef<int> );
  /**
  * Get basis condition value (kappa)
  **/
  ALLOW_STD_FEATURE( KAPPA, true)
  double Kappa() const;
  /**
  * FeasRelax
  **/
  ALLOW_STD_FEATURE( FEAS_RELAX, true)
  /**
  * Report sensitivity analysis suffixes
  **/
  ALLOW_STD_FEATURE( SENSITIVITY_ANALYSIS, true )
  ArrayRef<double> Senslbhi() const;
  ArrayRef<double> Senslblo() const;
  ArrayRef<double> Sensobjhi() const;
  ArrayRef<double> Sensobjlo() const;
  ArrayRef<double> Sensrhshi() const;
  ArrayRef<double> Sensrhslo() const;
  ArrayRef<double> Sensubhi() const;
  ArrayRef<double> Sensublo() const;
  /**
  * FixModel - duals, basis, and sensitivity for MIP
  * No API to overload,
  * Impl should check need_fixed_MIP()
  **/
  ALLOW_STD_FEATURE( FIX_MODEL, true )


  /////////////////////////////////////////////////////////////////////////////
  //////////////////////////// MODELING ACCESSORS /////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  static constexpr double Infinity() { return GRB_INFINITY; }
  static constexpr double MinusInfinity() { return -GRB_INFINITY; }

  void AddVariables(const VarArrayDef& );
  void SetLinearObjective( int iobj, const LinearObjective& lo );
  void SetQuadraticObjective( int iobj, const QuadraticObjective& qo );
  //////////////////////////// GENERAL CONSTRAINTS ////////////////////////////
  USE_BASE_CONSTRAINT_HANDLERS(BaseBackend)

  /// TODO Attributes (lazy/user cut, etc)
  ACCEPT_CONSTRAINT(LinConLE, Recommended, CG_Linear)
  void AddConstraint(const LinConLE& lc);
  ACCEPT_CONSTRAINT(LinConEQ, Recommended, CG_Linear)
  void AddConstraint(const LinConEQ& lc);
  ACCEPT_CONSTRAINT(LinConGE, Recommended, CG_Linear)
  void AddConstraint(const LinConGE& lc);
  ACCEPT_CONSTRAINT(QuadraticConstraint, Recommended, CG_Quadratic)
  void AddConstraint(const QuadraticConstraint& qc);
  ACCEPT_CONSTRAINT(MaximumConstraint, AcceptedButNotRecommended, CG_General)
  void AddConstraint(const MaximumConstraint& mc);
  ACCEPT_CONSTRAINT(MinimumConstraint, AcceptedButNotRecommended, CG_General)
  void AddConstraint(const MinimumConstraint& mc);
  ACCEPT_CONSTRAINT(AbsConstraint, AcceptedButNotRecommended, CG_General)
  void AddConstraint(const AbsConstraint& absc);
  ACCEPT_CONSTRAINT(ConjunctionConstraint, AcceptedButNotRecommended, CG_General)
  void AddConstraint(const ConjunctionConstraint& cc);
  ACCEPT_CONSTRAINT(DisjunctionConstraint, AcceptedButNotRecommended, CG_General)
  void AddConstraint(const DisjunctionConstraint& mc);
  /// Enabling built-in indicator for infinite bounds,
  /// but not recommended otherwise --- may be slow
  ACCEPT_CONSTRAINT(IndicatorConstraintLinLE, AcceptedButNotRecommended, CG_General)
  void AddConstraint(const IndicatorConstraintLinLE& mc);
  ACCEPT_CONSTRAINT(IndicatorConstraintLinEQ, AcceptedButNotRecommended, CG_General)
  void AddConstraint(const IndicatorConstraintLinEQ& mc);

  /// General
  ACCEPT_CONSTRAINT(SOS1Constraint, Recommended, CG_SOS)
  void AddConstraint(const SOS1Constraint& cc);
  ACCEPT_CONSTRAINT(SOS2Constraint, Recommended, CG_SOS)
  void AddConstraint(const SOS2Constraint& cc);
  ACCEPT_CONSTRAINT(ExpConstraint, Recommended, CG_General)
  void AddConstraint(const ExpConstraint& cc);
  ACCEPT_CONSTRAINT(ExpAConstraint, Recommended, CG_General)
  void AddConstraint(const ExpAConstraint& cc);
  ACCEPT_CONSTRAINT(LogConstraint, Recommended, CG_General)
  void AddConstraint(const LogConstraint& cc);
  ACCEPT_CONSTRAINT(LogAConstraint, Recommended, CG_General)
  void AddConstraint(const LogAConstraint& cc);
  ACCEPT_CONSTRAINT(PowConstraint, Recommended, CG_General)
  void AddConstraint(const PowConstraint& cc);
  ACCEPT_CONSTRAINT(SinConstraint, Recommended, CG_General)
  void AddConstraint(const SinConstraint& cc);
  ACCEPT_CONSTRAINT(CosConstraint, Recommended, CG_General) // y = cos(x)
  void AddConstraint(const CosConstraint& cc);  // GRBaddgenconstrCos(x, y);
  ACCEPT_CONSTRAINT(TanConstraint, Recommended, CG_General)
  void AddConstraint(const TanConstraint& cc);
  ACCEPT_CONSTRAINT(PLConstraint, Recommended, CG_General)
  void AddConstraint(const PLConstraint& cc);


  ///////////////////// Model attributes /////////////////////
  bool IsMIP() const;
  bool IsQP() const;
  bool IsQCP() const;

  /// Gurobi separates constraint classes
  int NumLinCons() const;
  int NumQPCons() const;
  int NumSOSCons() const;
  int NumGenCons() const;
  int NumVars() const;
  int NumObjs() const;
  int ModelSense() const;


  /////////////////////////////////////////////////////////////////////////////
  //////////////////////////// OPTION ACCESSORS ///////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  /// Gurobi-specific options
  void InitCustomOptions();

  /// Chance for the Backend to init solver environment, etc
  void InitOptionParsing();
  /// Chance to consider options immediately (open cloud, etc)
  void FinishOptionParsing();

  /// Public option API.
  /// These methods access Gurobi options. Used by AddSolverOption()
  void GetSolverOption(const char* key, int& value) const;
  void SetSolverOption(const char* key, int value);
  void GetSolverOption(const char* key, double& value) const;
  void SetSolverOption(const char* key, double value);
  void GetSolverOption(const char* key, std::string& value) const;
  void SetSolverOption(const char* key, const std::string& value);



  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////// SOLVING ACCESSORS ///////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  void SetInterrupter(mp::Interrupter* inter);

  /// This is called before model is pushed to the Backend
  void InitProblemModificationPhase();
  /// Chance to call GRBupdatemodel()
  void FinishProblemModificationPhase();

  void SolveAndReportIntermediateResults();

  /// Various solution attribute getters.
  ArrayRef<double> PrimalSolution();
  double ObjectiveValue() const;
  /// Return empty vector if not available
  ArrayRef<double> DualSolution();


  ///////////////////////////////////////////////////////////////////////////////
  //////////////////// PART 2. Implementation's internals ///////////////////////
  //////////////////// Gurobi methods should include name Gurobi or similar /////
  //////////////////// to avoid name clashes with the base classes //////////////
  ///////////////////////////////////////////////////////////////////////////////
protected:
  void OpenGurobi();
  void OpenGurobiModel();
  void CloseGurobi();

  void OpenGurobiComputeServer();
  void OpenGurobiCloud();

  void ExportModel(const std::string& file);

  void PrepareGurobiSolve();
  void DoGurobiFeasRelax();
  void SetPartitionValues();

  void DoGurobiTune();

  void WindupGurobiSolve();
  std::pair<int, std::string> ConvertGurobiStatus() const;
  void AddGurobiMessage();

  void ReportGurobiPool();
  /// Creates and solves, marks model_fixed to be used for duals/basis/sens
  void ConsiderGurobiFixedModel();
  /// Return error message if any
  std::string DoGurobiFixedModel();
  /// First objective's sense
  void NoteGurobiMainObjSense(obj::Type s);
  obj::Type GetGurobiMainObjSense() const;
  ArrayRef<double> CurrentGrbPoolPrimalSolution();
  double CurrentGrbPoolObjectiveValue() const;

  std::vector<double> GurobiDualSolution_LP();
  std::vector<double> GurobiDualSolution_QCP();

  ArrayRef<int> VarStatii();
  ArrayRef<int> ConStatii();
  void VarStatii(ArrayRef<int> );
  void ConStatii(ArrayRef<int> );

  ArrayRef<int> VarsIIS();
  pre::ValueMapInt ConsIIS();

  double NodeCount() const;
  double SimplexIterations() const;
  int BarrierIterations() const;

  /// REMEMBER Gurobi does not update attributes before calling optimize() etc
  /// Scalar attributes. If (flag), set *flag <-> success,
  /// otherwise fail on error
  int GrbGetIntAttr(const char* attr_id, bool *flag=nullptr) const;
  /// If (flag), set *flag <-> success, otherwise fail on error
  double GrbGetDblAttr(const char* attr_id, bool *flag=nullptr) const;
  /// Vector attributes. Return empty vector on failure
  std::vector<int> GrbGetIntAttrArray(const char* attr_id,
    std::size_t size, std::size_t offset=0) const;
  std::vector<double> GrbGetDblAttrArray(const char* attr_id,
    std::size_t size, std::size_t offset=0) const;
  std::vector<int> GrbGetIntAttrArray(GRBmodel* mdl, const char* attr_id,
    std::size_t size, std::size_t offset=0) const;
  std::vector<double> GrbGetDblAttrArray(GRBmodel* mdl, const char* attr_id,
    std::size_t size, std::size_t offset=0) const;

  /// varcon: 0 - vars, 1 - constraints
  std::vector<double> GrbGetDblAttrArray_VarCon(
      const char* attr, int varcon) const;
  /// varcon: 0 - vars, 1 - constraints
  std::vector<double> GrbGetDblAttrArray_VarCon(GRBmodel* mdl,
      const char* attr, int varcon) const;

  /// Set attributes.
  /// Return false on failure
  void GrbSetIntAttr(const char* attr_id, int val);
  void GrbSetDblAttr(const char* attr_id, double val);
  ///  Silently ignore empty vector arguments.
  void GrbSetIntAttrArray(const char* attr_id,
                               ArrayRef<int> values, std::size_t start=0);
  void GrbSetDblAttrArray(const char* attr_id,
                               ArrayRef<double> values, std::size_t start=0);
  ///  Silently ignore empty vector arguments.
  void GrbSetIntAttrList(const char* attr_id,
                         const std::vector<int>& idx, const std::vector<int>& val);
  void GrbSetDblAttrList(const char* attr_id,
                         const std::vector<int>& idx, const std::vector<double>& val);



private:
  GRBenv   *env_   = nullptr;
  GRBmodel *model_ = nullptr;
  GRBmodel *model_fixed_ = nullptr;

  /// The sense of the main objective
  obj::Type main_obj_sense_;

private:
  /// These options are stored in the class as variables
  /// for direct access
  struct Options {
    std::string exportFile_, paramRead_, paramWrite_;

    int nMIPStart_=1;
    int nPoolMode_=2;

    int nFixedMethod_=-2;

    std::string cloudid_, cloudkey_, cloudpool_;
    int cloudpriority_;

    std::string servers_, server_password_, server_group_, server_router_;
    int server_priority_=0, server_insecure_=0;
    double server_timeout_=-1.0;

    std::string tunebase_;
  } storedOptions_;


protected:  //////////// Option accessors ////////////////
  int Gurobi_mipstart() const { return storedOptions_.nMIPStart_; }

  const std::string& paramfile_read() const { return storedOptions_.paramRead_; }
  const std::string& paramfile_write() const { return storedOptions_.paramWrite_; }

  const std::string& cloudid() const { return storedOptions_.cloudid_; }
  const std::string& cloudkey() const { return storedOptions_.cloudkey_; }
  const std::string& cloudpool() const { return storedOptions_.cloudpool_; }
  int cloudpriority() const { return storedOptions_.cloudpriority_; }

  const std::string& servers() const { return storedOptions_.servers_; }
  const std::string& server_password() const { return storedOptions_.server_password_; }
  const std::string& server_group() const { return storedOptions_.server_group_; }
  const std::string& server_router() const { return storedOptions_.server_router_; }
  int server_priority() const { return storedOptions_.server_priority_; }
  int server_insecure() const { return storedOptions_.server_insecure_; }
  int server_timeout() const { return storedOptions_.server_timeout_; }

  const std::string& tunebase() const { return storedOptions_.tunebase_; }

private: /////////// Suffixes ///////////
  const SuffixDef<int> sufHintPri = { "hintpri", suf::VAR | suf::INPUT };


protected:  //////////// Wrappers for Get/SetSolverOption(). Assume model_ is set
  int GrbGetIntParam(const char* key) const;
  double GrbGetDblParam(const char* key) const;
  std::string GrbGetStrParam(const char* key) const;
  void GrbSetIntParam(const char* key, int value);
  void GrbSetDblParam(const char* key, double value);
  void GrbSetStrParam(const char* key, const std::string& value);

  /// For "obj:*:method" etc
  /// Should they be handled in the Converter?
public:
  using ObjNParamKey = std::pair< std::string, std::string >;
  template <class T>
  using ObjNParam = std::pair< ObjNParamKey, T >;
private:
  std::vector< ObjNParam<int> > objnparam_int_;
  std::vector< ObjNParam<double> > objnparam_dbl_;
protected:
  /// Assume opt has the * info
  void GrbSetObjIntParam(const SolverOption& opt, int val);
  void GrbSetObjDblParam(const SolverOption& opt, double val);
  int GrbGetObjIntParam(const SolverOption& opt) const;
  double GrbGetObjDblParam(const SolverOption& opt) const;

  void GrbPlayObjNParams();
};

} // namespace mp

#endif  // MP_GUROBI_BACKEND_H_
