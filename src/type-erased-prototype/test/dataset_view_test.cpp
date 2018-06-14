#include <gtest/gtest.h>

#include <boost/mpl/at.hpp>
#include <boost/mpl/sort.hpp>
#include <boost/mpl/vector_c.hpp>

#include "test_macros.h"

#include "dataset_view.h"

TEST(MultidimensionalIndex, end) {
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);
  MultidimensionalIndex i(dims);
  ASSERT_EQ(i.end, (std::vector<gsl::index>{2, 0, 1}));
}

TEST(MultidimensionalIndex, increment) {
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);
  MultidimensionalIndex i(dims);
  ASSERT_EQ(i.index, (std::vector<gsl::index>{0, 0, 0}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{1, 0, 0}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{2, 0, 0}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{0, 0, 1}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{1, 0, 1}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{2, 0, 1}));
}

TEST(LinearSubindex, full_subindex) {
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);
  MultidimensionalIndex i(dims);

  LinearSubindex sub(i, dims);
  gsl::index count{0};
  while (true) {
    ASSERT_EQ(sub.get(), count++);
    if (i.index == i.end)
      break;
    i.increment();
  }
}

TEST(LinearSubindex, zero_dimensional_subindex) {
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);
  MultidimensionalIndex i(dims);

  LinearSubindex sub(i, Dimensions{});
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
}

TEST(LinearSubindex, fast_1_dimensional_subindex) {
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);
  MultidimensionalIndex i(dims);

  LinearSubindex sub(i, Dimensions(Dimension::Tof, 3));
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 2);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 2);
}

TEST(LinearSubindex, slow_1_dimensional_subindex) {
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);
  MultidimensionalIndex i(dims);

  LinearSubindex sub(i, Dimensions(Dimension::Q, 2));
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
}

TEST(LinearSubindex, flipped_2_dimensional_subindex) {
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);
  MultidimensionalIndex i(dims);
  Dimensions subdims;
  subdims.add(Dimension::Q, 2);
  subdims.add(Dimension::Tof, 3);

  LinearSubindex sub(i, subdims);
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 2);
  i.increment();
  ASSERT_EQ(sub.get(), 4);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 3);
  i.increment();
  ASSERT_EQ(sub.get(), 5);
}

TEST(DatasetView, construct) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions{}, {1.1});
  d.insert<Data::Int>("name2", Dimensions{}, {2l});
  // Empty view forbidden by static_assert:
  //DatasetView<> view(d);
  ASSERT_NO_THROW(DatasetView<Data::Value> view(d));
  ASSERT_NO_THROW(DatasetView<Data::Int> view(d));
  ASSERT_NO_THROW(auto view = (DatasetView<Data::Int, Data::Value>(d)));
  ASSERT_THROW(auto view = (DatasetView<Data::Int, Data::Error>(d)),
               std::runtime_error);
}

TEST(DatasetView, single_column) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions(Dimension::Tof, 10), 10);
  d.insert<Data::Int>("name2", Dimensions(Dimension::Tof, 10), 10);
  auto &var = d.get<Data::Value>();
  var[0] = 0.2;
  var[3] = 3.2;

  DatasetView<Data::Value> view(d);
  ASSERT_EQ(view.get<Data::Value>(), 0.2);
  view.increment();
  ASSERT_EQ(view.get<Data::Value>(), 0.0);
  view.increment();
  ASSERT_EQ(view.get<Data::Value>(), 0.0);
  view.increment();
  ASSERT_EQ(view.get<Data::Value>(), 3.2);
}

TEST(DatasetView, multi_column) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions(Dimension::Tof, 2), 2);
  d.insert<Data::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Data::Value>();
  var[0] = 0.2;
  var[1] = 3.2;

  DatasetView<Data::Value, Data::Int> view(d);
  ASSERT_EQ(view.get<Data::Value>(), 0.2);
  ASSERT_EQ(view.get<Data::Int>(), 0);
  view.increment();
  ASSERT_EQ(view.get<Data::Value>(), 3.2);
  ASSERT_EQ(view.get<Data::Int>(), 0);
}

TEST(DatasetView, multi_column_mixed_dimension) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions(Dimension::Tof, 2), 2);
  d.insert<Data::Int>("name2", Dimensions{}, 1);
  auto &var = d.get<Data::Value>();
  var[0] = 0.2;
  var[1] = 3.2;

  ASSERT_ANY_THROW(auto view =
                       (DatasetView<Data::Value, Data::Int>(d)));
  ASSERT_NO_THROW(auto view =
                      (DatasetView<Data::Value, const Data::Int>(d)));
  auto view = (DatasetView<Data::Value, const Data::Int>(d));
  ASSERT_EQ(view.get<Data::Value>(), 0.2);
  ASSERT_EQ(view.get<const Data::Int>(), 0);
  view.increment();
  ASSERT_EQ(view.get<Data::Value>(), 3.2);
  ASSERT_EQ(view.get<const Data::Int>(), 0);
}

TEST(DatasetView, multi_column_unrelated_dimension) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions(Dimension::X, 2), 2);
  d.insert<Data::Int>("name2", Dimensions(Dimension::Y, 3), 3);
  DatasetView<Data::Value> view(d);
  view.increment();
  // We iterate only Data::Value, so there should be no iteration in
  // Dimension::Y.
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, multi_column_orthogonal_fail) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions(Dimension::X, 2), 2);
  d.insert<Data::Int>("name2", Dimensions(Dimension::Y, 3), 3);
  EXPECT_THROW_MSG((DatasetView<Data::Value, Data::Int>(d)), std::runtime_error,
                   "Variables requested for iteration do not span a joint "
                   "space. In case one of the variables represents bin edges "
                   "direct joint iteration is not possible. Use the Bins<> "
                   "wrapper to iterate over bins defined by edges instead.");
}

TEST(DatasetView, multi_column_mixed_dimension_with_slab) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions(Dimension::Tof, 2), 2);
  d.insert<Data::Int>("name2", Dimensions{}, 1);
  auto &var = d.get<Data::Value>();
  var[0] = 0.2;
  var[1] = 3.2;

  // Should fixed dimension be generic, or should we just provide a couple of
  // special cases, in particular for Tof?
  // Use direct column access otherwise (how to access things like Tof slices?)?
  // YAGNI? Just support a single fixed dimension, given at compile time?!
  // We might want to iterate all BinEdges, getting a slab of corresponding
  // counts (e.g., all spectra for certain polarization)?! That is,
  // Dimension::Tof and Dimension::SpectrumNumber might be fixed, while
  // Dimension::Polarization is running. Or Dimension::Tof and
  // Dimension::Polarization are fixed file Dimension::SpectrumNumber is
  // running. The latter is more likely to be expressed as "iterate only
  // SpectrumNumber", how do we handle that in general?
  // Maybe a better way to say this is: Iterate all dimensions of BinEdges. In
  // general we do not know which other columns need to be accessed as slabs,
  // how can we deal with this? Just access all as slab (which may be size 1)?
  DatasetView<Slab<Data::Value>, Data::Int> view(d, {Dimension::Tof});
  // view.get<double>(); // Does not compile, since we cannot get a single
  // double.
  view.get<Data::Int>();
}

TEST(DatasetView, single_column_edges) {
  Dataset d;
  auto edges = makeDataArray<Data::Value>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, edges);
  d.insert<Data::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Data::Value>();
  ASSERT_EQ(var.size(), 3);
  var[0] = 0.2;
  var[2] = 2.2;

  DatasetView<Data::Value> view(d);
  ASSERT_EQ(view.get<Data::Value>(), 0.2);
  view.increment();
  ASSERT_EQ(view.get<Data::Value>(), 0.0);
  ASSERT_FALSE(view.atLast());
  view.increment();
  ASSERT_EQ(view.get<Data::Value>(), 2.2);
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, single_column_bins) {
  Dataset d;
  auto edges = makeDataArray<Data::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, edges);
  d.insert<Data::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Data::Tof>();
  ASSERT_EQ(var.size(), 3);
  var[0] = 0.2;
  var[1] = 1.2;
  var[2] = 2.2;

  DatasetView<Bins<Data::Tof>> view(d);
  view.increment();
  // Lenth of edges is 3, but there are only 2 bins!
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, multi_column_edges) {
  Dataset d;
  auto edges = makeDataArray<Data::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, edges);
  d.insert<Data::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Data::Tof>();
  var[0] = 0.2;
  var[1] = 1.2;
  var[2] = 2.2;

  // Cannot simultaneously iterate edges and non-edges, so this throws.
  EXPECT_THROW_MSG((DatasetView<Data::Tof, Data::Int>(d)), std::runtime_error,
                   "Variables requested for iteration do not span a joint "
                   "space. In case one of the variables represents bin edges "
                   "direct joint iteration is not possible. Use the Bins<> "
                   "wrapper to iterate over bins defined by edges instead.");

  DatasetView<Bins<Data::Tof>, Data::Int> view(d);
  // TODO Singular 'Bin' instead of 'Bins' would make more sense.
  // TODO What are good names for named getters? tofCenter(), etc.?
  const auto &bin = view.get<Bins<Data::Tof>>();
  EXPECT_EQ(bin.center(), 0.7);
  EXPECT_EQ(bin.width(), 1.0);
  EXPECT_EQ(bin.left(), 0.2);
  EXPECT_EQ(bin.right(), 1.2);
  view.increment();
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, named_getter) {
  Dataset d;
  auto tof = makeDataArray<Data::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insert(tof);
  auto &var = d.get<Data::Tof>();
  ASSERT_EQ(var.size(), 3);
  var[0] = 0.2;
  var[2] = 2.2;

  DatasetView<Data::Tof> view(d);
  ASSERT_EQ(view.tof(), 0.2);
  view.increment();
  ASSERT_EQ(view.tof(), 0.0);
  view.increment();
  ASSERT_EQ(view.tof(), 2.2);
}

TEST(DatasetView, duplicate_data_tag) {
  Dataset d;
  d.insert<Data::Value>("name1", Dimensions{}, 1);
  d.insert<Data::Value>("name2", Dimensions{}, 1);

  EXPECT_THROW_MSG(DatasetView<Data::Value> view(d), std::runtime_error,
                   "Given variable tag is not unique. Must provide a name.");
  EXPECT_NO_THROW(DatasetView<Data::Value> view(d, "name2"));
}

TEST(DatasetView, histogram) {
  Dataset d;
  auto tof = makeDataArray<Data::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, tof);
  Dimensions dims;
  dims.add(Dimension::Tof, 2);
  dims.add(Dimension::Spectrum, 4);
  d.insert<Data::Value>("sample", dims,
                            {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
  d.insert<Data::Error>("sample", dims, 8);

  DatasetView<Data::Histogram> view(d, {Dimension::Tof});
  EXPECT_EQ(view.get<Data::Histogram>().value(0), 1.0);
  EXPECT_EQ(view.get<Data::Histogram>().value(1), 2.0);
  view.get<Data::Histogram>().value(1) += 0.2;
  EXPECT_EQ(d.get<Data::Value>()[1], 2.2);
  view.increment();
  EXPECT_EQ(view.get<Data::Histogram>().value(0), 3.0);
}

template <class T> constexpr int type_to_id();
template <> constexpr int type_to_id<double>() { return 0; }
template <> constexpr int type_to_id<int>() { return 1; }
template <> constexpr int type_to_id<char>() { return 2; }

template <int N> struct id_to_type;
template <> struct id_to_type<0> { using type = double; };
template <> struct id_to_type<1> { using type = int; };
template <> struct id_to_type<2> { using type = char; };
template <int N> using id_to_type_t = typename id_to_type<N>::type;

template <class Sorted, size_t... Is>
auto sort_types_impl(std::index_sequence<Is...>) {
  return std::tuple<
      id_to_type_t<boost::mpl::at_c<Sorted, Is>::type::value>...>{};
}

template <class... Ts> auto sort_types() {
  using Unsorted = boost::mpl::vector_c<int, type_to_id<Ts>()...>;
  return sort_types_impl<typename boost::mpl::sort<Unsorted>::type>(
      std::make_index_sequence<sizeof...(Ts)>{});
}

// Named "Set" because the order of types in the declaration does not matter,
// yields the same type.
template <class... Ts> using Set = decltype(sort_types<Ts...>());

TEST(SortTypes, same) {
  using unsorted1 = boost::mpl::vector_c<int, 4, 3, 1>;
  using unsorted2 = boost::mpl::vector_c<int, 4, 1, 3>;
  ASSERT_EQ(typeid(boost::mpl::sort<unsorted1>::type),
            typeid(boost::mpl::sort<unsorted2>::type));
}

TEST(SortTypes, different) {
  using unsorted1 = boost::mpl::vector_c<int, 4, 3, 1>;
  using unsorted2 = boost::mpl::vector_c<int, 4, 1, 2>;
  ASSERT_NE(typeid(boost::mpl::sort<unsorted1>::type),
            typeid(boost::mpl::sort<unsorted2>::type));
}

TEST(SortTypes, sort) {
  auto t = sort_types<char, double, int>();
  ASSERT_EQ(typeid(decltype(t)), typeid(std::tuple<double, int, char>));
}

TEST(SortTypes, type) {
  Set<char, double, int> a;
  Set<double, char, int> b;
  ASSERT_EQ(typeid(decltype(a)), typeid(decltype(b)));
}
