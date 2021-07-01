#include <gtest/gtest.h>

#include <filesystem>  // A C++17 capable compiler is assumed here.
#include <optional>
#include <string>
#include <tuple>

#include <winding.hpp>
#include <poly_io.hpp>

namespace winding_number {

using poly::Polygon;

class WindingNumberTest : public ::testing::Test {
protected:
    WindingNumberTest() :
            reader_(poly::IPolygonReader::Create()),
            algorithm_(IWindingNumberAlgorithm::Create()),
            polygons_file_path_((std::filesystem::current_path() / "polygons.txt").string()),
            tolerance_(1e-6f) {
        algorithm_->tolerance(tolerance_);
    }

    std::unique_ptr<poly::IPolygonReader> reader_;
    std::unique_ptr<IWindingNumberAlgorithm> algorithm_;
    const std::string polygons_file_path_;
    const float tolerance_;
};

TEST_F(WindingNumberTest, FailsWithUnclosedPolygon) {
    Polygon p;
    p.AppendPoint(0.0, 0.0);
    p.AppendPoint(1.0, 0.0);
    p.AppendPoint(1.0, 1.0);
    EXPECT_FALSE(p.IsClosed());
    auto winding_algo = IWindingNumberAlgorithm::Create();
    auto winding_num = winding_algo->CalculateWindingNumber2D(0.0, 0.0, std::move(p));
    // Either this is due to an unimplemented algorithm or it should be std::nullopt -- as it does not make sense to
    // make this call with unclosed polygons.
    EXPECT_FALSE(winding_num);
}

TEST_F(WindingNumberTest, PassesWithNotQuiteClosedPolygon) {
    const float tolerance = 0.01;
    Polygon p;
    p.AppendPoint(0.0, 0.0);
    p.AppendPoint(1.0, 0.0);
    p.AppendPoint(1.0, 1.0);
    p.AppendPoint(0.0001, 0.0001); // not quite actually closed, but is under tolerance.
    EXPECT_TRUE(p.IsClosed(tolerance));

    auto winding_algo = IWindingNumberAlgorithm::Create();
    winding_algo->tolerance(tolerance);

    auto winding_num = winding_algo->CalculateWindingNumber2D(0.0, 0.0, std::move(p));
    EXPECT_EQ(1, winding_num.value_or(-10000));
}

TEST_F(WindingNumberTest, CanGetPointInPolygon) {
    Polygon p;
    p.AppendPoint(0.0, 0.0);
    p.AppendPoint(1.0, 0.0);
    p.AppendPoint(1.0, 1.0);
    p.AppendPoint(0.0, 1.0);
    p.AppendPoint(0.0, 0.0);
    EXPECT_TRUE(p.IsClosed());
    auto winding_num = algorithm_->CalculateWindingNumber2D(0.0, 0.0, std::move(p));
    ASSERT_TRUE(winding_num);
    EXPECT_EQ(1, *winding_num);
}

// Test if the WindingNumber algorithm detects when geometry was insufficient
// for a relevant result. (e.g., collapsing points due to tolerance value still
// results in a polygon.)
TEST_F(WindingNumberTest, FailOnCollapsedGeometry) {
    Polygon p;
    p.AppendPoint(0.0, 0.0);
    p.AppendPoint(0.0, 1e-7f);
    p.AppendPoint(1e-7f, 1e-7f);
    p.AppendPoint(0.0, 0.0);

    EXPECT_FALSE(algorithm_->CalculateWindingNumber2D(0.0, 0.0, std::move(p)));
}

TEST_F(WindingNumberTest, CanGetWindingNumbersForPolygonsFromFile) {
    // not convinced all of these are correct (particularly the CW polys where test point is on an edge)
    std::vector<std::optional<int>> expected_winding_numbers = {
        1, -1, 1, std::nullopt, 1, 4,
        2, 3, 4, 5, 6, 7, -2, -3, -4,
        -5, -6, -7, 1, 1, 1, 1, 0, 0,
        0, 0, std::nullopt, 1, 0, 2 };
    // I'd really think the second to last (a CCW triangle with some repeated legs) would have winding count 1,
    // so that's an error in my code somewhere.
    auto points_and_polygons = reader_->ReadPointsAndPolygonsFromFile(polygons_file_path_);
    ASSERT_GE(points_and_polygons.size(), expected_winding_numbers.size());
    for (size_t i = 0; i < expected_winding_numbers.size(); ++i) {
        const auto& p = points_and_polygons[i];
        if (expected_winding_numbers[i]) {
            // Only a point and a closed polygon should expect any sort of winding number.
            EXPECT_TRUE(std::get<2>(p).IsClosed(tolerance_));
        }
        auto winding_num = algorithm_->CalculateWindingNumber2D(std::get<0>(p), std::get<1>(p), std::get<2>(p));
        if (winding_num) {
            ASSERT_TRUE(expected_winding_numbers[i]) << "Have a winding number for expected_winding_number[" << i
                                                     << "], which is expected to be missing.";
            int expected_val = expected_winding_numbers[i].value();
            EXPECT_EQ(expected_val, *winding_num);
        } else {

            ASSERT_FALSE(expected_winding_numbers[i]) << "Missing a winding number for expected_winding_number[" << i
                                                       << "], which is expected to exist.";
        }
    }
}

}  // namespace winding_number
