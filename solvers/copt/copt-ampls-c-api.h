#ifndef COPTAMPLSCAPI_H
#define COPTAMPLSCAPI_H
/*
 * C API for MP/Copt
 */

#include "copt.h"

#include "mp/ampls-c-api.h"

/*
 * Below are Copt-specific AMPLS API functions.
 * They complement the 'public' AMPLS API defined in ampls-c-api.h.
 */

/// Initialize AMPLS copt.
/// @param slv_opt: a string of solver options
/// (normally provided in the <solver>_options string).
/// Can be NULL.
/// @return pointer to populated AMPLS_MP_Solver struct, or NULL
AMPLS_MP_Solver* AMPLSOpenCopt(const char* slv_opt);

/// Shut down solver instance
void AMPLSCloseCopt(AMPLS_MP_Solver* slv);

/// Extract the Copt model handle
copt_prob* GetCoptmodel(AMPLS_MP_Solver* slv);


#endif // COPTAMPLSCAPI_H
