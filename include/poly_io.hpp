#ifndef POLY_IO_HPP_
#define POLY_IO_HPP_

#include <memory>
#include <string_view>  // A C++17 capable compiler is assumed here.
#include <tuple>
#include <vector>

namespace poly {

// Polygon represents a polygon in 2 dimensions, and is specified as an ordered series of points.
struct Polygon {
    Polygon(size_t capacity = 100);

    void AppendPoint(float x, float y);
    size_t size() const;

    // Ensures the last point in the polygon is the same as the first.
    void ClosePolygon();

    // Detects whether the last point in the polygon is the same of the first, up to some tolerance.
    bool IsClosed(float tolerance = 0.f) const;

    // data members
    std::vector<float> x_vec_;
    std::vector<float> y_vec_;
};

class IPolygonReader {
public:
    virtual ~IPolygonReader() = default;

    // Returns the implementation of the IPolygonReader that will be used.
    [[nodiscard]] static std::unique_ptr<IPolygonReader> Create();

    // Creates a point and a Polygon from a string with format:
    //
    // "point_x point_y x0 y0 x1 y1 x2 y2 ... xN yN"
    //
    // Each x-y pair is a 2-D coordinate of a point. The only delimiter is a space. The "... xN yN" just means that
    // there could be an arbitrary number of x-y pairs.
    //
    // This should throw a std::runtime_error if there were any errors parsing the string.
    virtual std::tuple<float, float, Polygon> CreatePointAndPolygonFromString(std::string_view polygon_string) = 0;

    // Creates a vector of point/Polygon pairs given a path to a file with one point and one polygon per line, in the
    // format that CreatePointAndPolygonFromString() accepts. This should throw a std::runtime_error if there were any issues
    // opening or parsing the file.
    virtual std::vector<std::tuple<float, float, Polygon>> ReadPointsAndPolygonsFromFile(std::string_view filepath) = 0;
};

}  // namespace poly

#endif
