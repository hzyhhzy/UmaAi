#pragma once
#include <cmath>
#include <cassert>
//Beasley-Springer-Moro algorithm by ChatGPT
double normalCDFInverse(double p) {
  assert(p > 0 && p < 1);

  // Coefficients in rational approximations
  static const double a[] = {
      -3.969683028665376e+01,
       2.209460984245205e+02,
      -2.759285104469687e+02,
       1.383577518672690e+02,
      -3.066479806614716e+01,
       2.506628277459239e+00
  };
  static const double b[] = {
      -5.447609879822406e+01,
       1.615858368580409e+02,
      -1.556989798598866e+02,
       6.680131188771972e+01,
      -1.328068155288572e+01
  };
  static const double c[] = {
      -7.784894002430293e-03,
      -3.223964580411365e-01,
      -2.400758277161838e+00,
      -2.549732539343734e+00,
       4.374664141464968e+00,
       2.938163982698783e+00
  };
  static const double d[] = {
      7.784695709041462e-03,
      3.224671290700398e-01,
      2.445134137142996e+00,
      3.754408661907416e+00
  };

  // Define break-points.
  static const double plow = 0.02425;
  static const double phigh = 1 - plow;

  // Rational approximation for lower region:
  if (p < plow) {
    double q = sqrt(-2 * log(p));
    return (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1);
  }

  // Rational approximation for upper region:
  if (p > phigh) {
    double q = sqrt(-2 * log(1 - p));
    return -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1);
  }

  // Rational approximation for central region:
  if (p >= plow && p <= phigh) {
    double q = p - 0.5;
    double r = q * q;
    return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q / (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1);
  }

  // If we get here, something has gone wrong

  assert(false && "This should not happen!");
}

