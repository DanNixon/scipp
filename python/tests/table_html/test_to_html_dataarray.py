import numpy as np
import pytest
from bs4 import BeautifulSoup

import scipp as sc
from scipp import Dim
from scipp.table_html import make_html

from .common import (ATTR_NAME, LABEL_NAME, MASK_NAME, assert_dims_section,
                     assert_section)


@pytest.mark.parametrize("dims, lengths",
                         (([Dim.X], (10, )), ([Dim.X, Dim.Y], (10, 10)),
                          ([Dim.X, Dim.Y, Dim.Z],
                           (10, 10, 10)), ([Dim.X, Dim.Y, Dim.Z, Dim.Spectrum],
                                           (10, 10, 10, 10))))
def test_basic(dims, lengths):
    in_unit = sc.units.m
    in_dtype = sc.dtype.float32
    data = sc.Variable(dims,
                       unit=in_unit,
                       dtype=in_dtype,
                       values=np.random.rand(*lengths),
                       variances=np.random.rand(*lengths))

    data_array = sc.DataArray(data,
                              coords={dims[0]: data},
                              attrs={ATTR_NAME: data},
                              labels={LABEL_NAME: data},
                              masks={MASK_NAME: data})

    html = BeautifulSoup(make_html(data_array), features="html.parser")
    sections = html.find_all(class_="xr-section-item")
    assert len(sections) == 6
    expected_sections = [
        "Dimensions", "Coordinates", "Labels", "Data", "Masks", "Attributes"
    ]
    for actual_section, expected_section in zip(sections, expected_sections):
        assert expected_section in actual_section.text

    dim_section = sections.pop(0)
    assert_dims_section(data, dim_section)

    data_names = [
        dims[0],
        LABEL_NAME,
        "",  # dataarray does not have a data name
        MASK_NAME,
        ATTR_NAME
    ]
    assert len(sections) == len(
        data_names), "Sections and expected data names do not match"
    for section, name in zip(sections, data_names):
        assert_section(section, name, dims, in_dtype, in_unit)


@pytest.mark.parametrize("dims, lengths",
                         (([Dim.X, Dim.Y], (10, sc.Dimensions.Sparse)),
                          ([Dim.X, Dim.Y, Dim.Z],
                           (10, 10, sc.Dimensions.Sparse)),
                          ([Dim.X, Dim.Y, Dim.Z, Dim.Spectrum],
                           (10, 10, 10, sc.Dimensions.Sparse))))
def test_sparse(dims, lengths):
    in_unit = sc.units.m
    in_dtype = sc.dtype.float32

    data = sc.Variable(dims, lengths, unit=in_unit, dtype=in_dtype)

    in_attr_dims = [dims[0]]
    attr = sc.Variable(in_attr_dims,
                       unit=in_unit,
                       dtype=in_dtype,
                       values=np.random.rand(lengths[0]))

    data_array = sc.DataArray(data,
                              coords={dims[0]: data},
                              attrs={ATTR_NAME: attr},
                              labels={LABEL_NAME: attr},
                              masks={MASK_NAME: attr})
    html = BeautifulSoup(make_html(data_array), features="html.parser")
    sections = html.find_all(class_="xr-section-item")

    assert len(sections) == 6
    expected_sections = [
        "Dimensions", "Coordinates", "Labels", "Data", "Masks", "Attributes"
    ]
    for actual_section, expected_section in zip(sections, expected_sections):
        assert expected_section in actual_section.text

    data_section = sections.pop(3)
    assert_section(data_section, "", dims, in_dtype, in_unit, has_sparse=True)

    dim_section = sections.pop(0)
    assert_dims_section(data, dim_section)

    coord_section = sections.pop(0)
    # the original dim used as a label (dim[0]) is not shown,
    # instead the sparse dim is shown
    assert_section(coord_section,
                   dims[-1],
                   dims,
                   in_dtype,
                   in_unit,
                   has_sparse=True)

    data_names = [LABEL_NAME, MASK_NAME, ATTR_NAME]

    for section, name in zip(sections, data_names):
        assert_section(section, name, in_attr_dims, in_dtype, in_unit)


@pytest.mark.parametrize(
    "dims, lengths", (([Dim.X], [10]), ([Dim.X, Dim.Y], [10, 10]),
                      ([Dim.X, Dim.Y, Dim.Z], [10, 10, 10]),
                      ([Dim.X, Dim.Y, Dim.Z, Dim.Spectrum], [10, 10, 10, 10])))
def test_bin_edge(dims, lengths):
    in_unit = sc.units.m
    in_dtype = sc.dtype.float32
    data = sc.Variable(dims,
                       unit=in_unit,
                       dtype=in_dtype,
                       values=np.random.rand(*lengths),
                       variances=np.random.rand(*lengths))
    # makes the first dimension be bin-edges
    lengths[0] += 1
    edges = sc.Variable(dims,
                        unit=in_unit,
                        dtype=in_dtype,
                        values=np.random.rand(*lengths))

    data_array = sc.DataArray(data,
                              coords={dims[0]: edges},
                              attrs={ATTR_NAME: data},
                              labels={LABEL_NAME: edges},
                              masks={MASK_NAME: edges})

    html = BeautifulSoup(make_html(data_array), features="html.parser")
    sections = html.find_all(class_="xr-section-item")
    assert len(sections) == 6
    expected_sections = [
        "Dimensions", "Coordinates", "Labels", "Data", "Masks", "Attributes"
    ]
    for actual_section, expected_section in zip(sections, expected_sections):
        assert expected_section in actual_section.text

    attr_section = sections.pop(len(sections) - 1)
    assert_section(attr_section, ATTR_NAME, dims, in_dtype, in_unit)

    data_section = sections.pop(3)
    assert_section(data_section, "", dims, in_dtype, in_unit)

    dim_section = sections.pop(0)
    assert_dims_section(data, dim_section)

    data_names = [dims[0], LABEL_NAME, MASK_NAME]

    for section, name in zip(sections, data_names):
        assert_section(section,
                       name,
                       dims,
                       in_dtype,
                       in_unit,
                       has_bin_edges=True)


@pytest.mark.parametrize(
    "dims, lengths",
    (([Dim.X, Dim.Y], [10, sc.Dimensions.Sparse]),
     ([Dim.X, Dim.Y, Dim.Z], [10, 10, sc.Dimensions.Sparse]),
     ([Dim.X, Dim.Y, Dim.Z, Dim.Spectrum], [10, 10, 10, sc.Dimensions.Sparse]))
)
def test_bin_edge_and_sparse(dims, lengths):
    in_unit = sc.units.m
    in_dtype = sc.dtype.float32

    data = sc.Variable(dims, lengths, unit=in_unit, dtype=in_dtype)

    # attribute data without the sparse dimension
    non_sparse_dims = dims[:-1]
    non_sparse_data = sc.Variable(non_sparse_dims,
                                  lengths[:-1],
                                  unit=in_unit,
                                  dtype=in_dtype)

    # makes the first dimension be bin-edges
    lengths[0] += 1
    non_sparse_bin_edges = sc.Variable(non_sparse_dims,
                                       lengths[:-1],
                                       unit=in_unit,
                                       dtype=in_dtype)

    data_array = sc.DataArray(data,
                              coords={dims[0]: non_sparse_bin_edges},
                              attrs={ATTR_NAME: non_sparse_data},
                              labels={LABEL_NAME: non_sparse_bin_edges},
                              masks={MASK_NAME: non_sparse_bin_edges})

    html = BeautifulSoup(make_html(data_array), features="html.parser")
    sections = html.find_all(class_="xr-section-item")

    expected_sections = [
        "Dimensions", "Coordinates", "Labels", "Data", "Masks", "Attributes"
    ]
    assert len(sections) == 6
    for actual_section, expected_section in zip(sections, expected_sections):
        assert expected_section in actual_section.text

    attr_section = sections.pop(len(sections) - 1)
    assert_section(attr_section, ATTR_NAME, non_sparse_dims, in_dtype, in_unit)

    data_section = sections.pop(3)
    assert_section(data_section, "", dims, in_dtype, in_unit, has_sparse=True)

    dim_section = sections.pop(0)
    assert_dims_section(data, dim_section)

    data_names = [dims[0], LABEL_NAME, MASK_NAME]

    for section, name in zip(sections, data_names):
        assert_section(section,
                       name,
                       non_sparse_dims,
                       in_dtype,
                       in_unit,
                       has_bin_edges=True)
