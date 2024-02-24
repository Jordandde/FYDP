import { createSlice } from "@reduxjs/toolkit";

export const matricesSlice = createSlice({
  name: "matrices",
  initialState: {
    matrices: [
      Array.from({ length: 4 }, () => Array.from({ length: 4 }, () => "0")),
      Array.from({ length: 4 }, () => Array.from({ length: 4 }, () => "0")),
    ],
    rows: 4,
    cols: 4,
  },
  reducers: {
    updateDimensions: (state, action) => {
      const { tempRows, tempCols } = action.payload;
      state.rows = tempRows;
      state.cols = tempCols;
      state.matrices = [
        Array.from({ length: tempRows }, () =>
          Array.from({ length: tempCols }, () => "0")
        ),
        Array.from({ length: tempRows }, () =>
          Array.from({ length: tempCols }, () => "0")
        ),
      ];
    },
    updateValue: (state, action) => {
      const { matrixIndex, row, col, value } = action.payload;
      state.matrices[matrixIndex] = state.matrices[matrixIndex].map((r, ri) =>
        r.map((c, ci) => (ri === row && ci === col ? value : c))
      );
    },
  },
});

export const { updateDimensions, updateValue } = matricesSlice.actions;

export default matricesSlice.reducer;
