//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Thomas Oberbichler
//
//  Ported from the ANurbs library (https://github.com/oberbichler/ANurbs)
//

#if !defined(KRATOS_NURBS_TEST_UTILITY_H_INCLUDED )
#define  KRATOS_NURBS_TEST_UTILITY_H_INCLUDED

// System includes

// External includes

// Project includes
#include "containers/array_1d.h"
#include "testing/testing.h"

namespace Kratos {
namespace Testing {

class NurbsTestUtility
{
public:     // static methods
    static array_1d<double, 3> Point(double x, double y, double z)
    {
        array_1d<double, 3> point;

        point[0] = x;
        point[1] = y;
        point[2] = z;

        return point;
    }

    static void ArrayAlmostEqual(const array_1d<double, 3>& actual,
        const array_1d<double, 3>& expected, const double tolerance = 1e-5)
    {
        for (size_t i = 0; i < 3; i++)
        {
            KRATOS_CHECK_NEAR(actual[i], expected[i], tolerance);
        }
    }
}; // class NurbsTestUtility

} // namespace Testing
} // namespace Kratos

#endif // KRATOS_NURBS_TEST_UTILITY_H_INCLUDED defined