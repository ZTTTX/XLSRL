// Copyright 2023 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
// Bitonic sort
// A parametric function for sort an array of u32.
// The array size must be a power of two.
// The parametric takes the size of the array as parameter, but that size can
// be inferred too.
//
// For example, assuming "array" is of type u32[8].
// Then, bitonic_sort(array) will return the sorted array.
//
// Source: https://en.wikipedia.org/wiki/Bitonic_sorter
////////////////////////////////////////////////////////////////////////////////

import std

fn swap<B: u32, N: u32>(array: uN[B][N], i: u32, j: u32) -> uN[B][N] {
    let i_element = array[i];
    let j_element = array[j];
    update(update(array, i, j_element), j, i_element)
}

fn bitonic_sort<N: u32, LOG_N: u32 = { std::clog2(N) }>(array: u32[N]) -> u32[N] {
    const_assert!(std::is_pow2(N));
    let (result, _) = for (_, (array, k)) in u32:0..LOG_N {
        let (result, _) = for (_, (array, j)) in u32:0..LOG_N {
            if (j == u32:0) {
                (array, j)
            } else {
                let result = for (i, array) in u32:0..N {
                    let l = i ^ j;
                    if (l <= i) {
                        (array)
                    } else {
                        if ( (((i & k) == u32:0) && (array[i] > array[l])) || (((i & k) != u32:0) && (array[i] < array[l])) ) {
                            swap(array, i, l)
                        } else {
                            (array)
                        }
                    }
                } (array);
                (result, j / u32:2)
            }
        } ((array, k / u32:2));
        (result, k * u32:2)
    } ((array, u32:2));
    result
}

////////////////////////////////////////////////////////////////////////////////
