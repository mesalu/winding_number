// Samuel Dunn

#include <winding.hpp>

#include <cmath>
#include <utility>

// Future Improvements:
//      - Iterators to improve function of traversing points and edges in a polygon.
//          - would abstract different ways of filtering points out, and possibly expose as strategies to clients
//            for better control
//      - I think, since modifications to winding_number are only ever 1 in magnitude, that we could reduce branching
//        (and therefore potential mispredictions) by converting boolean results to integers. I opted against
//        doing this for this submission as I valued readability and maintainability over performance for this context

// Known problems / missing pieces:
//      - I wasn't certain how to treat polygons that are just an oscilating line that passes over
//          the test point. Current implementation should just count every other edge.
//      - I tripped up on when the test point falls on a line & how to determine
//        direction of travel, After a fair bit of this, I re-read the prompt to see:
//        "so the winding number would be the number of times the polygon goes counter-clockwise through the point."
//        so if p falls on an edge, its treated as though it were to the left of the line.

namespace winding_number {
namespace {

// For convenience, not strictly necessary.
struct Point { float x, y; };

// Some helper methods that do not need to be bound to a class instance:

// Calculates the z-component of the cross product of the vectors created between: [a, b], [b, c]
// where the z-component of those vectors is 0. The result is a scalar value that
// indicates the directional relationship of c w.r.t. the line [a, b].
// - less than 0: the line is moving clockwise about c.
// - 0: c is somewhere along the line.
// - greater than 0: the line is moving counter clockwise about
float CrossProduct(const Point& a, const Point& b, const Point& c);

// A convenience method that extracts the n'th x and y values from the given
// polygon and returns them in point-form.
Point ExtractPoint(const poly::Polygon& polygon, int n);

// Specifies if the given poitns are within tolerance range in both cardinal direction
bool WithinTolerance(float tolerance, const Point& a, const Point& b);

// Typical fuzzy check, checks for equality out to the n'th decimal.
bool FuzzyEquals(float a, float b, float max_delta = 1e-6f);

// A straight forward implementation of IWindingNumberAlgorithm. It uses
// some assumptions about the input polygon and basic two dimensional linear
// algebra to compute the winding count in O(n) time. (with out vertex filtering.)
class SimpleWindingNumberAlgorithm : public IWindingNumberAlgorithm {
    std::optional<int> CalculateWindingNumber2D(float x, float y, poly::Polygon polygon) override {
        // Polygon is required to be closed.
        if (!polygon.IsClosed(tolerance())) {
            error_message("Input polygon is not closed.");
            return std::nullopt;
        }

        int winding_number = 0;
        Point p = {x, y};

        // Key Observations:
        // Since the polygon is required to be closed we can:
        // - Use any angle w.r.t. p to watch for edge traversal,
        //   cardinal directions from test point are the easiest.
        // - when crossing that angle, based on the direction of travel
        //   and the side of the edge the test point is on, drastically reducing
        //   cases to look for.

        Point a = ExtractPoint(polygon, 0);
        bool a_left_or_on_p = a.x <= p.x;   // used to avoid re-doing work.
        size_t poly_size = polygon.size();
        size_t evaluated_edge_count = 0;
        for (int i = 1; i < poly_size; i++) {
            // Don't depend fuzzy comparisons when closing the curve, we know its supposed
            // to be closed if we're here, behave accordingly.
            Point b = ExtractPoint(polygon, (i == poly_size - 1) ? 0 : i);

            // Skip if within tolerance range:
            if (WithinTolerance(tolerance(), a, b)) continue;

            bool b_left_or_on_p = b.x <= p.x;

            // Most edges should arrive here: check for passing p on x axis
            // and act accordingly.
            auto cross_product = CrossProduct(a, b, p);
            if (FuzzyEquals(cross_product, 0) && FuzzyEquals(a.x, b.x) && a.y < b.y
                && p.y <= b.y && a.y <= p.y) {
                    // the test point is on a vertically traversing edge
                winding_number++;
            }
            else if (a_left_or_on_p) {
                // left to right motion, moving clockwise if to right.
                if (!b_left_or_on_p && cross_product < 0) winding_number--;
            }
            else
                // right to left motion, moving ccw if to left or on the line.
                if (b_left_or_on_p && cross_product >= 0) winding_number++;

            // update trackers.
            a = b;
            a_left_or_on_p = b_left_or_on_p;
            evaluated_edge_count++;
        }

        if (evaluated_edge_count < 1) {
            error_message("Insufficient geometry in polygon for a meaningful result");
            return std::nullopt;
        }

        return winding_number;
    }
};

float CrossProduct(const Point& a, const Point& b, const Point& c) {
    return ((b.x - a.x) * (c.y - b.y)) - ((b.y - a.y) * (c.x - b.x));
}

Point ExtractPoint(const poly::Polygon& polygon, int n) {
    return {polygon.x_vec_.at(n), polygon.y_vec_.at(n)};
}

bool WithinTolerance(float tolerance, const Point& a, const Point& b) {
    return (std::abs(a.x - b.x) <= tolerance && std::abs(a.y - b.y) <= tolerance);
}

bool FuzzyEquals(float a, float b, float max_delta) {
    return (std::abs(a -b) <= max_delta);
}

}  // namespace

std::unique_ptr<IWindingNumberAlgorithm> IWindingNumberAlgorithm::Create() {
    return std::make_unique<SimpleWindingNumberAlgorithm>();
}

void IWindingNumberAlgorithm::tolerance(float tolerance) noexcept {
    tolerance_ = tolerance;
}

float IWindingNumberAlgorithm::tolerance() const noexcept {
    return tolerance_;
}

std::string IWindingNumberAlgorithm::error_message() const noexcept {
    return error_message_;
}

void IWindingNumberAlgorithm::error_message(std::string error_message) noexcept {
    error_message_ = std::move(error_message);
}

}  // namespace winding_number
