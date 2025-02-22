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

#include "ballistics/ballistics.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/**
 * A ballistics solution for a projectile at a certain yardage.
 */
typedef struct {
  double range_yards;
  double path_inches;
  double moa_correction;
  double seconds;
  double windage_inches;
  double spindrift_inches;
  double corrected_windage;
  double windage_moa;
  double corrected_windage_moa;
  double v_fps; // total velocity -> vector product of vx and vy
  double vx_fps; // velocity of projectile in the bore direction
  double vy_fps; // velocity of projectile perpendicular to the bore direction
} Point;

struct Ballistics {
  Point *yardages;
  int max_yardage;
};

Ballistics* Ballistics_alloc() {
  Ballistics* sln = malloc(sizeof(Ballistics));
  sln->yardages = malloc(sizeof(Point) * BALLISTICS_COMPUTATION_MAX_YARDS);
  return sln;
}

void Ballistics_free(Ballistics* ballistics) {
  free(ballistics->yardages);
  free(ballistics);
}

double Ballistics_get_range(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].range_yards;
  }
  else return 0;
}

double Ballistics_get_path(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].path_inches;
  }
  else return 0;
}

double Ballistics_get_moa(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].moa_correction;
  }
  else return 0;
}


double Ballistics_get_time(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].seconds;
  }
  else return 0;
}

double Ballistics_get_windage(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].windage_inches;
  }
  else return 0;
}
double Ballistics_get_spindrift(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].spindrift_inches;
  }
  else return 0;
}

double Ballistics_get_windage_moa(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].windage_moa;
  }
  else return 0;
}
double Ballistics_get_corrected_windage(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].corrected_windage;
  }
  else return 0;
}

double Ballistics_get_corrected_windage_moa(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].corrected_windage_moa;
  }
  else return 0;
}
double Ballistics_get_v_fps(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].v_fps;
  }
  else return 0;
}

double Ballistics_get_vx_fps(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].vx_fps;
  }
  else return 0;
}

double Ballistics_get_vy_fps(Ballistics* ballistics, int yardage) {
  if (yardage < ballistics->max_yardage) {
    return ballistics->yardages[yardage].vy_fps;
  }
  else return 0;
}

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
 */
double calculateGS(double bulletGrains, double twistDenominator, double caliber, double lengthOfBullet, double velocity, double temp, double inHg) {
  double m = bulletGrains;
  double t = twistDenominator / caliber;
  double d = caliber;
  double l = lengthOfBullet / caliber;

  double uncorrectedGS = ((30 * m) / (
  (pow(t,2))*
  (pow(d,3))*
  (
  l * (1 + pow(l,2))
  )
  ));
  double veloCorrection = velocity/(double)2800;
  veloCorrection = cbrt(veloCorrection);
  double tempCorrection = (
  ((temp + 460) * 29.92)
  /
  ((59+460) * inHg)
  );

  return uncorrectedGS * veloCorrection * tempCorrection;
}

/**
 * \brief calculates vertical deflection, in MOA, of a projectile
 * \param gs
 * \param length
 * \param caliber
 * \return
 */
double calculateVerticalDeflection(double gs, double length, double caliber) {
  return (.01 * gs - .0024 * (length / caliber) + .032);
}

/**
 * \brief calculates spin dirft offset in inches. It is negated because we look down the barrel, not out
 *
 * \param gs gyro stability factor
 * \param tof time of flight
 * \return the inches offset
 */
double calculateSpinDriftOffsetIn(double gs, double tof) {
  return -(1.25*(gs+1.2)*pow(tof,1.83));
}

int Ballistics_solve(Ballistics** ballistics, DragFunction drag_function, double drag_coefficient, double vi,
                     double sight_height, double shooting_angle, double zero_angle, double wind_speed, double wind_angle) {
  double t=0;
  double dt=0;
  double v=0;
  double vx=0, vx1=0, vy=0, vy1=0;
  double dv=0, dvx=0, dvy=0;
  double x=0, y=0;

  double hwind = headwind(wind_speed, wind_angle);
  double cwind = crosswind(wind_speed, wind_angle);

  double gy = GRAVITY*cos(deg_to_rad((shooting_angle + zero_angle)));
  double gx = GRAVITY*sin(deg_to_rad((shooting_angle + zero_angle)));

  *ballistics = Ballistics_alloc();

  vx = vi * cos(deg_to_rad(zero_angle));
  vy = vi * sin(deg_to_rad(zero_angle));

  y = -sight_height/12; // y is in feet

  int n = 0;
  for (t = 0;; t = t + dt) {
    vx1 = vx;
    vy1 = vy;
    v = pow(pow(vx,2)+pow(vy,2),0.5);
    dt = 0.5/v;

    // Compute acceleration using the drag function retardation  
    dv = retard(drag_function, drag_coefficient, v+hwind);
    dvx = -(vx/v)*dv;
    dvy = -(vy/v)*dv;

    // Compute velocity, including the resolved gravity vectors.  
    vx = vx + dt*dvx + dt*gx;
    vy = vy + dt*dvy + dt*gy;

    if (x/3 >= n) {
      Point* s = &(*ballistics)->yardages[n];
      s->range_yards = x/3;
      s->path_inches = y*12;
      s->moa_correction = -rad_to_moa(atan(y / x));
      s->seconds = t+dt;
      s->windage_inches = windage(cwind, vi, x, t + dt);
      s->windage_moa = rad_to_moa(atan((s->windage_inches/12) / x));
      s->v_fps = v;
      s->vx_fps = vx;
      s->vy_fps = vy;
      n++;
    }

    // Compute position based on average velocity.
    x = x + dt * (vx+vx1)/2;
    y = y + dt * (vy+vy1)/2;

    if (fabs(vy)>fabs(3*vx) || n>=BALLISTICS_COMPUTATION_MAX_YARDS) break;
  }

  (*ballistics)->max_yardage = n;
  return n;
}

int Ballistics_solve_modified_vertDeflect(Ballistics** ballistics, DragFunction drag_function, double drag_coefficient, double vi,
                     double sight_height, double shooting_angle, double zero_angle, double wind_speed, double wind_angle, double caliberInInches, double bulletLengthInInches, double temp, double inHg, double twistDenominator, double velocity, double bulletGrains, double formFactor) {
  double t=0;
  double dt=0;
  double v=0;
  double vx=0, vx1=0, vy=0, vy1=0;
  double dv=0, dvx=0, dvy=0;
  double x=0, y=0;

  double hwind = headwind(wind_speed, wind_angle);
  double cwind = crosswind(wind_speed, wind_angle);

  double gy = GRAVITY*cos(deg_to_rad((shooting_angle + zero_angle)));
  double gx = GRAVITY*sin(deg_to_rad((shooting_angle + zero_angle)));

  *ballistics = Ballistics_alloc();

  vx = vi * cos(deg_to_rad(zero_angle));
  vy = vi * sin(deg_to_rad(zero_angle));

  y = -sight_height/12; // y is in feet

  int n = 0;
  for (t = 0;; t = t + dt) {
    vx1 = vx;
    vy1 = vy;
    v = pow(pow(vx,2)+pow(vy,2),0.5);
    dt = 0.5/v;

    // Compute acceleration using the drag function retardation
    dv = retardModified(drag_function, drag_coefficient, v+hwind, formFactor);
    dvx = -(vx/v)*dv;
    dvy = -(vy/v)*dv;

    // Compute velocity, including the resolved gravity vectors.
    vx = vx + dt*dvx + dt*gx;
    vy = vy + dt*dvy + dt*gy;
    double currentGs = calculateGS(bulletGrains, twistDenominator, caliberInInches, bulletLengthInInches, v, temp, inHg);
    double windDeflectionOffsetMOA = calculateVerticalDeflection(currentGs, bulletLengthInInches, caliberInInches) * cwind;
    double windDeflectionOffsetRad = windDeflectionOffsetMOA * (M_PI / (180.0 * 60.0));
    if (x/3 >= n) {
      Point* s = &(*ballistics)->yardages[n];
      s->range_yards = x/3;
      s->path_inches = y*12;
      s->path_inches += tan(windDeflectionOffsetRad) * x;

      s->moa_correction = -rad_to_moa(atan(y / x)) + windDeflectionOffsetMOA;
      s->seconds = t+dt;
      s->spindrift_inches = calculateSpinDriftOffsetIn(currentGs, s->seconds);
      s->windage_inches = windage(cwind, vi, x, t + dt);
      s->windage_moa = rad_to_moa(atan((s->windage_inches/12) / x));
      s->corrected_windage = s->windage_inches + s->spindrift_inches;
      s->corrected_windage_moa = rad_to_moa(atan((s->corrected_windage/12) / x));
      s->v_fps = v;
      s->vx_fps = vx;
      s->vy_fps = vy;
      n++;
    }

    // Compute position based on average velocity.
    x = x + dt * (vx+vx1)/2;
    y = y + dt * (vy+vy1)/2;

    if (fabs(vy)>fabs(3*vx) || n>=BALLISTICS_COMPUTATION_MAX_YARDS) break;
  }

  (*ballistics)->max_yardage = n;
  return n;
}
