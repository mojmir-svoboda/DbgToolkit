#pragma once

enum E_CmpMode {
	  e_CmpL = 0
	, e_CmpLE
	, e_CmpEQ
	, e_CmpGE
	, e_CmpG
	, e_CmpNEQ
	, e_max_cmpmod_enum_value
};
static QString comparatorsStr[e_max_cmpmod_enum_value] = { "<", "<=", "==", ">=", ">", "!=" };
inline QString cmpModToString (E_CmpMode l) { return comparatorsStr[l]; }
inline E_CmpMode stringToCmpMod (QString const & qstr) {
	for (size_t i = 0; i < e_max_cmpmod_enum_value; ++i)
		if (qstr == comparatorsStr[i])
			return static_cast<E_CmpMode>(i);
	return e_CmpL;
}

template<typename PodT>
bool lesser (PodT lhs, PodT rhs) { return lhs < rhs; }
template<typename PodT>
bool lesser_or_eq (PodT lhs, PodT rhs) { return lhs <= rhs; }
template<typename PodT>
bool eq (PodT lhs, PodT rhs) { return lhs == rhs; }
template<typename PodT>
bool greater_or_eq (PodT lhs, PodT rhs) { return lhs >= rhs; }
template<typename PodT>
bool greater (PodT lhs, PodT rhs) { return lhs > rhs; }
template<typename PodT>
bool not_eq (PodT lhs, PodT rhs) { return lhs != rhs; }
typedef bool(*flt_cmp_t)(float, float);
flt_cmp_t const flt_comparators[e_max_cmpmod_enum_value] = { lesser<float>, lesser_or_eq<float>, eq<float>, greater_or_eq<float>, greater<float>, not_eq<float> };

typedef bool(*int_cmp_t)(int, int);
int_cmp_t const int_comparators[e_max_cmpmod_enum_value] = { lesser<int>, lesser_or_eq<int>, eq<int>, greater_or_eq<int>, greater<int>, not_eq<int> };

