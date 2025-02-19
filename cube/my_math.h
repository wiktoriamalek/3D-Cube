/*
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <immintrin.h>
#include <concepts>
#include <cmath>

#include "Utils.hpp"

#undef max
#undef min
#undef Max
#undef Min
#undef near
#undef far

// Convention: RH, Y UP

namespace lib
{
	template <typename T> concept be_number = std::integral<T> || std::floating_point<T>;

	inline constexpr f32 deg_to_rad(const f32 degrees)
	{
		return degrees * PI32 / 180.0f;
	}

	inline constexpr f32 rad_to_deg(const f32 radians)
	{
		return radians * 180.0f / PI32;
	}

	// Basically taken from XMScalarModAngle in DirectXMath, its modulo between -PI to PI
	inline f32 mod_pi(f32 angle)
	{
		constexpr f32 pi32_2 = 2.0f * PI32;
		angle = angle + PI32;
		// Perform the modulo, unsigned
		float f_t = fabsf(angle);
		f_t = f_t - (pi32_2 * (f32)((s32)(f_t / pi32_2)));
		// Restore the number to the range of -PI to PI-epsilon
		f_t = f_t - PI32;
		// If the modulo'd value was negative, restore negation
		if (angle < 0.0f)
		{
			f_t = -f_t;
		}
		return f_t;
	}

	inline f32 sqrt(const f32 f)
	{
		__m128 temp = _mm_set_ss(f);
		temp = _mm_sqrt_ss(temp);

		return _mm_cvtss_f32(temp);
	}

	inline f32 rsqrt(const f32 f)
	{
		__m128 temp = _mm_set_ss(f);
		temp = _mm_rsqrt_ss(temp);

		return _mm_cvtss_f32(temp);
	}

	inline s32 ceil(const f32 f)
	{
		return _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(f), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC));
	}

	inline s32 floor(const f32 f)
	{
		return _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(f), _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC));
	}

	inline s32 round(const f32 f)
	{
		return _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(f), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
	}

	inline s32 trunc(const f32 f)
	{
		return _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(f), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC));
	}

	//? Naive solutions produce better assembly in optimized than SIMD version for clamp, min, max, abs, lerp

	template<be_number T>
	inline T clamp(T val, T min, T max)
	{
		const T t = val < min ? min : val;
		return t > max ? max : t;
	}

	template<be_number T>
	inline T min(T a, T b)
	{
		return a < b ? a : b;
	}

	template<be_number T>
	inline T max(T a, T b)
	{
		return a > b ? a : b;
	}

	template <be_number T>
	inline T mod(T a, T b)
	{
		return (a % b + b) % b;
	}

	template<be_number T>
	inline T abs(T val)
	{
		return val > 0 ? val : -val;
	}

	//! Be aware that below implementations are not clamped, and not checked for division by 0!
	//? MSVC fnma+fma combo, Clang sub+fma - both are good
	template<be_number T>
	inline T lerp(T a, T b, T val)
	{
		return a * (1 - val) + (b * val);
	}

	template<be_number T>
	inline T inv_lerp(T a, T b, T val)
	{
		return (val - a) / (b - a);
	}

	template<be_number T>
	inline T remap_range(T in_min, T in_max, T out_min, T out_max, T val)
	{
		constexpr T temp = inv_lerp(in_min, in_max, val);
		return lerp(out_min, out_max, temp);
	}

	union Vec2
	{
		struct
		{
			f32 x, y;
		};

		struct
		{
			f32 width, height;
		};

		struct
		{
			f32 u, v;
		};

		f32 e[2];

		inline Vec2 operator-() const { return Vec2{ -x, -y }; }
		inline const f32& operator[](s32 i) const { return e[i]; }
		inline f32& operator[](s32 i) { return e[i]; }
		inline Vec2& operator/=(const f32 t) { return *this *= 1.0f / t; }

		inline Vec2& operator+=(const Vec2& b)
		{
			x += b.x;
			y += b.y;

			return *this;
		}

		inline Vec2& operator*=(const f32 t)
		{
			x *= t;
			y *= t;

			return *this;
		}
	};

	inline Vec2 operator+(const Vec2 a, const Vec2 b)
	{
		return Vec2{ a.x + b.x, a.y + b.y };
	}

	inline Vec2 operator-(const Vec2 a, const Vec2 b)
	{
		return Vec2{ a.x - b.x, a.y - b.y };
	}

	inline Vec2 operator*(const Vec2 a, const Vec2 b)
	{
		return Vec2{ a.x * b.x, a.y * b.y };
	}

	inline Vec2 operator*(const f32 t, const Vec2 b)
	{
		return Vec2{ t * b.x, t * b.y };
	}

	inline Vec2 operator*(const Vec2 b, const f32 t)
	{
		return t * b;
	}

	inline Vec2 operator/(const Vec2 b, const f32 t)
	{
		return (1.0f / t) * b;
	}

	//! Be aware that floats are rarely(never) perfectly equal
	//TODO: Consider using FLT_EPSILON to get somewhat accurate aprroximation 
	inline b32 operator==(const Vec2 a, const Vec2 b)
	{
		return { a.x == b.x && a.y == b.y };
	}

	inline f32 dot(const Vec2 a, const Vec2 b)
	{
		return (a.x * b.x)
			+ (a.y * b.y);
	}

	inline f32 length_vec(const Vec2 a)
	{
		return sqrt((a.x * a.x) + (a.y * a.y));
	}

	inline f32 length_squared_vec(const Vec2 a)
	{
		return (a.x * a.x) + (a.y * a.y);
	}

	inline Vec2 normalize(const Vec2 a)
	{
		Vec2 out{};

		f32 length = length_vec(a);
		if (length != 0.0f)
		{
			f32 multi = 1.0f / length;
			out = { a.x * multi, a.y * multi };
		}

		return out;
	}

	inline Vec2 normalize_fast(const Vec2 a)
	{
		return a * rsqrt(dot(a, a));
	}

	inline Vec2 perp(const Vec2 b)
	{
		return Vec2{ -b.y, b.x };
	}

	// https://mathworld.wolfram.com/PerpDotProduct.html
	// https://mathworld.wolfram.com/CrossProduct.html
	inline f32 perp_dot(const Vec2 a, const Vec2 b)
	{
		return (a.x * b.y) - (a.y * b.x);
	}

	inline Vec2 reflect(const Vec2 a, const Vec2 b)
	{
		return a - ((2.0f * b) * dot(a, b));
	}

	//? Normalized vectors assumed
	// http://www.cse.chalmers.se/edu/year/2013/course/TDA361/refractionvector.pdf
	// https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
	inline Vec2 refract(const Vec2 a, const Vec2 b, const f32 ratio)
	{
		Vec2 out{};

		f32 cos_i = -dot(a, b);
		f32 sin_t = (ratio * ratio) * (1.0f - (cos_i * cos_i));

		if (sin_t <= 1.0f) // check for total internal reflection (TIR)
			out = (ratio * a) + ((ratio * cos_i - sqrt(1.0f - sin_t)) * b);

		return out;
	}

	//! WIP, do not call this!
	// https://shaderbits.com/blog/optimized-snell-s-law-refraction
	inline Vec2 refract_fast(const Vec2 a, const Vec2 b, const f32 ratio)
	{
		//TODO: This will be specialized implementation for known materials (eg. only air - water refraction)
		__debugbreak();
	}

	inline Vec2 project(const Vec2 a, const Vec2 b)
	{
		return (dot(a, b) / dot(b, b)) * b;
	}

	//? for normalized vectors
	inline Vec2 project_norm(const Vec2 a, const Vec2 b)
	{
		return dot(a, b) * b;
	}

	inline f32 project_length(const Vec2 a, const Vec2 b)
	{
		return dot(a, b) / length_vec(b);
	}

	inline Vec2 reject(const Vec2 a, const Vec2 b)
	{
		return a - (dot(a, b) / dot(b, b)) * b;
	}

	//? for normalized vectors
	inline Vec2 reject_norm(const Vec2 a, const Vec2 b)
	{
		return a - dot(a, b) * b;
	}

	inline f32 reject_length(const Vec2 a, const Vec2 b)
	{
		return fabs(perp_dot(a, b)) / length_vec(b);
	}

	union Vec3
	{
		struct
		{
			f32 x, y, z;
		};

		struct
		{
			f32 r, g, b;
		};

		struct
		{
			f32 u, v, w;
		};

		f32 e[3];

		inline Vec3 operator-() const { return Vec3{ -x, -y, -z }; }
		inline const f32& operator[](s32 i) const { return e[i]; }
		inline f32& operator[](s32 i) { return e[i]; }
		inline Vec3& operator/=(const f32 t) { return *this *= 1.0f / t; }

		inline Vec3& operator+=(const Vec3 other)
		{
			x += other.x;
			y += other.y;
			z += other.z;

			return *this;
		}

		inline Vec3& operator*=(const f32 t)
		{
			x *= t;
			y *= t;
			z *= t;

			return *this;
		}
	};

	inline Vec3 operator+(const Vec3 a, const Vec3 b)
	{
		return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
	}

	inline Vec3 operator-(const Vec3 a, const Vec3 b)
	{
		return Vec3{ a.x - b.x, a.y - b.y, a.z - b.z };
	}

	inline Vec3 operator*(const Vec3 a, const Vec3 b)
	{
		return Vec3{ a.x * b.x, a.y * b.y, a.z * b.z };
	}

	inline Vec3 operator*(const f32 t, const Vec3 b)
	{
		return Vec3{ t * b.x, t * b.y, t * b.z };
	}

	inline Vec3 operator*(const Vec3 b, const f32 t)
	{
		return t * b;
	}

	inline Vec3 operator/(const Vec3 b, const f32 t)
	{
		return (1.0f / t) * b;
	}

	//! Be aware that floats are rarely perfectly equal
	inline b32 operator==(const Vec3 a, const Vec3 b)
	{
		return { a.x == b.x && a.y == b.y && a.z == b.z };
	}

	inline f32 dot(const Vec3 a, const Vec3 b)
	{
		return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	}

	inline f32 length_vec(const Vec3 a)
	{
		return sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
	}

	inline f32 length_squared_vec(const Vec3 a)
	{
		return (a.x * a.x) + (a.y * a.y) + (a.z * a.z);
	}

	inline Vec3 normalize(const Vec3 a)
	{
		Vec3 out{};

		f32 length = length_vec(a);
		if (length != 0.0f)
		{
			f32 multi = 1.0f / length;
			out = { a.x * multi, a.y * multi, a.z * multi };
		}
		return out;
	}

	inline Vec3 normalize_fast(const Vec3 a)
	{
		return a * rsqrt(dot(a, a));
	}

	inline Vec3 cross(const Vec3 a, const Vec3 b)
	{
		return Vec3{ (a.y * b.z) - (a.z * b.y),
									(a.z * b.x) - (a.x * b.z),
									(a.x * b.y) - (a.y * b.x) };
	}

	inline Vec3 reflect(const Vec3 a, const Vec3 b)
	{
		return a - (2.0f * b * dot(a, b));
	}

	inline Vec3 refract(const Vec3 a, const Vec3 b, const f32 ratio)
	{
		Vec3 out{};

		f32 cos_i = -dot(a, b);
		f32 sin_t = (ratio * ratio) * (1.0f - (cos_i * cos_i));

		if (sin_t <= 1.0f) // check for total internal reflection (TIR)
			out = (ratio * a) + ((ratio * cos_i - sqrt(1.0f - sin_t)) * b);

		return out;
	}

	inline Vec3 project(const Vec3 a, const Vec3 b)
	{
		return (dot(a, b) / dot(b, b)) * b;
	}

	//? for normalized vectors
	inline Vec3 project_norm(const Vec3 a, const Vec3 b)
	{
		return dot(a, b) * b;
	}

	inline f32 project_length(const Vec3 a, const Vec3 b)
	{
		return dot(a, b) / length_vec(b);
	}

	inline Vec3 reject(const Vec3 a, const Vec3 b)
	{
		return a - (dot(a, b) / dot(b, b)) * b;
	}

	//? for normalized vectors
	inline Vec3 reject_norm(const Vec3 a, const Vec3 b)
	{
		return a - dot(a, b) * b;
	}

	inline f32 reject_length(const Vec3 a, const Vec3 b)
	{
		return length_vec(cross(a, b)) / length_vec(b);
	}

	//? For 3D CG, Vec4 is assumed to behave like Vec3 in homogeneous space, 
	//? therefore its fourth component must be 0 to yield proper results from most operations in CG
	//? For operations with Mat4 user should choose wheter Vec4 represents point (w=1) or vector (w=0)
	union alignas(__m128) Vec4
	{
		struct
		{
			Vec3 xyz;
			f32 w;
		};

		struct
		{
			f32 x, y, z, w;
		};

		struct
		{
			Vec3 rgb;
			f32 a;
		};

		struct
		{
			f32 r, g, b, a;
		};

		f32 e[4];
		__m128 simd;

		inline Vec4 operator-() const { return Vec4{ -x, -y, -z, -w }; }
		inline const f32& operator[](s32 i) const { return e[i]; }
		inline f32& operator[](s32 i) { return e[i]; }
		inline Vec4& operator/=(const f32 t)
		{
			__m128 single = _mm_set_ps1(t);
			this->simd = _mm_div_ps(this->simd, single);

			return *this;
		}

		Vec4& operator+=(const Vec4 other)
		{
			this->simd = _mm_add_ps(this->simd, other.simd);

			return *this;
		}

		Vec4& operator*=(const f32 t)
		{
			__m128 single = _mm_set_ps1(t);
			this->simd = _mm_mul_ps(this->simd, single);

			return *this;
		}
	};

	inline Vec4 operator+(const Vec4 a, const Vec4 b)
	{
		return Vec4{ .simd = _mm_add_ps(a.simd, b.simd) };
	}

	inline Vec4 operator-(const Vec4 a, const Vec4 b)
	{
		return Vec4{ .simd = _mm_sub_ps(a.simd, b.simd) };
	}

	inline Vec4 operator*(const Vec4 a, const Vec4 b)
	{
		return Vec4{ .simd = _mm_mul_ps(a.simd, b.simd) };
	}

	inline Vec4 operator*(const f32 t, const Vec4 b)
	{
		__m128 single = _mm_set_ps1(t);
		return Vec4{ .simd = _mm_mul_ps(b.simd, single) };
	}

	inline Vec4 operator*(const Vec4 b, const f32 t)
	{
		__m128 single = _mm_set_ps1(t);
		return Vec4{ .simd = _mm_mul_ps(b.simd, single) };
	}

	inline Vec4 operator/(const Vec4 a, const Vec4 b)
	{
		return Vec4{ .simd = _mm_div_ps(a.simd, b.simd) };
	}

	inline Vec4 operator/(const Vec4 b, const f32 t)
	{
		__m128 single = _mm_set_ps1(t);
		return Vec4{ .simd = _mm_div_ps(b.simd, single) };
	}

	//! Be aware that floats are rarely perfectly equal
	inline b32 operator==(const Vec4 a, const Vec4 b)
	{
		return { a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w };
	}

	//? Scalar is faster for single cases like this function would be used, _mm_dp_ps is slower also
	//? More dot products (like 4) at once is where SIMD would be beneficial
	inline f32 dot(const Vec4 a, const Vec4 b)
	{
		return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
	}

	inline f32 length_vec(const Vec4 a)
	{
		return sqrt(dot(a, a));
	}

	inline f32 length_squared_vec(const Vec4 a)
	{
		return dot(a, a);
	}

	inline Vec4 normalize(const Vec4 a)
	{
		Vec4 out{};

		f32 length = length_vec(a);
		if (length != 0.0f)
		{
			__m128 single = _mm_set_ps1(1.0f / length);
			out.simd = _mm_mul_ps(a.simd, single);
		}

		return out;
	}

	inline Vec4 normalize_fast(const Vec4 a)
	{
		return a * rsqrt(dot(a, a));
	}

	//? More effective method compared to what I had previously (4 shuffles). 
	//? From: https://geometrian.com/programming/tutorials/cross-product/index.php
	inline Vec4 cross(const Vec4 a, const Vec4 b)
	{
		__m128 tmp0 = _mm_shuffle_ps(a.simd, a.simd, _MM_SHUFFLE(3, 0, 2, 1));
		__m128 tmp1 = _mm_shuffle_ps(b.simd, b.simd, _MM_SHUFFLE(3, 1, 0, 2));
		__m128 tmp2 = _mm_mul_ps(tmp0, b.simd);
		__m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
		__m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));

		return Vec4{ .simd = _mm_sub_ps(tmp3, tmp4) };
	}

	inline Vec4 reflect(const Vec4 a, const Vec4 b)
	{
		return a - (2.0f * b * dot(a, b));
	}

	inline Vec4 refract(const Vec4 a, const Vec4 b, const f32 ratio)
	{
		Vec4 out{};
		f32 cos_i = -dot(a, b);
		f32 sin_t = (ratio * ratio) * (1.0f - (cos_i * cos_i));

		if (sin_t <= 1.0f) // check for total internal reflection (TIR)
			out = (ratio * a) + ((ratio * cos_i - sqrt(1.0f - sin_t)) * b);

		return out;
	}

	inline Vec4 project(const Vec4 a, const Vec4 b)
	{
		return (dot(a, b) / dot(b, b)) * b;
	}

	//? for normalized vectors
	inline Vec4 project_norm(const Vec4 a, const Vec4 b)
	{
		return dot(a, b) * b;
	}

	inline f32 project_length(const Vec4 a, const Vec4 b)
	{
		return dot(a, b) / length_vec(b);
	}

	inline Vec4 reject(const Vec4 a, const Vec4 b)
	{
		return a - (dot(a, b) / dot(b, b)) * b;
	}

	//? for normalized vectors
	inline Vec4 reject_norm(const Vec4 a, const Vec4 b)
	{
		return a - dot(a, b) * b;
	}

	inline f32 reject_length(const Vec4 a, const Vec4 b)
	{
		return  length_vec(cross(a, b)) / length_vec(b);
	}

	//? Column major matrix (each memory row is one column)
	struct alignas(__m128) Mat4
	{
		union
		{
			f32 e[4][4];
			__m128 columns[4];
			Vec4 vecs[4];
		};

		inline Mat4 operator-() const
		{
			Mat4 out{};

			out.columns[0] = _mm_xor_ps(columns[0], _mm_set_ps1(-0.f));
			out.columns[1] = _mm_xor_ps(columns[1], _mm_set_ps1(-0.f));
			out.columns[2] = _mm_xor_ps(columns[2], _mm_set_ps1(-0.f));
			out.columns[3] = _mm_xor_ps(columns[3], _mm_set_ps1(-0.f));

			return out;
		}

		//? overloaded () for accessing by math notation
		inline f32& operator ()(const s32 row, const s32 column)
		{
			return (e[column][row]);
		}

		inline const f32& operator ()(const s32 row, const s32 column) const
		{
			return (e[column][row]);
		}

		inline const Vec4& operator[](const s32 column) const { return vecs[column]; }
		inline Vec4& operator[](const s32 column) { return vecs[column]; }
	};

	inline Mat4 operator+(const Mat4 a, const Mat4 b)
	{
		Mat4 out{};

		out.columns[0] = _mm_add_ps(a.columns[0], b.columns[0]);
		out.columns[1] = _mm_add_ps(a.columns[1], b.columns[1]);
		out.columns[2] = _mm_add_ps(a.columns[2], b.columns[2]);
		out.columns[3] = _mm_add_ps(a.columns[3], b.columns[3]);

		return out;
	}

	inline Mat4 operator-(const Mat4 a, const Mat4 b)
	{
		Mat4 out{};

		out.columns[0] = _mm_sub_ps(a.columns[0], b.columns[0]);
		out.columns[1] = _mm_sub_ps(a.columns[1], b.columns[1]);
		out.columns[2] = _mm_sub_ps(a.columns[2], b.columns[2]);
		out.columns[3] = _mm_sub_ps(a.columns[3], b.columns[3]);

		return out;
	}

	//? Helper for matrix multiplications - basically multiplication of matrix4 with vector4 by 
	//? multiplying each column (left to right) on the left with single vec element (top to bottom)
	inline __m128 linear_combination(const Mat4 a, __m128 b)
	{
		__m128 out{};

		out = _mm_mul_ps(a.columns[0], _mm_shuffle_ps(b, b, _MM_SHUFFLE(0, 0, 0, 0)));
		out = _mm_add_ps(out, _mm_mul_ps(a.columns[1], _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 1, 1, 1))));
		out = _mm_add_ps(out, _mm_mul_ps(a.columns[2], _mm_shuffle_ps(b, b, _MM_SHUFFLE(2, 2, 2, 2))));
		out = _mm_add_ps(out, _mm_mul_ps(a.columns[3], _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 3, 3, 3))));

		return out;
	}

	inline Vec4 operator*(const Mat4 a, const Vec4 b)
	{
		return { .simd = linear_combination(a, b.simd) };
	}

	inline Mat4 operator*(const Mat4 a, const Mat4 b)
	{
		Mat4 out{};

		out.columns[0] = linear_combination(a, b.columns[0]);
		out.columns[1] = linear_combination(a, b.columns[1]);
		out.columns[2] = linear_combination(a, b.columns[2]);
		out.columns[3] = linear_combination(a, b.columns[3]);

		return out;
	}

	inline Mat4 operator*(const f32 t, const Mat4 b)
	{
		Mat4 out{};
		__m128 temp = _mm_set_ps1(t);
		out.columns[0] = _mm_mul_ps(temp, b.columns[0]);
		out.columns[1] = _mm_mul_ps(temp, b.columns[1]);
		out.columns[2] = _mm_mul_ps(temp, b.columns[2]);
		out.columns[3] = _mm_mul_ps(temp, b.columns[3]);

		return out;
	}

	inline Mat4 operator*(const Mat4 a, const f32 t)
	{
		return t * a;
	}

	inline Mat4 operator/(const Mat4 a, const f32 t)
	{
		Mat4 out{};
		__m128 temp = _mm_set_ps1(t);
		out.columns[0] = _mm_div_ps(a.columns[0], temp);
		out.columns[1] = _mm_div_ps(a.columns[1], temp);
		out.columns[2] = _mm_div_ps(a.columns[2], temp);
		out.columns[3] = _mm_div_ps(a.columns[3], temp);

		return out;
	}

	inline Mat4& operator/=(Mat4& a, const f32 t)
	{
		return a = a / t;
	}

	inline Mat4& operator+=(Mat4& a, const Mat4 b)
	{
		return a = a + b;
	}

	inline Mat4& operator*=(Mat4& a, const f32 t)
	{
		return a = a * t;
	}

	inline Mat4 transpose(Mat4 a)
	{
		Mat4 out = a;
		_MM_TRANSPOSE4_PS(out.columns[0], out.columns[1], out.columns[2], out.columns[3]);
		return out;
	}

	[[nodiscard]]
	inline Mat4 create_diagonal_matrix(const f32 val = 1.0f)
	{
		Mat4 out{};

		out.e[0][0] = val;
		out.e[1][1] = val;
		out.e[2][2] = val;
		out.e[3][3] = val;

		return out;
	}

	[[nodiscard]]
	inline Mat4 create_translate(Vec3 translation)
	{
		Mat4 out = create_diagonal_matrix();

		out.e[3][0] = translation.x;
		out.e[3][1] = translation.y;
		out.e[3][2] = translation.z;

		return out;
	}

	inline Mat4 create_rotation(Vec3 axis, f32 angle)
	{
		Mat4 out = create_diagonal_matrix();

		axis = normalize(axis);
		f32 sin_theta = sin(angle);
		f32 cos_theta = cos(angle);
		f32 cos_val = 1.0f - cos_theta;

		out.e[0][0] = (axis.x * axis.x * cos_val) + cos_theta;
		out.e[0][1] = (axis.x * axis.y * cos_val) + (axis.z * sin_theta);
		out.e[0][2] = (axis.x * axis.z * cos_val) - (axis.y * sin_theta);

		out.e[1][0] = (axis.y * axis.x * cos_val) - (axis.z * sin_theta);
		out.e[1][1] = (axis.y * axis.y * cos_val) + cos_theta;
		out.e[1][2] = (axis.y * axis.z * cos_val) + (axis.x * sin_theta);

		out.e[2][0] = (axis.z * axis.x * cos_val) + (axis.y * sin_theta);
		out.e[2][1] = (axis.z * axis.y * cos_val) - (axis.x * sin_theta);
		out.e[2][2] = (axis.z * axis.z * cos_val) + cos_theta;

		return out;
	}

	inline Mat4 create_rotation_x(f32 angle)
	{
		Mat4 out = create_diagonal_matrix();

		f32 sin_theta = sin(angle);
		f32 cos_theta = cos(angle);

		out.e[0][0] = 1.0f;
		out.e[0][1] = 0.0f;
		out.e[0][2] = 0.0f;

		out.e[1][0] = 0.0f;
		out.e[1][1] = cos_theta;
		out.e[1][2] = sin_theta;

		out.e[2][0] = 0.0f;
		out.e[2][1] = -sin_theta;
		out.e[2][2] = cos_theta;

		return out;
	}

	inline Mat4 create_rotation_y(f32 angle)
	{
		Mat4 out = create_diagonal_matrix();

		f32 sin_theta = sin(angle);
		f32 cos_theta = cos(angle);

		out.e[0][0] = cos_theta;
		out.e[0][1] = 0.0f;
		out.e[0][2] = -sin_theta;

		out.e[1][0] = 0.0f;
		out.e[1][1] = 1.0f;
		out.e[1][2] = 0.0f;

		out.e[2][0] = sin_theta;
		out.e[2][1] = 0.0f;
		out.e[2][2] = cos_theta;

		return out;
	}

	inline Mat4 create_rotation_z(f32 angle)
	{
		Mat4 out = create_diagonal_matrix();

		f32 sin_theta = sin(angle);
		f32 cos_theta = cos(angle);

		out.e[0][0] = cos_theta;
		out.e[0][1] = sin_theta;
		out.e[0][2] = 0.0f;

		out.e[1][0] = -sin_theta;
		out.e[1][1] = cos_theta;
		out.e[1][2] = 0.0f;

		out.e[2][0] = 0.0f;
		out.e[2][1] = 0.0f;
		out.e[2][2] = 1.0f;

		return out;
	}

	inline Mat4 create_look_at(Vec3 eye, Vec3 target, Vec3 up)
	{
		Mat4 out{};

		Vec3 forward = normalize(eye - target);
		Vec3 right = normalize(cross(up, forward));
		Vec3 upward = cross(forward, right);

		out.e[0][0] = right.x;
		out.e[0][1] = upward.x;
		out.e[0][2] = forward.x;
		out.e[0][3] = 0.0f;

		out.e[1][0] = right.y;
		out.e[1][1] = upward.y;
		out.e[1][2] = forward.y;
		out.e[1][3] = 0.0f;

		out.e[2][0] = right.z;
		out.e[2][1] = upward.z;
		out.e[2][2] = forward.z;
		out.e[2][3] = 0.0f;

		out.e[3][0] = -dot(right, eye);
		out.e[3][1] = -dot(upward, eye);
		out.e[3][2] = -dot(forward, eye);
		out.e[3][3] = 1.0f;

		return out;
	}

	inline Mat4 create_fpp_view(Vec3 eye, f32 pitch, f32 yaw)
	{
		Mat4 out{};

		f32 cos_pitch = cos(pitch);
		f32 sin_pitch = sin(pitch);
		f32 cos_yaw = cos(yaw);
		f32 sin_yaw = sin(yaw);

		Vec3 forward{ cos_yaw, 0.0f, -sin_yaw };
		Vec3 right{ sin_yaw * sin_pitch, cos_pitch, cos_yaw * sin_pitch };
		Vec3 upward{ sin_yaw * cos_pitch, -sin_pitch, cos_pitch * cos_yaw };

		out.e[0][0] = right.x;
		out.e[0][1] = upward.x;
		out.e[0][2] = forward.x;
		out.e[0][3] = 0.0f;

		out.e[1][0] = right.y;
		out.e[1][1] = upward.y;
		out.e[1][2] = forward.y;
		out.e[1][3] = 0.0f;

		out.e[2][0] = right.z;
		out.e[2][1] = upward.z;
		out.e[2][2] = forward.z;
		out.e[2][3] = 0.0f;

		out.e[3][0] = -dot(right, eye);
		out.e[3][1] = -dot(upward, eye);
		out.e[3][2] = -dot(forward, eye);
		out.e[3][3] = 1.0f;

		return out;
	}

	inline Mat4 create_perspective(f32 fov, f32 aspect, f32 near, f32 far)
	{
		Mat4 out{};

		f32 y_scale = 1.0f / tan(fov / 2.0f);
		f32 x_scale = y_scale / aspect;

		out.e[0][0] = x_scale;
		out.e[1][1] = y_scale;
		out.e[2][3] = -1.0f;

		out.e[2][2] = (far) / (near - far);
		out.e[3][2] = (near * far) / (near - far);

		return out;
	}

	inline f32 det(const Mat4 a)
	{
		f32 out = 0.0f;

		Vec3 c01 = cross(a.vecs[0].xyz, a.vecs[1].xyz);
		Vec3 c23 = cross(a.vecs[2].xyz, a.vecs[3].xyz);
		Vec3 u = a.vecs[0].xyz * a.vecs[1].w - a.vecs[1].xyz * a.vecs[0].w;
		Vec3 v = a.vecs[2].xyz * a.vecs[3].w - a.vecs[3].xyz * a.vecs[2].w;

		out = dot(c01, v) + dot(c23, u);
		return out;
	}

	// Math formula from Eric Lengyel book "Foundations of Game Engine Development"
	inline Mat4 inverse(const Mat4 a)
	{
		Mat4 out{};

		Vec3 c01 = cross(a.vecs[0].xyz, a.vecs[1].xyz);
		Vec3 c23 = cross(a.vecs[2].xyz, a.vecs[3].xyz);
		Vec3 u = a.vecs[0].xyz * a.vecs[1].w - a.vecs[1].xyz * a.vecs[0].w;
		Vec3 v = a.vecs[2].xyz * a.vecs[3].w - a.vecs[3].xyz * a.vecs[2].w;

		float inv_det = 1.0f / (dot(c01, v) + dot(c23, u));
		c01 = c01 * inv_det;
		c23 = c23 * inv_det;
		u = u * inv_det;
		v = v * inv_det;

		out.vecs[0] = Vec4{ cross(a.vecs[1].xyz, v) + c23 * a.vecs[1].w, -dot(a.vecs[1].xyz, c23) };
		out.vecs[1] = Vec4{ cross(v, a.vecs[0].xyz) - c23 * a.vecs[0].w,  dot(a.vecs[0].xyz, c23) };
		out.vecs[2] = Vec4{ cross(a.vecs[3].xyz, u) + c01 * a.vecs[3].w, -dot(a.vecs[3].xyz, c01) };
		out.vecs[3] = Vec4{ cross(u, a.vecs[2].xyz) - c01 * a.vecs[2].w,  dot(a.vecs[2].xyz, c01) };

		return transpose(out);
	}

	//? Linear combination with last add+mul skipped
	inline __m128 trans_linear_combination(const Mat4 a, __m128 b)
	{
		__m128 out{};

		out = _mm_mul_ps(a.columns[0], _mm_shuffle_ps(b, b, _MM_SHUFFLE(0, 0, 0, 0)));
		out = _mm_add_ps(out, _mm_mul_ps(a.columns[1], _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 1, 1, 1))));
		out = _mm_add_ps(out, _mm_mul_ps(a.columns[2], _mm_shuffle_ps(b, b, _MM_SHUFFLE(2, 2, 2, 2))));

		return out;
	}

	// used for multiplication with normal vector: https://github.com/graphitemaster/normals_revisited
	// so translation column doesnt matter this is essentialy 3x3 adjugate
	inline Mat4 adjugate_trans(const Mat4 a)
	{
		Mat4 out{};

		out.vecs[0] = Vec4{ cross(a.vecs[1].xyz, a.vecs[2].xyz), 0.0f };
		out.vecs[1] = Vec4{ cross(a.vecs[2].xyz, a.vecs[0].xyz), 0.0f };
		out.vecs[2] = Vec4{ cross(a.vecs[0].xyz, a.vecs[1].xyz), 0.0f };
		out.vecs[3][3] = 1.0f;

		return transpose(out);
	}

	//? Multiplication when Vec3 is 3D vector (w=0), we can skip last mul+add completely
	inline Vec3 mul_trans_vec(const Mat4 a, const Vec3 p)
	{
		Vec4 out{};

		__m128 converted = _mm_set_ps(0.0f, p.z, p.y, p.x);
		out.simd = trans_linear_combination(a, converted);

		return { out.x, out.y, out.z };
	}

	//? Multiplication when Vec3 is a homogenous point (w=1), we can skip last multiplication
	inline Vec3 mul_trans_point(const Mat4 a, const Vec3 p)
	{
		Vec4 out{};

		__m128 con = _mm_set_ps(1.0f, p.z, p.y, p.x);
		out.simd = _mm_mul_ps(a.columns[0], _mm_shuffle_ps(con, con, _MM_SHUFFLE(0, 0, 0, 0)));
		out.simd = _mm_add_ps(out.simd, _mm_mul_ps(a.columns[1], _mm_shuffle_ps(con, con, _MM_SHUFFLE(1, 1, 1, 1))));
		out.simd = _mm_add_ps(out.simd, _mm_mul_ps(a.columns[2], _mm_shuffle_ps(con, con, _MM_SHUFFLE(2, 2, 2, 2))));
		out.simd = _mm_add_ps(out.simd, a.columns[3]);

		return { out.x, out.y, out.z };
	}

	//? "cutted" linear combination for first 3 columns cause bottom element is 0
	inline Mat4 mul_trans(const Mat4 a, const Mat4 b)
	{
		Mat4 out{};

		out.columns[0] = trans_linear_combination(a, b.columns[0]);
		out.columns[1] = trans_linear_combination(a, b.columns[1]);
		out.columns[2] = trans_linear_combination(a, b.columns[2]);
		out.columns[3] = linear_combination(a, b.columns[3]);

		return out;
	}

	inline Vec3 get_scale(const Mat4 a)
	{
		return { a.e[0][0], a.e[1][1], a.e[2][2] };
	}

	inline Vec3 get_translation(const Mat4 a)
	{
		return { a.e[3][0],a.e[3][1], a.e[3][2] };
	}

	//? Based on https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
	//? Basically divide each column axis by its length squared then transpose, in implemnetation transpose is done first to have
	//? data already prepared for SIMD dots for length calculation. 
	//! Removed divide by 0 check of original implementation, YOLO
	//! This may not work for transforms with shear, skew TEST!
	inline Mat4 inverse_trans(const Mat4 a)
	{
		Mat4 out{};

		// transpose 3x3
		__m128 t0 = _mm_movelh_ps(a.columns[0], a.columns[1]);
		__m128 t1 = _mm_movehl_ps(a.columns[0], a.columns[1]); // this should be [1] then [0]?
		out.columns[0] = _mm_shuffle_ps(t0, a.columns[2], _MM_SHUFFLE(3, 0, 2, 0));
		out.columns[1] = _mm_shuffle_ps(t0, a.columns[2], _MM_SHUFFLE(3, 1, 3, 1));
		out.columns[2] = _mm_shuffle_ps(t1, a.columns[2], _MM_SHUFFLE(3, 2, 2, 0));

		// calculate dot products for squared length of each axis - 3 dots at once
		__m128 lengths_squared{};
		lengths_squared = _mm_mul_ps(out.columns[0], out.columns[0]);
		lengths_squared = _mm_add_ps(lengths_squared, _mm_mul_ps(out.columns[1], out.columns[1]));
		lengths_squared = _mm_add_ps(lengths_squared, _mm_mul_ps(out.columns[2], out.columns[2]));

		__m128 r_lengths_squared = _mm_div_ps(_mm_set_ps1(1.f), lengths_squared);

		out.columns[0] = _mm_mul_ps(out.columns[0], r_lengths_squared);
		out.columns[1] = _mm_mul_ps(out.columns[1], r_lengths_squared);
		out.columns[2] = _mm_mul_ps(out.columns[2], r_lengths_squared);

		// 3x3 inverse multiplied by -transpose, linear combination and negate with setting w=1.0f
		out.columns[3] = _mm_mul_ps(out.columns[0], _mm_shuffle_ps(a.columns[3], a.columns[3], _MM_SHUFFLE(0, 0, 0, 0)));
		out.columns[3] = _mm_add_ps(out.columns[3], _mm_mul_ps(out.columns[1], _mm_shuffle_ps(a.columns[3], a.columns[3], _MM_SHUFFLE(1, 1, 1, 1))));
		out.columns[3] = _mm_add_ps(out.columns[3], _mm_mul_ps(out.columns[2], _mm_shuffle_ps(a.columns[3], a.columns[3], _MM_SHUFFLE(2, 2, 2, 2))));
		//out.columns[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), out.columns[3]); //maybe use xor instead?
		out.columns[3] = _mm_xor_ps(out.columns[3], _mm_set_ps(1.f, -0.f, -0.f, -0.f));

		return out;
	}
}
