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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "constants.h"
#include "angle.h"
#include "atmosphere.h"
#include "windage.h"
#include "pbr.h"

typedef struct Ballistics Ballistics;

// Functions for retrieving data from a solution generated with solve()
void Ballistics_free(Ballistics* ballistics);

// Returns range, in yards.
double Ballistics_get_range(Ballistics* ballistics, int yardage);
// Returns projectile path, in inches, relative to the line of sight.
double Ballistics_get_path(Ballistics* ballistics, int yardage);
// Returns an estimated elevation correction for achieving a zero at this range.
// this is useful for "click charts" and the like.
double Ballistics_get_moa(Ballistics* ballistics, int yardage);
// Returns the projectile's time of flight to this range.
double Ballistics_get_time(Ballistics* ballistics, int yardage);
// Returns the windage correction in inches required to achieve zero at this range.
double Ballistics_get_windage(Ballistics* ballistics, int yardage);
// Returns an approximate windage correction in MOA to achieve a zero at this range.
double Ballistics_get_windage_moa(Ballistics* ballistics, int yardage);
double Ballistics_get_spindrift(Ballistics* ballistics, int yardage);
 double Ballistics_get_windage_moa(Ballistics* ballistics, int yardage);
double Ballistics_get_corrected_windage(Ballistics* ballistics, int yardage);
double Ballistics_get_corrected_windage_moa(Ballistics* ballistics, int yardage);
// Returns the projectile's total velocity (Vector product of Vx and Vy)
double Ballistics_get_v_fps(Ballistics* ballistics, int yardage);
// Returns the velocity of the projectile in the bore direction.
double Ballistics_get_vx_fps(Ballistics* ballistics, int yardage);
// Returns the velocity of the projectile perpendicular to the bore direction.
double Ballistics_get_vy_fps(Ballistics* ballistics, int yardage);
 /**
 * 30m
 * ____
 * t^2 * d^3 * l(1+l^2)
 *
 * where:
 * m is bullet mass in grains
 * t is the rifling twist in calibers per turn
 * d is the diameter (caliber) of bullet in inches
 * l is length of the bullet in calibers
 *
 * page 428 of applied ballistics
 */
double calculateGS(double bulletGrains, double twistDenominator, double caliber, double lengthOfBullet, double velocity, double temp, double inHg);
/**
 * \brief calculates vertical deflection, in MOA, of a projectile
 * \param gs
 * \param length
 * \param caliber
 * \return
 */
double calculateVerticalDeflection(double gs, double length, double caliber);

// For very steep shooting angles, vx can actually become what you would think of as vy relative to the ground,
// because vx is referencing the bore's axis.  All computations are carried out relative to the bore's axis, and
// have very little to do with the ground's orientation.

/**
 * A function to generate a ballistic solution table in 1 yard increments, up to BCOMP_MAXRANGE.
 * @param drag_function    G1, G2, G3, G5, G6, G7, or G8
 * @param drag_coefficient The coefficient of drag for the projectile you wish to model.
 * @param vi               The projectile initial velocity.
 * @param sight_height     The height of the sighting system above the bore centerline.
 *                         Most scopes are in the 1.5"-2.0" range.
 * @param shooting_angle   The uphill or downhill shooting angle, in degrees.  Usually 0, but can be anything from
 *                         90 (directly up), to -90 (directly down).
 * @param zero_angle       The angle of the sighting system relative to the bore, in degrees.  This can be easily computed
 *                         using the ZeroAngle() function documented above.
 * @param wind_speed       The wind velocity, in mi/hr
 * @param wind_angle       The angle at which the wind is approaching from, in degrees.
 *                         0 degrees is a straight headwind
 *                         90 degrees is from right to left
 *                         180 degrees is a straight tailwind
 *                         -90 or 270 degrees is from left to right.
 * @param ballistics       A pointer provided for accessing the solution after it has been generated.
 *                         Memory for this pointer is allocated in the function, so the user does not need
 *                         to worry about it.  This solution can be passed to the retrieval functions to get
 *                         useful data from the solution.
 * @return This function returns an integer representing the maximum valid range of the
 *         solution.  This also indicates the maximum number of rows in the solution matrix,
 *         and should not be exceeded in order to avoid a memory segmentation fault.
 */
int Ballistics_solve(Ballistics** ballistics, DragFunction drag_function, double drag_coefficient, double vi,
                     double sight_height, double shooting_angle, double zero_angle, double wind_speed, double wind_angle);

/**
 * \brief Vertical deflection and spindrift compensated version of the ballistics solver.
 * \param ballistics
 * \param drag_function
 * \param drag_coefficient
 * \param vi
 * \param sight_height
 * \param shooting_angle
 * \param zero_angle
 * \param wind_speed
 * \param wind_angle
 * \param caliberInInches
 * \param bulletLengthInInches
 * \param temp
 * \param inHg
 * \param twistDenominator
 * \param velocity
 * \param bulletGrains
 * \param formFactor
 * \return
 */
int Ballistics_solve_modified_vertDeflect(Ballistics** ballistics, DragFunction drag_function, double drag_coefficient, double vi,
                                          double sight_height, double shooting_angle, double zero_angle, double wind_speed, double wind_angle, double caliberInInches, double bulletLengthInInches, double temp, double inHg, double twistDenominator, double velocity, double bulletGrains, double formFactor);

/**
 * \brief calculates spin drift offset
 * \param gs
 * \param tof
 * \return
 */
double calculateSpinDriftOffsetIn(double gs, double tof);
#ifdef __cplusplus
} // extern "C"
#endif