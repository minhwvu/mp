#include <vector>
#include <climits>
#include <cfloat>

#include "cplexbackend.h"


namespace {

volatile int terminate_flag = 0;
bool InterruptCplex(void *) {
  terminate_flag = 1;
  return true;
}

}  // namespace

std::unique_ptr<mp::BasicBackend> CreateCplexBackend() {
  return std::unique_ptr<mp::BasicBackend>{new mp::CplexBackend()};
}

namespace mp {

CplexBackend::CplexBackend() {
  OpenSolver();
}

CplexBackend::~CplexBackend() {
  CloseSolver();
}

const char* CplexBackend::GetBackendName()
  { return "CplexBackend"; }

std::string CplexBackend::GetSolverVersion() {
  int version;
  CPXversionnumber(env(), &version);
  return fmt::format("{}", version);
}


bool CplexBackend::IsMIP() const {
  int probtype = CPXgetprobtype (env(), lp());
  return
      CPXPROB_MILP == probtype ||
      CPXPROB_MIQP == probtype ||
      CPXPROB_MIQCP == probtype;
}

bool CplexBackend::IsQCP() const {
  int probtype = CPXgetprobtype (env(), lp());
  return probtype >= 5;
}


Solution CplexBackend::GetSolution() {
  auto mv = GetPresolver().PostsolveSolution(
        { PrimalSolution(), DualSolution() } );
  return { mv.GetVarValues()(), mv.GetConValues()(),
    GetObjectiveValues() };   // TODO postsolve obj values
}

ArrayRef<double> CplexBackend::PrimalSolution() {
  int num_vars = NumVars();
  std::vector<double> x(num_vars);
  int error = CPXgetx (env(), lp(), x.data(), 0, num_vars-1);
  if (error)
    x.clear();
  return x;
}

pre::ValueMapDbl CplexBackend::DualSolution() {
  return {{ { CG_Linear, DualSolution_LP() } }};
}

ArrayRef<double> CplexBackend::DualSolution_LP() {
  int num_cons = NumLinCons();
  std::vector<double> pi(num_cons);
  int error = CPXgetpi (env(), lp(), pi.data(), 0, num_cons-1);
  if (error)
    pi.clear();
  return pi;
}

double CplexBackend::ObjectiveValue() const {
  double objval = -1e308;
  CPXgetobjval (env(), lp(), &objval );   // failsafe
  return objval;
}

double CplexBackend::NodeCount() const {
  return CPXgetnodecnt (env(), lp());
}

double CplexBackend::SimplexIterations() const {
  return std::max(
        CPXgetmipitcnt (env(), lp()), CPXgetitcnt (env(), lp()));
}

int CplexBackend::BarrierIterations() const {
  return CPXgetbaritcnt (env(), lp());
}

void CplexBackend::ExportModel(const std::string &file) {
  CPLEX_CALL( CPXwriteprob (env(), lp(), file.c_str(), NULL) );
}


void CplexBackend::SetInterrupter(mp::Interrupter *inter) {
  inter->SetHandler(InterruptCplex, nullptr);
  CPLEX_CALL( CPXsetterminate (env(), &terminate_flag) );
}

void CplexBackend::SolveAndReportIntermediateResults() {
  if (!storedOptions_.exportFile_.empty()) {
    ExportModel(storedOptions_.exportFile_);
  }

  CPLEX_CALL( CPXmipopt(env(), lp()) );

  WindupCPLEXSolve();
}

void CplexBackend::WindupCPLEXSolve() {
  SetStatus( ConvertCPLEXStatus() );
  AddCPLEXMessages();
}

void CplexBackend::AddCPLEXMessages() {
  if (auto ni = SimplexIterations())
    AddToSolverMessage(
          fmt::format("{} simplex iterations\n", ni));
  if (auto nbi = BarrierIterations())
    AddToSolverMessage(
          fmt::format("{} barrier iterations\n", nbi));
  if (auto nnd = NodeCount())
    AddToSolverMessage(
          fmt::format("{} branching nodes\n", nnd));
}

std::pair<int, std::string> CplexBackend::ConvertCPLEXStatus() {
  namespace sol = mp::sol;
  int optimstatus = CPXgetstat(env(), lp());
  switch (optimstatus) {
  default:
    // Fall through.
    if (interrupter()->Stop()) {
      return { sol::INTERRUPTED, "interrupted" };
    }
    int solcount;
    solcount = CPXgetsolnpoolnumsolns (env(), lp());  // Can we use it without CPXpopulate?
    if (solcount>0) {
      return { sol::UNCERTAIN, "feasible solution" };
    }
    return { sol::UNKNOWN, "unknown solution status" };
  case CPX_STAT_OPTIMAL:
  case CPXMIP_OPTIMAL:
  case CPX_STAT_MULTIOBJ_OPTIMAL:
    return { sol::SOLVED, "optimal solution" };
  case CPX_STAT_INFEASIBLE:
  case CPXMIP_INFEASIBLE:
  case CPX_STAT_MULTIOBJ_INFEASIBLE:
    return { sol::INFEASIBLE, "infeasible problem" };
  case CPX_STAT_INForUNBD:
  case CPXMIP_INForUNBD:
  case CPX_STAT_MULTIOBJ_INForUNBD:
    return { sol::INF_OR_UNB, "infeasible or unbounded problem" };
  case CPX_STAT_UNBOUNDED:
  case CPXMIP_UNBOUNDED:
  case CPX_STAT_MULTIOBJ_UNBOUNDED:
    return { sol::UNBOUNDED, "unbounded problem" };
  case CPX_STAT_FEASIBLE_RELAXED_INF:
  case CPX_STAT_FEASIBLE_RELAXED_QUAD:
  case CPX_STAT_FEASIBLE_RELAXED_SUM:
  case CPX_STAT_NUM_BEST:
  case CPX_STAT_OPTIMAL_INFEAS:
  case CPX_STAT_OPTIMAL_RELAXED_INF:
  case CPX_STAT_OPTIMAL_RELAXED_QUAD:
  case CPX_STAT_OPTIMAL_RELAXED_SUM:
    return { sol::UNCERTAIN, "feasible or optimal but numeric issue" };
  }
}


void CplexBackend::FinishOptionParsing() {
  int v=-1;
  GetSolverOption(CPXPARAM_MIP_Display, v);
  set_verbose_mode(v>0);
}


////////////////////////////// OPTIONS /////////////////////////////////

void CplexBackend::InitCustomOptions() {

  set_option_header(
      "IBM ILOG CPLEX Optimizer Options for AMPL\n"
      "--------------------------------------------\n"
      "\n"
      "To set these options, assign a string specifying their values to the "
      "AMPL option ``cplexdirect_options``. For example::\n"
      "\n"
      "  ampl: option cplexdirect_options 'mipgap=1e-6';\n");

  AddSolverOption("tech:outlev outlev",
      "0-5: output logging verbosity. "
      "Default = 0 (no logging).",
      CPXPARAM_MIP_Display, 0, 5);
  SetSolverOption(CPXPARAM_MIP_Display, 0);

  AddStoredOption("tech:exportfile writeprob",
      "Specifies the name of a file where to export the model before "
      "solving it. This file name can have extension ``.lp()``, ``.mps``, etc. "
      "Default = \"\" (don't export the model).",
      storedOptions_.exportFile_);

  AddSolverOption("mip:gap mipgap",
      "Relative optimality gap |bestbound-bestinteger|/(1e-10+|bestinteger|).",
      CPXPARAM_MIP_Tolerances_MIPGap, 0.0, 1.0);

  AddSolverOption("tech:threads threads",
      "How many threads to use when using the barrier algorithm\n"
      "or solving MIP problems; default 0 ==> automatic choice.",
      CPXPARAM_Threads, 0, INT_MAX);

  AddSolverOption("lim:time timelim timelimit",
      "limit on solve time (in seconds; default: no limit).",
      CPXPARAM_TimeLimit, 0.0, DBL_MAX);

}


} // namespace mp
