#pragma once
#include "../macros.h"

namespace foo
{
    struct MathFuncs_1
    {
        // Returns a + b
        static MY_API double Add (double a, double b);

        // Returns a - b
        static MY_API double Subtract (double a, double b);

        // Returns a * b
        static MY_API double Multiply (double a, double b);

        // Returns a / b
        // Throws DivideByZeroException if b is 0
        static MY_API double Divide (double a, double b);
    };
}
