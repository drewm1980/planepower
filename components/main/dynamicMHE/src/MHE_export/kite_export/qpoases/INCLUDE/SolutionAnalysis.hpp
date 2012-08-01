// Solution analysis class, based on a class in the standard version of the qpOASES

#ifndef QPOASES_SOLUTIONANALYSIS_HPP
#define QPOASES_SOLUTIONANALYSIS_HPP

#include <QProblem.hpp>

#define KKT_DIM (2 * NVMAX + NCMAX)

class SolutionAnalysis
{
public:
	
	/** Default constructor. */
	SolutionAnalysis( );
	
	/** Copy constructor (deep copy). */
	SolutionAnalysis(	const SolutionAnalysis& rhs	/**< Rhs object. */
						);
	
	/** Destructor. */
	~SolutionAnalysis( );
	
	/** Copy asingment operator (deep copy). */
	SolutionAnalysis& operator=(	const SolutionAnalysis& rhs	/**< Rhs object. */
								);
	
	returnValue getHessianInverse( QProblem* qp, real_t* hessianInverse );
	
	returnValue getHessianInverse( QProblemB* qp, real_t* hessianInverse );
	
	returnValue getVarianceCovariance( QProblem* qp, real_t* g_b_bA_VAR, real_t* Primal_Dual_VAR );
	
private:
	
	real_t delta_g_cov[ NVMAX ];		/** A covariance-vector of g */
	real_t delta_lb_cov[ NVMAX ];		/** A covariance-vector of lb */
	real_t delta_ub_cov[ NVMAX ];		/** A covariance-vector of ub */
	real_t delta_lbA_cov[ NCMAX ];	/** A covariance-vector of lbA */
	real_t delta_ubA_cov[ NCMAX ];	/** A covariance-vector of ubA */
	
	real_t K[KKT_DIM * KKT_DIM];		/** A matrix to store an intermediate result*/
	
	/** Index arays*/
	
	int FR_idx[ NVMAX ];
	int FX_idx[ NVMAX ];
	int AC_idx[ NCMAX ];
	
	real_t delta_xFR[ NVMAX ];
	real_t delta_xFX[ NVMAX ];
	real_t delta_yAC[ NVMAX ];
	real_t delta_yFX[ NVMAX ];
	
};

#endif // QPOASES_SOLUTIONANALYSIS_HPP