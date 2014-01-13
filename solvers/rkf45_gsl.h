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

		while (Solver<ODEdef, XVector>::odeDone(t)) {
		  step(t, x, dx, x, xerr, dx);
		  t += dt;
		  odeSeries.line_of_data(t, x);
		}

	  }

	  //----------------------------------------------------
	  // code ported from o2scl/ode_rkf45_gsl.h starts here
	  //----------------------------------------------------
	protected:

	  /// \name Storage for the intermediate steps
	  //@{
	  XVector k2, k3, k4, k5, k6;
	  XVector xtmp;
	  //@}hout return


	  /** \name Storage for the coefficients
	   */
	  //@{
	  Eigen::Matrix<double, 5, 1> ah;
	  Eigen::Vector2d b3;
	  Eigen::Vector3d b4;
	  Eigen::Vector4d b5;
	  Eigen::Matrix<double, 5, 1> b6;
	  double c1, c3, c4, c5, c6;
	  Eigen::Matrix<double, 7, 1> ec;
	  //@}

	public:

	  RKF45_gsl<ODEdef, XVector>(ODEdef& ode)
	    :Solver<ODEdef, XVector>(ode), dX(odeSeries) {
		this->noiseShape = ODETypes::NOISE_NONE;
		this->order=5;

		ah << 1.0/4.0, 3.0/8.0, 12.0/13.0, 1.0, 1.0/2.0;
		b3 << 3.0/32.0, 9.0/32.0;
		b4 << 1932.0/2197.0, -7200.0/2197.0, 7296.0/2197.0;
		b5 << 8341.0/4104.0, -32832.0/4104.0, 29440.0/4104.0, -845.0/4104.0;
		b6 << -6080.0/20520.0, 41040.0/20520.0, -28352.0/20520.0, 9295.0/20520.0, -5643.0/20520.0;

		c1=902880.0/7618050.0;
		c3=3953664.0/7618050.0;
		c4=3855735.0/7618050.0;
		c5=-1371249.0/7618050.0;
		c6=277020.0/7618050.0;

		ec << 0.0, 1.0/360.0, 0.0, -128.0/4275.0, -2197.0/75240.0, 1.0/50.0, 2.0/55.0;

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

                //bool printout = true;
                //if (printout) std::cout << dxdt[0] << ", " << dxdt[1] << std::endl;

		// k1 step
		xtmp = x + dt * ah(0) * dxdt;
		//if (printout) std::cout << "k1: " << xtmp[0] << ", " << xtmp[1] << std::endl;

		// k2 step
		k2 = dX.f(t + ah[0]*dt, xtmp);
		xtmp = x + dt * (b3(0) * dxdt + b3(1) * k2);
		//if (printout) std::cout << "k2: " << xtmp[0] << ", " << xtmp[1] << std::endl;

		// k3 step
		k3 = dX.f(t + ah[1]*dt, xtmp);
		xtmp = x + dt * (b4(0) * dxdt + b4(1) * k2 + b4(2) * k3);
		//if (printout) std::cout << "k3: " << xtmp[0] << ", " << xtmp[1] << std::endl;

		// k4 step
		k4 = dX.f(t + ah[2]*dt, xtmp);
		xtmp = x + dt * (b5(0) * dxdt + b5(1) * k2 + b5(2) * k3
				 + b5(3) * k4);
		//if (printout) std::cout << "k4: " << xtmp[0] << ", " << xtmp[1] << std::endl;

		// k5 step
		k5 = dX.f(t + ah[3]*dt, xtmp);
		xtmp = x + dt * (b6(0) * dxdt + b6(1) * k2 + b6(2) * k3
				 + b6(3) * k4 + b6(4) * k5);
		//if (printout) std::cout << "k5: " << xtmp[0] << ", " << xtmp[1] << std::endl;

		// k6 step and final sum
		k6 = dX.f(t + ah[4]*dt, xtmp);
		xout = x + dt * (c1 * dxdt + c3 * k3 + c4 * k4
				 + c5 * k5 + c6 * k6);
		//if (printout) std::cout << "k6: " << xout[0] << ", " << xout[1] << std::endl;

		// We put this before the last function evaluation, in contrast
		// to the GSL version, so that the dxdt that appears
		// below isn't modified by the subsequent derivative
		// evaluation using dxdt_out. (The user could have given the
		// same vector for both)
		xerr = dt * (ec(1) * dxdt + ec(3) * k3 + ec(4) * k4
			     + ec(5) * k5 + ec(6) * k6);
		//if (printout) std::cout << "k1: " << xerr[0] << ", " << xerr[1] << std::endl;


		dxdt_out = dX.f(t + dt, xout);

		//if (printout) assert(false);

	  }


	};
}

#endif // RKF45_GSL_H
