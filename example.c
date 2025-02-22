/**
 * Copyright 2017 William Grim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "ballistics/ballistics.h"

int main(void) {
	int k=0;
	Ballistics* solution;
	double bc=0.224; // The ballistic coefficient for the projectile.
	double v=2650; // Intial velocity, in ft/s
	double sh=1.5	; // The Sight height over bore, in inches.
	double angle=0; // The shooting angle (uphill / downhill), in degrees.
	double zero=100; // The zero range of the rifle, in yards.
	double windspeed=10; // The wind speed in miles per hour.
	double windangle=90; // The wind angle (0=headwind, 90=right to left, 180=tailwind, 270/-90=left to right)
	double temp=59;
	double inHg=29.92;
	double caliber=.308;
	double bulletGrains=168;
	double bulletLength=1.215;
	double twistDenominator=12;
	double formFactor = 1.0;
	
	// If we wish to use the weather correction features, we need to 
	// Correct the BC for any weather conditions.  If we want standard conditions,
	// then we can just leave this commented out.

	bc = atmosphere_correction(bc, 0, inHg, temp, .78);
	
	
	// First find the angle of the bore relative to the sighting system.
	// We call this the "zero angle", since it is the angle required to 
	// achieve a zero at a particular yardage.  This value isn't very useful
	// to us, but is required for making a full ballistic solution.
	// It is left here to allow for zero-ing at altitudes (bc) different from the
	// final solution, or to allow for zero's other than 0" (ex: 3" high at 100 yds)
	//double zeroangle = zero_angle(G1, bc, v, 1.6, zero, 0);
	double zeroangle = zero_angle(G7, bc, v, sh, zero, 0);

	// Now we have everything needed to generate a full solution.
	// So we do.  The solution is stored in the pointer "sln" passed as the last argument.
	// k has the number of yards the solution is valid for, also the number of rows in the solution.
	k = Ballistics_solve(&solution, G7, bc, v, sh, angle, zeroangle, windspeed, windangle);
	Ballistics * solution2;
	int k2 = Ballistics_solve_modified_vertDeflect(&solution2, G7, bc, v, sh, angle, zeroangle, windspeed, windangle,caliber,bulletLength,temp,inHg,twistDenominator,v,bulletGrains, formFactor);

	// Now print a simple chart of X / Y trajectory spaced at 10yd increments
	int s=0;
	for (s=0; s<=1000; s+=100) {
		printf("X: %.0f     Y: %.2f		Wind: %.2f\n", Ballistics_get_range(solution, s), Ballistics_get_path(solution, s),
		       Ballistics_get_windage(solution, s));
	}

	//appendix example
	printf("%.5f\n",calculateGS(155,13,.308,1.2,3000,85,28.5));
	//vert deflec example
	printf("%.5f\n",calculateGS(180,8,.284,1.517,2800,59,29.92));

	printf("%.5f\n\n\n\n",calculateVerticalDeflection(calculateGS(180,8,.284,1.517,2800,59,29.92), 1.517, .284));
	for (s=0; s<=1000; s+=100) {
		printf("Yards: %.0f     Drop: %.2f		Wind: %.2f		Spin: %.2f		Total:%.2f		Time:%.2f\n", Ballistics_get_range(solution2, s), Ballistics_get_path(solution2, s),
			   Ballistics_get_windage(solution2, s), Ballistics_get_spindrift(solution2, s), Ballistics_get_corrected_windage(solution2, s), Ballistics_get_time(solution2,s));
	}


  Ballistics_free(solution);
	
	return 0;
}