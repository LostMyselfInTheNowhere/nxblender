/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2022 Blender Foundation. */

#pragma once

/** \file
 * \ingroup bli
 */

#include <cmath>
#include <type_traits>

#include "BLI_math_base.hh"
#include "BLI_math_vector_types.hh"
#include "BLI_span.hh"
#include "BLI_utildefines.h"

namespace blender::math {

template<typename T, int Size> [[nodiscard]] inline bool is_zero(const vec_base<T, Size> &a)
{
  for (int i = 0; i < Size; i++) {
    if (a[i] != T(0)) {
      return false;
    }
  }
  return true;
}

template<typename T, int Size> [[nodiscard]] inline bool is_any_zero(const vec_base<T, Size> &a)
{
  for (int i = 0; i < Size; i++) {
    if (a[i] == T(0)) {
      return true;
    }
  }
  return false;
}

template<typename T, int Size>
[[nodiscard]] inline bool almost_equal_relative(const vec_base<T, Size> &a,
                                                const vec_base<T, Size> &b,
                                                const T &epsilon_factor)
{
  for (int i = 0; i < Size; i++) {
    const float epsilon = epsilon_factor * math::abs(a[i]);
    if (math::distance(a[i], b[i]) > epsilon) {
      return false;
    }
  }
  return true;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> abs(const vec_base<T, Size> &a)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = a[i] >= 0 ? a[i] : -a[i];
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> min(const vec_base<T, Size> &a, const vec_base<T, Size> &b)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = a[i] < b[i] ? a[i] : b[i];
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> max(const vec_base<T, Size> &a, const vec_base<T, Size> &b)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = a[i] > b[i] ? a[i] : b[i];
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> clamp(const vec_base<T, Size> &a,
                                             const vec_base<T, Size> &min,
                                             const vec_base<T, Size> &max)
{
  vec_base<T, Size> result = a;
  for (int i = 0; i < Size; i++) {
    result[i] = std::clamp(result[i], min[i], max[i]);
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> clamp(const vec_base<T, Size> &a,
                                             const T &min,
                                             const T &max)
{
  vec_base<T, Size> result = a;
  for (int i = 0; i < Size; i++) {
    result[i] = std::clamp(result[i], min, max);
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> mod(const vec_base<T, Size> &a, const vec_base<T, Size> &b)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    BLI_assert(b[i] != 0);
    result[i] = std::fmod(a[i], b[i]);
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> mod(const vec_base<T, Size> &a, const T &b)
{
  BLI_assert(b != 0);
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = std::fmod(a[i], b);
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> safe_mod(const vec_base<T, Size> &a,
                                                const vec_base<T, Size> &b)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = (b[i] != 0) ? std::fmod(a[i], b[i]) : 0;
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> safe_mod(const vec_base<T, Size> &a, const T &b)
{
  if (b == 0) {
    return vec_base<T, Size>(0);
  }
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = std::fmod(a[i], b);
  }
  return result;
}

/**
 * Returns \a a if it is a multiple of \a b or the next multiple or \a b after \b a .
 * In other words, it is equivalent to `divide_ceil(a, b) * b`.
 * It is undefined if \a a is negative or \b b is not strictly positive.
 */
template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> ceil_to_multiple(const vec_base<T, Size> &a,
                                                        const vec_base<T, Size> &b)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    BLI_assert(a[i] >= 0);
    BLI_assert(b[i] > 0);
    result[i] = ((a[i] + b[i] - 1) / b[i]) * b[i];
  }
  return result;
}

/**
 * Integer division that returns the ceiling, instead of flooring like normal C division.
 * It is undefined if \a a is negative or \b b is not strictly positive.
 */
template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> divide_ceil(const vec_base<T, Size> &a,
                                                   const vec_base<T, Size> &b)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    BLI_assert(a[i] >= 0);
    BLI_assert(b[i] > 0);
    result[i] = (a[i] + b[i] - 1) / b[i];
  }
  return result;
}

template<typename T, int Size>
void min_max(const vec_base<T, Size> &vector, vec_base<T, Size> &min, vec_base<T, Size> &max)
{
  min = math::min(vector, min);
  max = math::max(vector, max);
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> safe_divide(const vec_base<T, Size> &a,
                                                   const vec_base<T, Size> &b)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = (b[i] == 0) ? 0 : a[i] / b[i];
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> safe_divide(const vec_base<T, Size> &a, const T &b)
{
  return (b != 0) ? a / b : vec_base<T, Size>(0.0f);
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> floor(const vec_base<T, Size> &a)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = std::floor(a[i]);
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> ceil(const vec_base<T, Size> &a)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = std::ceil(a[i]);
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> fract(const vec_base<T, Size> &a)
{
  vec_base<T, Size> result;
  for (int i = 0; i < Size; i++) {
    result[i] = a[i] - std::floor(a[i]);
  }
  return result;
}

template<typename T, int Size>
[[nodiscard]] inline T dot(const vec_base<T, Size> &a, const vec_base<T, Size> &b)
{
  T result = a[0] * b[0];
  for (int i = 1; i < Size; i++) {
    result += a[i] * b[i];
  }
  return result;
}

template<typename T, int Size> [[nodiscard]] inline T length_manhattan(const vec_base<T, Size> &a)
{
  T result = std::abs(a[0]);
  for (int i = 1; i < Size; i++) {
    result += std::abs(a[i]);
  }
  return result;
}

template<typename T, int Size> [[nodiscard]] inline T length_squared(const vec_base<T, Size> &a)
{
  return dot(a, a);
}

template<typename T, int Size> [[nodiscard]] inline T length(const vec_base<T, Size> &a)
{
  return std::sqrt(length_squared(a));
}

template<typename T, int Size> [[nodiscard]] inline bool is_unit_scale(const vec_base<T, Size> &v)
{
  /* Checks are flipped so NAN doesn't assert because we're making sure the value was
   * normalized and in the case we don't want NAN to be raising asserts since there
   * is nothing to be done in that case. */
  const T test_unit = math::length_squared(v);
  return (!(std::abs(test_unit - T(1)) >= AssertUnitEpsilon<T>::value) ||
          !(std::abs(test_unit) >= AssertUnitEpsilon<T>::value));
}

template<typename T, int Size>
[[nodiscard]] inline T distance_manhattan(const vec_base<T, Size> &a, const vec_base<T, Size> &b)
{
  return length_manhattan(a - b);
}

template<typename T, int Size>
[[nodiscard]] inline T distance_squared(const vec_base<T, Size> &a, const vec_base<T, Size> &b)
{
  return length_squared(a - b);
}

template<typename T, int Size>
[[nodiscard]] inline T distance(const vec_base<T, Size> &a, const vec_base<T, Size> &b)
{
  return length(a - b);
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> reflect(const vec_base<T, Size> &incident,
                                               const vec_base<T, Size> &normal)
{
  BLI_assert(is_unit_scale(normal));
  return incident - 2.0 * dot(normal, incident) * normal;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> refract(const vec_base<T, Size> &incident,
                                               const vec_base<T, Size> &normal,
                                               const T &eta)
{
  float dot_ni = dot(normal, incident);
  float k = 1.0f - eta * eta * (1.0f - dot_ni * dot_ni);
  if (k < 0.0f) {
    return vec_base<T, Size>(0.0f);
  }
  return eta * incident - (eta * dot_ni + sqrt(k)) * normal;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> project(const vec_base<T, Size> &p,
                                               const vec_base<T, Size> &v_proj)
{
  if (UNLIKELY(is_zero(v_proj))) {
    return vec_base<T, Size>(0.0f);
  }
  return v_proj * (dot(p, v_proj) / dot(v_proj, v_proj));
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> normalize_and_get_length(const vec_base<T, Size> &v,
                                                                T &out_length)
{
  out_length = length_squared(v);
  /* A larger value causes normalize errors in a scaled down models with camera extreme close. */
  constexpr T threshold = std::is_same_v<T, double> ? 1.0e-70 : 1.0e-35f;
  if (out_length > threshold) {
    out_length = sqrt(out_length);
    return v / out_length;
  }
  /* Either the vector is small or one of it's values contained `nan`. */
  out_length = 0.0;
  return vec_base<T, Size>(0.0);
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> normalize(const vec_base<T, Size> &v)
{
  T len;
  return normalize_and_get_length(v, len);
}

template<typename T>
[[nodiscard]] inline vec_base<T, 3> cross(const vec_base<T, 3> &a, const vec_base<T, 3> &b)
{
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

[[nodiscard]] inline vec_base<float, 3> cross_high_precision(const vec_base<float, 3> &a,
                                                             const vec_base<float, 3> &b)
{
  return {float(double(a.y) * double(b.z) - double(a.z) * double(b.y)),
          float(double(a.z) * double(b.x) - double(a.x) * double(b.z)),
          float(double(a.x) * double(b.y) - double(a.y) * double(b.x))};
}

template<typename T> [[nodiscard]] inline vec_base<T, 3> cross_poly(Span<vec_base<T, 3>> poly)
{
  /* Newell's Method. */
  int nv = int(poly.size());
  if (nv < 3) {
    return vec_base<T, 3>(0, 0, 0);
  }
  const vec_base<T, 3> *v_prev = &poly[nv - 1];
  const vec_base<T, 3> *v_curr = &poly[0];
  vec_base<T, 3> n(0, 0, 0);
  for (int i = 0; i < nv;) {
    n[0] = n[0] + ((*v_prev)[1] - (*v_curr)[1]) * ((*v_prev)[2] + (*v_curr)[2]);
    n[1] = n[1] + ((*v_prev)[2] - (*v_curr)[2]) * ((*v_prev)[0] + (*v_curr)[0]);
    n[2] = n[2] + ((*v_prev)[0] - (*v_curr)[0]) * ((*v_prev)[1] + (*v_curr)[1]);
    v_prev = v_curr;
    ++i;
    if (i < nv) {
      v_curr = &poly[i];
    }
  }
  return n;
}

template<typename T, typename FactorT, int Size>
[[nodiscard]] inline vec_base<T, Size> interpolate(const vec_base<T, Size> &a,
                                                   const vec_base<T, Size> &b,
                                                   const FactorT &t)
{
  return a * (1 - t) + b * t;
}

template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> midpoint(const vec_base<T, Size> &a,
                                                const vec_base<T, Size> &b)
{
  return (a + b) * 0.5;
}

/**
 * Return `vector` if `incident` and `reference` are pointing in the same direction.
 */
template<typename T, int Size>
[[nodiscard]] inline vec_base<T, Size> faceforward(const vec_base<T, Size> &vector,
                                                   const vec_base<T, Size> &incident,
                                                   const vec_base<T, Size> &reference)
{
  return (dot(reference, incident) < 0) ? vector : -vector;
}

template<typename T> [[nodiscard]] inline int dominant_axis(const vec_base<T, 3> &a)
{
  vec_base<T, 3> b = abs(a);
  return ((b.x > b.y) ? ((b.x > b.z) ? 0 : 2) : ((b.y > b.z) ? 1 : 2));
}

/**
 * Calculates a perpendicular vector to \a v.
 * \note Returned vector can be in any perpendicular direction.
 * \note Returned vector might not the same length as \a v.
 */
template<typename T> [[nodiscard]] inline vec_base<T, 3> orthogonal(const vec_base<T, 3> &v)
{
  const int axis = dominant_axis(v);
  switch (axis) {
    case 0:
      return {-v.y - v.z, v.x, v.x};
    case 1:
      return {v.y, -v.x - v.z, v.y};
    case 2:
      return {v.z, v.z, -v.x - v.y};
  }
  return v;
}

/**
 * Calculates a perpendicular vector to \a v.
 * \note Returned vector can be in any perpendicular direction.
 */
template<typename T> [[nodiscard]] inline vec_base<T, 2> orthogonal(const vec_base<T, 2> &v)
{
  return {-v.y, v.x};
}

template<typename T, int Size>
[[nodiscard]] inline bool is_equal(const vec_base<T, Size> &a,
                                   const vec_base<T, Size> &b,
                                   const T epsilon = T(0))
{
  for (int i = 0; i < Size; i++) {
    if (std::abs(a[i] - b[i]) > epsilon) {
      return false;
    }
  }
  return true;
}

/** Intersections. */

template<typename T> struct isect_result {
  enum {
    LINE_LINE_COLINEAR = -1,
    LINE_LINE_NONE = 0,
    LINE_LINE_EXACT = 1,
    LINE_LINE_CROSS = 2,
  } kind;
  typename T::base_type lambda;
};

template<typename T, int Size>
[[nodiscard]] isect_result<vec_base<T, Size>> isect_seg_seg(const vec_base<T, Size> &v1,
                                                            const vec_base<T, Size> &v2,
                                                            const vec_base<T, Size> &v3,
                                                            const vec_base<T, Size> &v4);

}  // namespace blender::math
