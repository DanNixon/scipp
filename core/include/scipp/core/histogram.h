// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#ifndef SCIPP_CORE_HISTOGRAM_H
#define SCIPP_CORE_HISTOGRAM_H

#include <algorithm>
#include <tuple>

#include "scipp/core/except.h"

namespace scipp::core {

SCIPP_CORE_EXPORT DataArray histogram(const DataArrayConstView &realigned);
SCIPP_CORE_EXPORT Dataset histogram(const DatasetConstView &realigned);

SCIPP_CORE_EXPORT Dim edge_dimension(const DataArrayConstView &a);
SCIPP_CORE_EXPORT bool is_histogram(const DataArrayConstView &a, const Dim dim);

/// Return params for computing bin index for linear edges (constant bin width).
constexpr static auto linear_edge_params = [](const auto &edges) {
  auto len = scipp::size(edges) - 1;
  const auto offset = edges.front();
  const auto nbin = static_cast<decltype(offset)>(len);
  const auto scale = static_cast<double>(nbin) / (edges.back() - edges.front());
  return std::tuple{offset, nbin, scale};
};

namespace expect::histogram {
template <class T> void sorted_edges(const T &edges) {
  if (!std::is_sorted(edges.begin(), edges.end()))
    throw except::BinEdgeError("Bin edges of histogram must be sorted.");
}
} // namespace expect::histogram

} // namespace scipp::core

#endif // SCIPP_CORE_HISTOGRAM_H
