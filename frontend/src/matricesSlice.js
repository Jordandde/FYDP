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
    third: 4,
  },
  reducers: {
    updateDimensions: (state, action) => {
      const { tempRows, tempCols,tempThird } = action.payload;
      state.rows = tempRows;
      state.cols = tempCols;
      state.third = tempThird;
      state.matrices = [
        Array.from({ length: tempRows }, () =>
          Array.from({ length: tempCols }, () => "0")
        ),
        Array.from({ length: tempCols}, () =>
          Array.from({ length: tempThird}, () => "0")
        ),
      ];
    },
    updateValue: (state, action) => {
      const { matrixIndex, row, col, value } = action.payload;
      state.matrices[matrixIndex] = state.matrices[matrixIndex].map((r, ri) =>
        r.map((c, ci) => (ri === row && ci === col ? value : c))
      );
    },
    clear: (state, action) => {
      const { rows, cols, third } = action.payload;
      state.rows = rows;
      state.cols = cols;
      state.third = third;
      state.matrices = [
        Array.from({ length: rows }, () =>
          Array.from({ length: cols }, () => "0")
        ),
        Array.from({ length: cols}, () =>
          Array.from({ length: third}, () => "0")
        ),
      ];
    },
    randomize: (state, action) => {
      const { rows, cols,third } = action.payload;
      state.rows = rows;
      state.cols = cols;
      state.third = third;
      state.matrices = [
        Array.from({ length: rows }, () =>
          Array.from({ length: cols }, () =>
            String(Math.floor(Math.random() * 10))
          )
        ),
        Array.from({ length: cols}, () =>
          Array.from({ length: third}, () =>
            String(Math.floor(Math.random() * 10))
          )
        ),
      ];
    },
  },
});

export const { updateDimensions, updateValue, clear, randomize } =
  matricesSlice.actions;

export default matricesSlice.reducer;
