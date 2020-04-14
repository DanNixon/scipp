// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
#include <gtest/gtest.h>

#include "scipp/dataset/dataset.h"
#include "scipp/variable/variable.h"
#include "scipp/variable/variable_reduction.h"

using namespace scipp;
using namespace scipp::dataset;

namespace {
auto make_sparse() {
  auto var = makeVariable<event_list<double>>(Dims{Dim::Y}, Shape{3});
  const auto &var_ = var.values<event_list<double>>();
  var_[0] = {1, 2, 3};
  var_[1] = {4, 5};
  var_[2] = {6, 7};
  return var;
}
} // namespace

TEST(ReduceSparseTest, flatten_fail) {
  EXPECT_THROW(static_cast<void>(flatten(make_sparse(), Dim::X)),
               except::DimensionError);
  EXPECT_THROW(static_cast<void>(flatten(make_sparse(), Dim::Z)),
               except::DimensionError);
}

TEST(ReduceSparseTest, flatten) {
  auto expected = makeVariable<event_list<double>>(
      Dims{}, Shape{}, Values{event_list<double>{1, 2, 3, 4, 5, 6, 7}});
  EXPECT_EQ(flatten(make_sparse(), Dim::Y), expected);
}

TEST(ReduceSparseTest, flatten_dataset_with_mask) {
  Dataset d;
  d.setMask("y", makeVariable<bool>(Dims{Dim::Y}, Shape{3},
                                    Values{false, true, false}));
  d.coords().set(Dim::X, make_sparse());
  d.coords().set(Dim("label"), make_sparse());
  d.setData("b", make_sparse());
  auto expected = makeVariable<event_list<double>>(
      Dims{}, Shape{}, Values{event_list<double>{1, 2, 3, 6, 7}});

  const auto flat = flatten(d, Dim::Y);

  EXPECT_EQ(flat["b"].coords()[Dim::X], expected);
  EXPECT_EQ(flat["b"].coords()[Dim("label")], expected);
  EXPECT_EQ(flat["b"].data(), expected);
}

TEST(ReduceSparseTest, flatten_dataset_non_constant_scalar_weight_fail) {
  Dataset d;
  d.coords().set(Dim::X, make_sparse());
  d.setData("b", makeVariable<double>(Dims{Dim::Y}, Shape{3}, Values{1, 2, 3}));
  EXPECT_THROW(flatten(d, Dim::Y), except::EventDataError);
  d.setData("b", makeVariable<double>(Dims{Dim::Y}, Shape{3}, Values{1, 1, 1}));
  EXPECT_NO_THROW(flatten(d, Dim::Y));
}
