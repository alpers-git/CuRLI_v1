#pragma once
#include "glm/glm.hpp"
#include "cyVector.h"

namespace glm
{
	//glm::vec converters from cy::Vec classes
	template <typename T>
	inline vec2 cy2GLM(const cy::Vec2<T>& v){ return vec2(v.x, v.y); };
	template <typename T>
	inline vec3 cy2GLM(const cy::Vec3<T>& v) { return vec3(v.x, v.y, v.z); };
	template <typename T>
	inline vec4 cy2GLM(const cy::Vec4<T>& v) { return vec4(v.x, v.y, v.z, v.w); };
	
	//cy::Vec converters from glm::vec classes
	template <typename T>
	inline cy::Vec2<T> GLM2cy(const vec2& v) { return cy::Vec2<T>(v.x, v.y); };
	template <typename T>
	inline cy::Vec3<T> GLM2cy(const vec3& v) { return cy::Vec3<T>(v.x, v.y, v.z); };
	template <typename T>
	inline cy::Vec4<T> GLM2cy(const vec4& v) { return cy::Vec4<T>(v.x, v.y, v.z, v.w); };
}