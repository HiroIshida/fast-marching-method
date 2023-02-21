// Copyright 2017 Tommy Hinks
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <gtest/gtest.h>

#include "../include/thinks/fast_marching_method/fast_marching_method.hpp"
#include "util.hpp"

namespace {

// Fixtures.

template <typename T>
class UnsignedArrivalTimeTest : public ::testing::Test {
 protected:
  virtual ~UnsignedArrivalTimeTest() {}
};

template <typename T>
class UnsignedArrivalTimeAccuracyTest : public ::testing::Test {
 protected:
  virtual ~UnsignedArrivalTimeAccuracyTest() {}
};

// Associate types with fixtures.

typedef ::testing::Types<
    util::ScalarDimensionPair<float, 2>, util::ScalarDimensionPair<float, 3>,
    util::ScalarDimensionPair<float, 4>, util::ScalarDimensionPair<double, 2>,
    util::ScalarDimensionPair<double, 3>, util::ScalarDimensionPair<double, 4>>
    UnsignedArrivalTimeTypes;

typedef ::testing::Types<
    util::ScalarDimensionPair<float, 2>, util::ScalarDimensionPair<float, 3>,
    util::ScalarDimensionPair<double, 2>, util::ScalarDimensionPair<double, 3>>
    AccuracyTypes;

TYPED_TEST_SUITE(UnsignedArrivalTimeTest, UnsignedArrivalTimeTypes, );
TYPED_TEST_SUITE(UnsignedArrivalTimeAccuracyTest, AccuracyTypes, );

// UnsignedArrivalTime fixture.

TYPED_TEST(UnsignedArrivalTimeTest, ZeroElementInGridSizeThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  for (auto i = size_t{0}; i < kDimension; ++i) {
    auto grid_size = util::FilledArray<kDimension>(size_t{10});
    grid_size[i] = 0;  // Zero element in i'th position.
    auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
    auto const speed = ScalarType{1};

    auto boundary_indices = vector<array<int32_t, kDimension>>();
    boundary_indices.push_back(util::FilledArray<kDimension>(int32_t{0}));
    auto const boundary_distances = vector<ScalarType>(1, ScalarType{1});

    auto expected_reason = stringstream();
    expected_reason << "invalid size: " << util::ToString(grid_size);

    // Act.
    auto const ft = util::FunctionThrows<invalid_argument>([=]() {
      auto const unsigned_times = fmm::UnsignedArrivalTime(
          grid_size, boundary_indices, boundary_distances,
          EikonalSolverType(grid_spacing, speed));
    });

    // Assert.
    ASSERT_TRUE(ft.first);
    ASSERT_EQ(expected_reason.str(), ft.second);
  }
}

TYPED_TEST(UnsignedArrivalTimeTest, EmptyBoundaryThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const speed = ScalarType{1};
  auto const boundary_indices = vector<array<int32_t, kDimension>>{};  // Empty.
  auto const boundary_distances = vector<ScalarType>{};                // Empty.

  // Act.
  auto const ft = util::FunctionThrows<invalid_argument>([=]() {
    auto const unsigned_times = fmm::UnsignedArrivalTime(
        grid_size, boundary_indices, boundary_distances,
        EikonalSolverType(grid_spacing, speed));
  });

  // Assert.
  ASSERT_TRUE(ft.first);
  ASSERT_EQ("empty boundary condition", ft.second);
}

TYPED_TEST(UnsignedArrivalTimeTest, FullGridBoundaryIndicesThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const speed = ScalarType{1};

  // Add every cell in the grid!
  auto boundary_indices = vector<array<int32_t, kDimension>>();
  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  while (index_iter.has_next()) {
    boundary_indices.push_back(index_iter.index());
    index_iter.Next();
  }

  auto const boundary_distances =
      vector<ScalarType>(util::LinearSize(grid_size), ScalarType{1});

  // Act.
  auto const ft = util::FunctionThrows<invalid_argument>([=]() {
    auto const unsigned_times = fmm::UnsignedArrivalTime(
        grid_size, boundary_indices, boundary_distances,
        EikonalSolverType(grid_spacing, speed));
  });

  // Assert.
  ASSERT_TRUE(ft.first);
  ASSERT_EQ("full grid boundary", ft.second);
}

TYPED_TEST(UnsignedArrivalTimeTest, DuplicateBoundaryIndicesThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const speed = ScalarType{1};

  auto boundary_indices = vector<array<int32_t, kDimension>>();
  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  boundary_indices.push_back(index_iter.index());
  boundary_indices.push_back(index_iter.index());  // Same index!

  auto const boundary_times = vector<ScalarType>(2, ScalarType{1});

  auto expected_reason = stringstream();
  expected_reason << "duplicate boundary index: "
                  << util::ToString(index_iter.index());

  // Act.
  auto const ft = util::FunctionThrows<invalid_argument>([=]() {
    auto const unsigned_times =
        fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                                 EikonalSolverType(grid_spacing, speed));
  });

  // Assert.
  ASSERT_TRUE(ft.first);
  ASSERT_EQ(expected_reason.str(), ft.second);
}

TYPED_TEST(UnsignedArrivalTimeTest, BoundaryIndexOutsideGridThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const speed = ScalarType{1};

  auto boundary_indices = vector<array<int32_t, kDimension>>();
  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  boundary_indices.push_back(index_iter.index());
  // Outside!
  boundary_indices.push_back(util::FilledArray<kDimension>(int32_t{-1}));

  auto const boundary_times = vector<ScalarType>(2, ScalarType{1});

  auto expected_reason = stringstream();
  expected_reason << "boundary index outside grid - "
                  << "index: " << util::ToString(boundary_indices.back())
                  << ", "
                  << "grid size: " << util::ToString(grid_size);

  // Act.
  auto const ft = util::FunctionThrows<invalid_argument>([=]() {
    auto const unsigned_times =
        fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                                 EikonalSolverType(grid_spacing, speed));
  });

  // Assert.
  ASSERT_TRUE(ft.first);
  ASSERT_EQ(expected_reason.str(), ft.second);
}

TYPED_TEST(UnsignedArrivalTimeTest, BoundaryIndicesAndTimesSizeMismatchThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const speed = ScalarType{1};

  auto boundary_indices = vector<array<int32_t, kDimension>>();
  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  boundary_indices.push_back(index_iter.index());
  index_iter.Next();
  boundary_indices.push_back(index_iter.index());

  // Two indices, three distances.
  auto const boundary_times = vector<ScalarType>(3, ScalarType{1});

  // Act.
  auto const ft = util::FunctionThrows<invalid_argument>([=]() {
    auto const unsigned_times =
        fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                                 EikonalSolverType(grid_spacing, speed));
  });

  // Assert.
  ASSERT_TRUE(ft.first);
  ASSERT_EQ("boundary indices[2] / boundary times[3] size mismatch", ft.second);
}

TYPED_TEST(UnsignedArrivalTimeTest, InvalidBoundaryTimeThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const invalid_boundary_times =
      array<ScalarType, 4>{{ScalarType{-1}, numeric_limits<ScalarType>::max(),
                            -numeric_limits<ScalarType>::max(),
                            numeric_limits<ScalarType>::quiet_NaN()}};

  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const speed = ScalarType{1};
  for (auto const invalid_boundary_time : invalid_boundary_times) {
    auto boundary_indices = vector<array<int32_t, kDimension>>();
    auto index_iter = util::IndexIterator<kDimension>(grid_size);
    boundary_indices.push_back(index_iter.index());

    auto const boundary_times =
        vector<ScalarType>(1, invalid_boundary_time);  // Invalid!

    auto expected_reason = stringstream();
    expected_reason << "invalid boundary time: " << invalid_boundary_time;

    // Act.
    auto const ft = util::FunctionThrows<invalid_argument>([=]() {
      auto const unsigned_times =
          fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                                   EikonalSolverType(grid_spacing, speed));
    });

    // Assert.
    ASSERT_TRUE(ft.first);
    ASSERT_EQ(expected_reason.str(), ft.second);
  }
}

TYPED_TEST(UnsignedArrivalTimeTest, EikonalSolverFailThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr auto kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const speed = ScalarType{1};

  // Create a scenario where solver has a very small value in one direction
  // and a very large value in another. Cannot resolve gradient for
  // this scenario.
  auto boundary_indices = vector<array<int32_t, kDimension>>();
  boundary_indices.push_back(util::FilledArray<kDimension>(int32_t{0}));
  boundary_indices.push_back(util::FilledArray<kDimension>(int32_t{1}));
  auto boundary_times = vector<ScalarType>();
  boundary_times.push_back(ScalarType{1000});
  boundary_times.push_back(ScalarType{1});

  // Act.
  auto const ft = util::FunctionThrows<runtime_error>([=]() {
    auto const unsigned_times =
        fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                                 EikonalSolverType(grid_spacing, speed));
  });

  // Assert.
  ASSERT_TRUE(ft.first);
  ASSERT_EQ("invalid arrival time (distance)", ft.second.substr(size_t{0}, 31));
}

TYPED_TEST(UnsignedArrivalTimeTest, ContainedComponentThrows) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{20});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType(0.05));
  auto const uniform_speed = ScalarType{1};

  auto const sphere_center1 = util::FilledArray<kDimension>(ScalarType{0.5});
  auto const sphere_radius1 = ScalarType(0.2);
  auto const sphere_center2 = util::FilledArray<kDimension>(ScalarType{0.5});
  auto const sphere_radius2 = ScalarType(0.45);

  auto sphere_boundary_indices1 = vector<array<int32_t, kDimension>>();
  auto sphere_boundary_times1 = vector<ScalarType>();
  util::HyperSphereBoundaryCells(
      sphere_center1, sphere_radius1, grid_size, grid_spacing,
      [](ScalarType const d) { return fabs(d); }, &sphere_boundary_indices1,
      &sphere_boundary_times1);
  auto sphere_boundary_indices2 = vector<array<int32_t, kDimension>>();
  auto sphere_boundary_times2 = vector<ScalarType>();
  util::HyperSphereBoundaryCells(
      sphere_center2, sphere_radius2, grid_size, grid_spacing,
      [](ScalarType const d) { return fabs(d); }, &sphere_boundary_indices2,
      &sphere_boundary_times2);

  auto boundary_indices = sphere_boundary_indices1;
  auto boundary_times = sphere_boundary_times1;
  for (auto i = size_t{0}; i < sphere_boundary_indices2.size(); ++i) {
    boundary_indices.push_back(sphere_boundary_indices2[i]);
    boundary_times.push_back(sphere_boundary_times2[i]);
  }

  // Act.
  auto const ft = util::FunctionThrows<invalid_argument>([=]() {
    auto const unsigned_distance = fmm::UnsignedArrivalTime(
        grid_size, boundary_indices, boundary_times,
        EikonalSolverType(grid_spacing, uniform_speed));
  });

  // Assert.
  ASSERT_TRUE(ft.first);
  ASSERT_EQ("contained component", ft.second);
}

TYPED_TEST(UnsignedArrivalTimeTest, DifferentUniformSpeed) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});

  auto boundary_indices = vector<array<int32_t, kDimension>>();
  boundary_indices.push_back(util::FilledArray<kDimension>(int32_t{5}));

  auto boundary_times = vector<ScalarType>();
  boundary_times.push_back(ScalarType{0});

  auto const speed1 = ScalarType{1};
  auto const speed2 = ScalarType{2};

  // Act.
  auto const unsigned_times1 =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                               EikonalSolverType(grid_spacing, speed1));

  auto const unsigned_times2 =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                               EikonalSolverType(grid_spacing, speed2));

  // Assert.
  // Check that the distance is halved when the speed is halved.
  // Note that the boundary distance is zero, which also passes this check.
  for (auto i = size_t{0}; i < unsigned_times1.size(); ++i) {
    auto const d1 = unsigned_times1[i];
    auto const d2 = unsigned_times2[i];
    ASSERT_LE(fabs(speed1 * d1 - speed2 * d2), ScalarType(1e-3));
  }
}

TYPED_TEST(UnsignedArrivalTimeTest, VaryingSpeed) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::VaryingSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{11});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});

  auto boundary_indices = vector<array<int32_t, kDimension>>();
  boundary_indices.push_back(util::FilledArray<kDimension>(int32_t{5}));

  auto boundary_times = vector<ScalarType>();
  boundary_times.push_back(ScalarType{0});

  auto const speed_grid_size = grid_size;
  auto speed_buffer = vector<ScalarType>(util::LinearSize(speed_grid_size));
  auto speed_grid =
      util::Grid<ScalarType, kDimension>(speed_grid_size, speed_buffer.front());
  auto const speed = ScalarType(1);
  auto const mirror_speed = ScalarType(2);
  auto speed_index_iter = util::IndexIterator<kDimension>(speed_grid.size());
  while (speed_index_iter.has_next()) {
    auto const index = speed_index_iter.index();
    if (index[0] < boundary_indices[0][0]) {
      speed_grid.Cell(index) = mirror_speed;
    } else {
      speed_grid.Cell(index) = speed;
    }
    speed_index_iter.Next();
  }

  // Act.
  auto unsigned_times = fmm::UnsignedArrivalTime(
      grid_size, boundary_indices, boundary_times,
      EikonalSolverType(grid_spacing, speed_grid_size, speed_buffer));

  // Assert.
  auto time_grid =
      util::Grid<ScalarType, kDimension>(grid_size, unsigned_times.front());

  auto time_index_iter = util::IndexIterator<kDimension>(grid_size);
  while (time_index_iter.has_next()) {
    auto const index = time_index_iter.index();
    auto mid = true;
    for (auto i = size_t{1}; i < kDimension; ++i) {
      if (index[i] != grid_size[i] / 2) {
        mid = false;
        break;
      }
    }
    if (index[0] > boundary_indices[0][0] && mid) {
      auto mirror_index = index;
      mirror_index[0] = 2 * boundary_indices[0][0] - index[0];
      auto const time = time_grid.Cell(index);
      auto const mirror_time = time_grid.Cell(mirror_index);
      ASSERT_NEAR(time * speed, mirror_time * mirror_speed, 1e-6);
    }
    time_index_iter.Next();
  }
}

TYPED_TEST(UnsignedArrivalTimeTest, NonUniformGridSpacing) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{31});
  auto grid_spacing = util::FilledArray<kDimension>(ScalarType{0});
  for (auto i = size_t{0}; i < kDimension; ++i) {
    grid_spacing[i] = ScalarType{1} / (i + 1);
  }
  auto const uniform_speed = ScalarType(1);

  auto boundary_indices = vector<array<int32_t, kDimension>>(
      size_t{1}, util::FilledArray<kDimension>(int32_t{15}));
  auto boundary_times = vector<ScalarType>(size_t{1}, ScalarType{0});

  // Act.
  auto unsigned_times =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                               EikonalSolverType(grid_spacing, uniform_speed));

  // Assert.
  auto time_grid =
      util::Grid<ScalarType, kDimension>(grid_size, unsigned_times.front());
  auto center_position = util::FilledArray<kDimension>(ScalarType(0));
  for (auto i = size_t{0}; i < kDimension; ++i) {
    center_position[i] =
        (boundary_indices[0][i] + ScalarType(0.5)) * grid_spacing[i];
  }
  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  while (index_iter.has_next()) {
    auto const index = index_iter.index();
    auto position = util::FilledArray<kDimension>(ScalarType(0));
    for (auto i = size_t{0}; i < kDimension; ++i) {
      position[i] = (index[i] + ScalarType(0.5)) * grid_spacing[i];
    }
    auto delta = util::FilledArray<kDimension>(ScalarType(0));
    for (auto i = size_t{0}; i < kDimension; ++i) {
      delta[i] = center_position[i] - position[i];
    }
    auto const gt = util::Magnitude(delta);

    auto const time = time_grid.Cell(index);
    auto const time_abs_error = fabs(time - gt);

    typedef util::PointSourceAccuracyBounds<kDimension> Bounds;
    ASSERT_LE(time_abs_error, ScalarType(Bounds::max_abs_error()));

    index_iter.Next();
  }
}

TYPED_TEST(UnsignedArrivalTimeTest, BoxBoundary) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr size_t kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});

  auto boundary_indices = vector<array<int32_t, kDimension>>();
  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  while (index_iter.has_next()) {
    auto const index = index_iter.index();
    for (auto i = size_t{0}; i < kDimension; ++i) {
      if (index[i] == 0 || index[i] == grid_size[i] - 1) {
        boundary_indices.push_back(index);
        break;
      }
    }
    index_iter.Next();
  }

  auto boundary_times =
      vector<ScalarType>(boundary_indices.size(), ScalarType{0});

  auto const speed = ScalarType{1};

  // Act.
  auto const unsigned_times =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                               EikonalSolverType(grid_spacing, speed));

  // Assert.
  for (auto const unsigned_time : unsigned_times) {
    ASSERT_GE(unsigned_time, ScalarType{0});
  }
}

TYPED_TEST(UnsignedArrivalTimeTest, Checkerboard) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr auto kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{10});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});

  auto const is_even = [](auto const i) { return i % 2 == 0; };
  auto boundary_indices = vector<array<int32_t, kDimension>>();
  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  while (index_iter.has_next()) {
    auto const index = index_iter.index();
    if (all_of(begin(index), end(index), is_even) ||
        none_of(begin(index), end(index), is_even)) {
      boundary_indices.push_back(index);
    }
    index_iter.Next();
  }

  auto boundary_times =
      vector<ScalarType>(boundary_indices.size(), ScalarType{0});

  auto const speed = ScalarType{1};

  // Act.
  auto unsigned_times =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                               EikonalSolverType(grid_spacing, speed));

  // Assert.
  auto time_grid =
      util::Grid<ScalarType, kDimension>(grid_size, unsigned_times.front());
  {
    auto index_iter = util::IndexIterator<kDimension>(grid_size);
    while (index_iter.has_next()) {
      auto const index = index_iter.index();
      auto const is_boundary = all_of(begin(index), end(index), is_even) ||
                               none_of(begin(index), end(index), is_even);
      if (!is_boundary) {
        auto const time = time_grid.Cell(index);
        ASSERT_GT(time, ScalarType{0});
      }
      index_iter.Next();
    }
  }
}

TYPED_TEST(UnsignedArrivalTimeTest, OverlappingBoxes) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr auto kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{16});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const uniform_speed = ScalarType{1};

  auto box_corner1 = util::FilledArray<kDimension>(int32_t{1});
  auto box_size1 = util::FilledArray<kDimension>(size_t{10});
  auto box_corner2 = util::FilledArray<kDimension>(int32_t{5});
  auto box_size2 = util::FilledArray<kDimension>(size_t{10});

  auto box_boundary_indices1 = vector<array<int32_t, kDimension>>();
  auto box_boundary_times1 = vector<ScalarType>();
  util::BoxBoundaryCells(box_corner1, box_size1, grid_size,
                         &box_boundary_indices1, &box_boundary_times1);
  auto box_boundary_indices2 = vector<array<int32_t, kDimension>>();
  auto box_boundary_times2 = vector<ScalarType>();
  util::BoxBoundaryCells(box_corner2, box_size2, grid_size,
                         &box_boundary_indices2, &box_boundary_times2);

  // Merge and remove duplicate indices.
  auto boundary_indices = box_boundary_indices1;
  auto boundary_distances = box_boundary_times1;
  for (auto i = size_t{0}; i < box_boundary_indices2.size(); ++i) {
    auto const index = box_boundary_indices2[i];
    if (find(begin(boundary_indices), end(boundary_indices), index) ==
        end(boundary_indices)) {
      boundary_indices.push_back(index);
      boundary_distances.push_back(box_boundary_times2[i]);
    }
  }

  // Act.
  auto unsigned_times =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_distances,
                               EikonalSolverType(grid_spacing, uniform_speed));

  // Assert.
  auto time_grid =
      util::Grid<ScalarType, kDimension>(grid_size, unsigned_times.front());
  auto const time0 = time_grid.Cell(util::FilledArray<kDimension>(int32_t{0}));
  auto const time2 = time_grid.Cell(util::FilledArray<kDimension>(int32_t{2}));
  auto const time6 = time_grid.Cell(util::FilledArray<kDimension>(int32_t{6}));
  auto const time14 =
      time_grid.Cell(util::FilledArray<kDimension>(int32_t{14}));
  ASSERT_GT(time0, ScalarType{0});
  ASSERT_GT(time2, ScalarType{0});
  ASSERT_GT(time6, ScalarType{0});
  ASSERT_GT(time14, ScalarType{0});
}

TYPED_TEST(UnsignedArrivalTimeAccuracyTest, PointSourceAccuracy) {
  using namespace std;

  typedef TypeParam::ScalarType ScalarType;
  static constexpr auto kDimension = TypeParam::kDimension;
  namespace fmm = thinks::fast_marching_method;
  typedef fmm::UniformSpeedEikonalSolver<ScalarType, kDimension>
      EikonalSolverType;
  typedef fmm::DistanceSolver<ScalarType, kDimension> DistanceSolverType;
  typedef fmm::HighAccuracyUniformSpeedEikonalSolver<ScalarType, kDimension>
      HighAccuracyEikonalSolverType;

  // Arrange.
  auto const grid_size = util::FilledArray<kDimension>(size_t{41});
  auto const grid_spacing = util::FilledArray<kDimension>(ScalarType{1});
  auto const uniform_speed = ScalarType(1);

  // Simple point boundary for regular fast marching.
  auto boundary_indices = vector<array<int32_t, kDimension>>(
      size_t{1}, util::FilledArray<kDimension>(int32_t{20}));
  auto boundary_times = vector<ScalarType>(size_t{1}, ScalarType{0});

  auto center_position = util::FilledArray<kDimension>(ScalarType(0));
  for (auto i = size_t{0}; i < kDimension; ++i) {
    center_position[i] =
        (boundary_indices[0][i] + ScalarType(0.5)) * grid_spacing[i];
  }

  // Compute exact distances in vertex neighborhood for high accuracy
  // fast marching.
  auto ha_boundary_indices = vector<array<int32_t, kDimension>>();
  ha_boundary_indices.push_back(
      util::FilledArray<kDimension>(int32_t{20}));  // Center.
  auto const vtx_neighbor_offsets = util::VertexNeighborOffsets<kDimension>();
  for (auto const& vtx_neighbor_offset : vtx_neighbor_offsets) {
    auto index = boundary_indices[0];
    for (auto i = size_t{0}; i < kDimension; ++i) {
      index[i] += vtx_neighbor_offset[i];
    }
    ha_boundary_indices.push_back(index);
  }
  auto ha_boundary_times = vector<ScalarType>();
  ha_boundary_times.push_back(ScalarType{0});  // Center.
  for (auto j = size_t{1}; j < ha_boundary_indices.size(); ++j) {
    auto const& index = ha_boundary_indices[j];
    auto position = util::FilledArray<kDimension>(ScalarType(0));
    for (auto i = size_t{0}; i < kDimension; ++i) {
      position[i] = (index[i] + ScalarType(0.5)) * grid_spacing[i];
    }
    auto delta = util::FilledArray<kDimension>(ScalarType(0));
    for (auto i = size_t{0}; i < kDimension; ++i) {
      delta[i] = center_position[i] - position[i];
    }
    ha_boundary_times.push_back(util::Magnitude(delta));
  }

  // Act.
  auto unsigned_time =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                               EikonalSolverType(grid_spacing, uniform_speed));
  auto unsigned_distance =
      fmm::UnsignedArrivalTime(grid_size, boundary_indices, boundary_times,
                               DistanceSolverType(grid_spacing[0]));
  auto ha_unsigned_time = fmm::UnsignedArrivalTime(
      grid_size, ha_boundary_indices, ha_boundary_times,
      HighAccuracyEikonalSolverType(grid_spacing, uniform_speed));

  // Compute errors.
  auto time_grid =
      util::Grid<ScalarType, kDimension>(grid_size, unsigned_time.front());
  auto distance_grid =
      util::Grid<ScalarType, kDimension>(grid_size, unsigned_distance.front());
  auto ha_time_grid =
      util::Grid<ScalarType, kDimension>(grid_size, ha_unsigned_time.front());

  auto time_abs_errors = vector<ScalarType>();
  auto distance_abs_errors = vector<ScalarType>();
  auto ha_time_abs_errors = vector<ScalarType>();

  auto index_iter = util::IndexIterator<kDimension>(grid_size);
  while (index_iter.has_next()) {
    auto const index = index_iter.index();
    auto position = util::FilledArray<kDimension>(ScalarType(0));
    for (auto i = size_t{0}; i < kDimension; ++i) {
      position[i] = (index[i] + ScalarType(0.5)) * grid_spacing[i];
    }
    auto delta = util::FilledArray<kDimension>(ScalarType(0));
    for (auto i = size_t{0}; i < kDimension; ++i) {
      delta[i] = center_position[i] - position[i];
    }
    auto const gt = util::Magnitude(delta);

    auto const time = time_grid.Cell(index);
    auto const distance = distance_grid.Cell(index);
    auto const ha_time = ha_time_grid.Cell(index);
    auto const time_abs_error = abs(time - gt);
    auto const distance_abs_error = abs(distance - gt);
    auto const ha_time_abs_error = abs(ha_time - gt);
    if (gt <= ScalarType{20}) {
      time_abs_errors.push_back(time_abs_error);
      distance_abs_errors.push_back(distance_abs_error);
      ha_time_abs_errors.push_back(ha_time_abs_error);
    }
    index_iter.Next();
  }

  auto time_max_abs_error = ScalarType{0};
  auto time_avg_abs_error = ScalarType{0};
  for (auto const& time_abs_error : time_abs_errors) {
    time_max_abs_error = max(time_max_abs_error, time_abs_error);
    time_avg_abs_error += time_abs_error;
  }
  time_avg_abs_error /= time_abs_errors.size();

  auto distance_max_abs_error = ScalarType{0};
  auto distance_avg_abs_error = ScalarType{0};
  for (auto const& distance_abs_error : distance_abs_errors) {
    distance_max_abs_error = max(distance_max_abs_error, distance_abs_error);
    distance_avg_abs_error += distance_abs_error;
  }
  distance_avg_abs_error /= distance_abs_errors.size();

  auto ha_time_max_abs_error = ScalarType{0};
  auto ha_time_avg_abs_error = ScalarType{0};
  for (auto const& ha_dist_abs_error : ha_time_abs_errors) {
    ha_time_max_abs_error = max(ha_time_max_abs_error, ha_dist_abs_error);
    ha_time_avg_abs_error += ha_dist_abs_error;
  }
  ha_time_avg_abs_error /= ha_time_abs_errors.size();

  // Assert.
  typedef util::PointSourceAccuracyBounds<kDimension> Bounds;
  ASSERT_LE(time_max_abs_error, ScalarType(Bounds::max_abs_error()));
  ASSERT_LE(time_avg_abs_error, ScalarType(Bounds::avg_abs_error()));
  ASSERT_LE(distance_max_abs_error, ScalarType(Bounds::max_abs_error()));
  ASSERT_LE(distance_avg_abs_error, ScalarType(Bounds::avg_abs_error()));
  ASSERT_LE(ha_time_max_abs_error,
            ScalarType(Bounds::high_accuracy_max_abs_error()));
  ASSERT_LE(ha_time_avg_abs_error,
            ScalarType(Bounds::high_accuracy_avg_abs_error()));
}

}  // namespace
