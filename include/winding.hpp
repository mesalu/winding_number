#ifndef WINDING_HPP_
#define WINDING_HPP_

#include <memory>
#include <optional>  // A C++17 capable compiler is assumed here.
#include <string>
#include <vector>

#include <poly_io.hpp>

namespace winding_number {

// Interface for the Winding Number algorithm.
//
// The winding number is the number of times a polygon winds counter-clockwise around a point. If the polygon winds
// clockwise around the point then the sign is reversed. So one loop clockwise around a point would result in a winding
// number of -1, whereas one loop counter-clockwise around a point would be a winding number of 1.
//
// If the point is outside the polygon then it has a winding number of 0.
//
// If the point lies on the edge of the polygon, then it is considered inside the polygon -- so the winding number would
// be the number of times the polygon goes counter-clockwise through the point.
class IWindingNumberAlgorithm {
public:
    virtual ~IWindingNumberAlgorithm() = default;

    // Returns the implementation of the IWindingNumberAlgorithm that will be used.
    [[nodiscard]] static std::unique_ptr<IWindingNumberAlgorithm> Create();

    // Returns the winding number of a 2D point with respect to a 2D polygon, when it is possible to do so, otherwise
    // returns std::nullopt.
    virtual std::optional<int> CalculateWindingNumber2D(float x, float y, poly::Polygon polygon) = 0;

    // Getters and setters for an initial set of parameters and results.
    float tolerance() const noexcept;
    void tolerance(float tolerance) noexcept;

    std::string error_message() const noexcept;

protected:
    void error_message(std::string error_message) noexcept;

private:
    // Tolerance is a distance measure -- when the two points are as close, or closer than, tolerance_ apart in all
    // dimensions, then they are considered the same point.
    float tolerance_ = 0.f;

    // An error message describing what, if anything, went wrong with the most recent call to CalculateWindingNumber().
    std::string error_message_;
};

}  // namespace winding_number

#endif
