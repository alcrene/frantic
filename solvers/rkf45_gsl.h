/* Runge-Kutta-Fehlberg embedded Runge-Kutta ODE for error control
 * Adapted version from the version in O2scl; that code's copyright info follows
 */
/*
  -------------------------------------------------------------------

  Copyright (C) 2006-2013, Andrew W. Steiner

  This file is part of O2scl.

  O2scl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  O2scl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with O2scl. If not, see <http://www.gnu.org/licenses/>.

  -------------------------------------------------------------------
*/
/* ode-initval/rkf45.c
 *
 * Copyright (C) 2001, 2004, 2007 Brian Gough
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#ifndef RKF45_GSL_H
#define RKF45_GSL_H

#include "solver.h"

//#include <o2scl/test_mgr.h>
//#include <o2scl/ode_funct.h>
//#include <o2scl/ode_rkf45_gsl.h>
//#include <o2scl/table.h>
//#include <o2scl/ode_iv_solve.h>
//#include <o2scl/hdf_file.h>
//#include <o2scl/hdf_io.h>

using std::vector;

namespace solvers {

  template <typename ODEdef, typename XVector>
        class RKF45_gsl : public Solver<ODEdef, XVector>
        {
        private:
//	  ODETypes::NOISE_SHAPE noiseShape = ODETypes::NOISE_NONE;

	  // required because this is a template function
	  using Solver<ODEdef, XVector>::odeSeries;
	  using Solver<ODEdef, XVector>::tBegin;
	  using Solver<ODEdef, XVector>::tEnd;
	  using Solver<ODEdef, XVector>::dt;  // timestep
	  using Solver<ODEdef, XVector>::tNumSteps;
	  using Solver<ODEdef, XVector>::ode;

	  typename ODEdef::func_dX dX;

	public:
	  // Constructor below
	  virtual ~RKF45_gsl() {}

	  void solve(Param parameters) {
		double t;
		XVector x, dx, xout, dx_out;
		XVector xerr;

		//o2scl::test_mgr test_mgr;
		//test_mgr.set_output_level(1);

		dX.setParameters(parameters);

		t = tBegin;
		x = ode.x0;
		dx = dX.f(t, x);

		while (t < tEnd) {
		  step(t, x, dx, x, xerr, dx);
		  t += dt;
		  odeSeries.line_of_data(t, x);
		}

	  }

	  //----------------------------------------------------
	  // code from o2scl/ode_rkf45_gsl.h starts here
	  //----------------------------------------------------
	protected:

	  /// \name Storage for the intermediate steps
	  //@{
	  XVector k2, k3, k4, k5, k6;
	  XVector xtmp;
	  //@}

	  /// Size of allocated vectors
	  //	  size_t ndim;

	  /** \name Storage for the coefficients
	   */
	  //@{
	  double ah[5], b3[2], b4[3], b5[4], b6[5];
	  double c1, c3, c4, c5, c6;
	  double ec[7];
	  //@}

	public:

	  RKF45_gsl<ODEdef, XVector>(ODEdef& ode)
	    :Solver<ODEdef, XVector>(ode), dX(odeSeries) {
		this->noiseShape = ODETypes::NOISE_NONE;
		this->order=5;

		ah[0]=1.0/4.0;
		ah[1]=3.0/8.0;
		ah[2]=12.0/13.0;
		ah[3]=1.0;
		ah[4]=1.0/2.0;

		b3[0]=3.0/32.0;
		b3[1]=9.0/32.0;

		b4[0]=1932.0/2197.0;
		b4[1]=-7200.0/2197.0;
		b4[2]=7296.0/2197.0;

		b5[0]=8341.0/4104.0;
		b5[1]=-32832.0/4104.0;
		b5[2]=29440.0/4104.0;
		b5[3]=-845.0/4104.0;

		b6[0]=-6080.0/20520.0;
		b6[1]=41040.0/20520.0;
		b6[2]=-28352.0/20520.0;
		b6[3]=9295.0/20520.0;
		b6[4]=-5643.0/20520.0;

		c1=902880.0/7618050.0;
		c3=3953664.0/7618050.0;
		c4=3855735.0/7618050.0;
		c5=-1371249.0/7618050.0;
		c6=277020.0/7618050.0;

		ec[0]=0.0;
		ec[1]=1.0/360.0;
		ec[2]=0.0;
		ec[3]=-128.0/4275.0;
		ec[4]=-2197.0/75240.0;
		ec[5]=1.0/50.0;
		ec[6]=2.0/55.0;

		//		ndim=0;
	  }

protected:
	  /** \brief Perform an integration step

		  Given initial value of the n-dimensional function in \c x and
		  the derivative in \c dxdt (which must be computed beforehand) at
		  the point \c x, take a step of size \c h giving the result in \c
		  xout, the uncertainty in \c xerr, and the new derivative in \c
		  dxdt_out using function \c derivs to calculate derivatives. The
		  parameters \c xout and \c x and the parameters \c dxdt_out and
		  \c dxdt may refer to the same object.

		  If \c derivs always returns zero, then this function will
		  also return zero. If not, <tt>step()</tt> will return the first
		  non-zero value which was obtained in a call to \c derivs .
		  The error handler is never called.

		  \todo: use standard std::vector for xerr to avoid overhead ?
		  \todo: implement error checking (removed o2scl mechanism)
	  */
	  void step(double t, XVector &x, XVector &dxdt,
		    XVector &xout, XVector &xerr, XVector &dxdt_out) {

		int ret=0;
		size_t i;


		// k1 step
		for (i=0; i<XVector::SizeAtCompileTime; ++i) {
		  xtmp[i]=x[i]+ah[0]*dt*dxdt[i];
		}

		// k2 step		dX.alpha = ode.alpha;   //FIXME: use generic std::map
		k2 = dX.f(t + ah[0]*dt, xtmp);

		for (i=0; i<XVector::SizeAtCompileTime; ++i) {
		  xtmp[i]=x[i]+dt*(b3[0]*dxdt[i]+b3[1]*k2[i]);
		}

		// k3 step
		k3 = dX.f(t + ah[1]*dt, xtmp);

		for (i=0; i<XVector::SizeAtCompileTime; ++i) {
		  xtmp[i]=x[i]+dt*(b4[0]*dxdt[i]+b4[1]*k2[i]+b4[2]*k3[i]);
		}

		// k4 step
		k4 = dX.f(t + ah[2]*dt, xtmp);

		for (i=0; i<XVector::SizeAtCompileTime; ++i) {
		  xtmp[i]=x[i]+dt*(b5[0]*dxdt[i]+b5[1]*k2[i]+b5[2]*k3[i]+
						   b5[3]*k4[i]);
		}

		// k5 step
		k5 = dX.f(t + ah[3]*dt, xtmp);

		for (i=0; i<XVector::SizeAtCompileTime; ++i) {
		  xtmp[i]=x[i]+dt*(b6[0]*dxdt[i]+b6[1]*k2[i]+b6[2]*k3[i]+
						   b6[3]*k4[i]+b6[4]*k5[i]);
		}

		// k6 step and final sum
		k6 = dX.f(t + ah[4]*dt, xtmp);

		for (i=0; i<XVector::SizeAtCompileTime; ++i) {
		  xout[i]=x[i]+dt*(c1*dxdt[i]+c3*k3[i]+c4*k4[i]+c5*k5[i]+c6*k6[i]);
		}

		// We put this before the last function evaluation, in contrast
		// to the GSL version, so that the dxdt[i] that appears in the
		// for loop below isn't modified by the subsequent derivative
		// evaluation using dxdt_out. (The user could have given the
		// same vector for both)
		for (i=0; i<XVector::SizeAtCompileTime; ++i) {
		  xerr[i] = dt * (  ec[1] * dxdt[i]
							+ ec[3] * k3[i]
							+ ec[4] * k4[i]
							+ ec[5] * k5[i]
							+ ec[6] * k6[i]);
		}

	  }


	};
}

#endif // RKF45_GSL_H
