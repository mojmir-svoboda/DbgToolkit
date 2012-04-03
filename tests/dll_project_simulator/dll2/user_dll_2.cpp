// compile with: /EHsc /LD

#include "user_dll_2.h"
#include "../../../trace_client/trace.h"
#include <stdexcept>

using namespace std;

namespace bar
{
    double MathFuncs_2::Add(double a, double b)
    {
		TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Adding: %f +  %f = ", a, b, a + b);
        return a + b;
    }

    double MathFuncs_2::Subtract(double a, double b)
    {
		TRACE_SCOPE(trace::e_Info, trace::CTX_Default);
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Adding: %f + -%f = ", a, b, a - b);
        return a - b;
    }

    double MathFuncs_2::Multiply(double a, double b)
    {
        return a * b;
    }

    double MathFuncs_2::Divide(double a, double b)
    {
        if (b == 0)
        {
            throw new invalid_argument("b cannot be zero!");
        }

        return a / b;
    }
}
