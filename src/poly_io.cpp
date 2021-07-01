#include <poly_io.hpp>

#include <cassert>
#include <cmath>
#include <filesystem>  // A C++17 capable compiler is assumed here.
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace poly {
namespace {

    // Split a string in to separate strings, separated by any of the passed in delimeters (each is a character).
    std::vector<std::string_view> SplitString(std::string_view to_split, std::string_view delims) {
        std::vector<std::string_view> split_strings;

        for (size_t first = 0, second = 0; first < to_split.size() && second != std::string_view::npos;
             first = second + 1) {
            second = to_split.find_first_of(delims, first);
            if (first != second) {
                split_strings.emplace_back(to_split.substr(first, second - first));
            }
        }
        return split_strings;
    }

    class DefaultPolygonReader : public IPolygonReader {
    public:
        std::tuple<float, float, Polygon> CreatePointAndPolygonFromString(std::string_view polygon_string) override;
        std::vector<std::tuple<float, float, Polygon>> ReadPointsAndPolygonsFromFile(
                std::string_view filepath) override;
    };

    std::tuple<float, float, Polygon> DefaultPolygonReader::CreatePointAndPolygonFromString(
            std::string_view polygon_string) {

        // Permit trailing comments, setting # as the formal comment character.
        std::string_view line_body = polygon_string.substr(0, polygon_string.find('#'));

        // Use horizontal tabs for white space delimiting as well.
        auto split_string = SplitString(line_body, " \t");

        std::optional<float> x, point_x, point_y;
        std::string element_string;
        Polygon polygon;
        for (std::string_view element_view : split_string) {
            element_string = element_view;
            float element_value;
            try {
                element_value = std::stof(element_string);
            } catch (const std::invalid_argument&) {
                throw std::runtime_error("Could not parse line because this is not a floating point value: " +
                                         element_string);
            } catch (const std::out_of_range&) {
                throw std::runtime_error("Could not parse line because this is too large to fit in a float: " +
                                         element_string);
            }
            if (!point_x) {
                point_x = element_value;
            } else if (!point_y) {
                point_y = element_value;
            } else if (!x) {
                x = element_value;
            } else {
                polygon.AppendPoint(*x, element_value);
                x.reset();
            }
        }
        if (!point_x) {
            throw std::runtime_error("Missing initial x-value for point.");
        } else if (!point_y) {
            throw std::runtime_error("Missing initial y-value for point.");
        }

        // Post-submission addition.
        if (polygon.size() <= 1) {
            throw std::runtime_error("Insufficient geometry to compose polygon.");
        }
        return {*point_x, *point_y, polygon};
    }

    std::vector<std::tuple<float, float, Polygon>> DefaultPolygonReader::ReadPointsAndPolygonsFromFile(
            std::string_view filepath) {
        std::filesystem::path path(filepath);
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
            throw std::runtime_error("Provided filepath is not readable as a file: " + std::string(filepath));
        }
        std::vector<std::tuple<float, float, Polygon>> point_and_polygons;
        std::string line;
        std::ifstream fs;
        int i = 0;
        try {
            fs = std::ifstream(std::string(filepath), std::ios::in);
            fs.exceptions(fs.failbit | fs.badbit);

            // SD: Interestingly, merely peeking for EOF is insufficient - if the stream has already reached EOF
            //     then peeking for it may trip the failbit and throw an exception (due to the above exception
            //     mask) This can be fixed by shortcircuiting if eofbit is already set. Another thing to do - which
            //     may be better overall is to use fs.good() to ensure the stream is still in a usable state overall.
            while (!fs.eof() && fs.peek() != std::char_traits<char>::eof()) {
                std::getline(fs, line);
                try {
                    point_and_polygons.emplace_back(CreatePointAndPolygonFromString(line));
                } catch (const std::runtime_error&) {
                    // failed to parse line - toss & move on.
                    continue;
                }
            }
        } catch (const std::ios_base::failure& e) {
            fs.close();
            throw std::runtime_error("Failed to read:\t" + std::string(filepath) + "\n" +  //
                                     "Error:\t\t" + e.what());
        } catch (const std::runtime_error& e) {
            fs.close();
            throw std::runtime_error("Failed to read a line in:\t" + std::string(filepath) + "\n" +  //
                                     "Error:\t\t" + e.what());
        }
        fs.close();
        return point_and_polygons;
    }

}  // namespace

Polygon::Polygon(size_t capacity) {
    x_vec_.reserve(capacity);
    y_vec_.reserve(capacity);
}

void Polygon::AppendPoint(float x, float y) {
    x_vec_.push_back(x);
    y_vec_.push_back(y);
}

size_t Polygon::size() const {
    size_t x_vec_size = x_vec_.size();
    assert(x_vec_size == y_vec_.size());
    return x_vec_size;
}

void Polygon::ClosePolygon() {
    if (size() == 0 || IsClosed()) {
        return;
    }
    x_vec_.push_back(x_vec_[0]);
    y_vec_.push_back(y_vec_[0]);
}

bool Polygon::IsClosed(float tolerance) const {
    return (std::abs(x_vec_.front() - x_vec_.back()) <= tolerance &&  //
            std::abs(y_vec_.front() - y_vec_.back()) <= tolerance);
}

std::unique_ptr<IPolygonReader> IPolygonReader::Create() {
    return std::make_unique<DefaultPolygonReader>();
}

}  // namespace poly
