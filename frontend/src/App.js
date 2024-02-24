import React, { useState } from "react";
import axios from "axios";
import { useSelector, useDispatch } from "react-redux";
import { updateDimensions, updateValue } from "./matricesSlice";

function App() {
  let postPort = 0xb00b;
  const [submitted, setSubmitted] = useState(false);
  const [calcFinished, setCalcFinished] = useState(false);
  const [tempRows, setTempRows] = useState(4);
  const [tempCols, setTempCols] = useState(4);
  const matrices = useSelector((state) => state.matrices);
  const rows = useSelector((state) => state.rows);
  const cols = useSelector((state) => state.cols);

  const [result, setResult] = useState([
    Array.from({ length: 4 }, () => Array.from({ length: 4 }, () => "0")),
  ]);

  const dispatch = useDispatch();

  const handleChange = (matrixIndex, row, col, value) => {
    setSubmitted(false);
    setCalcFinished(false);
    dispatch(updateValue({ matrixIndex, row, col, value }));
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    try {
      const response = await axios.post(
        "http://localhost:" + postPort + "/matrices",
        { matrices }
      );
      setSubmitted(true);
      const responseData = response.data;
      const numbers = responseData.split(" ").map(Number);

      const newResult = result.map((matrix) => {
        return matrix.map((row, ri) =>
          row.map((col, ci) => {
            const index = ri * cols + ci;
            return numbers[index];
          })
        );
      });
      setResult(newResult);
      setCalcFinished(true);
      console.log(result);
    } catch (error) {
      console.error("Error sending matrices:", error);
      alert("Failed to send matrices.");
    }
  };

  const handleRowChange = (e) => {
    const value = parseInt(e.target.value);
    setTempRows(value);
  };

  const handleColChange = (e) => {
    const value = parseInt(e.target.value);
    setTempCols(value);
  };
  const handleDimChange = (e) => {
    e.preventDefault();
    if (tempRows % 2 !== 0 || tempCols % 2 !== 0) {
      alert("Invalid dimensions, must be even and greater than 0");
      return;
    }
    dispatch(updateDimensions({ tempRows, tempCols }));
    setResult([
      Array.from({ length: tempRows }, () =>
        Array.from({ length: tempCols }, () => "0")
      ),
    ]);
  };
  return (
    <div className="App">
      <form onSubmit={handleDimChange}>
        <label>
          Rows:
          <input
            type="number"
            value={tempRows}
            onChange={handleRowChange}
            min="1"
          />
        </label>
        <label>
          Columns:
          <input
            type="number"
            value={tempCols}
            onChange={handleColChange}
            min="1"
          />
        </label>
        <button type="submit">Change matrices</button>
      </form>
      <form onSubmit={handleSubmit}>
        {matrices?.map((matrix, matrixIndex) => (
          <div key={matrixIndex}>
            <h2>
              Enter a {rows}x{cols} Matrix ({matrixIndex + 1})
            </h2>
            {matrix.map((row, rowIndex) => (
              <div key={rowIndex}>
                {row.map((col, colIndex) => (
                  <input
                    key={`${matrixIndex}-${rowIndex}-${colIndex}`}
                    type="text"
                    value={col}
                    onChange={(e) =>
                      handleChange(
                        matrixIndex,
                        rowIndex,
                        colIndex,
                        e.target.value
                      )
                    }
                    style={{ width: "50px", marginRight: "10px" }}
                  />
                ))}
                <br />
              </div>
            ))}
          </div>
        ))}
        <button type="submit">Send Matrices</button>
        {submitted && <h3> Matrices Sent!</h3>}
      </form>
      {calcFinished &&
        result.map((matrix, matrixIndex) => (
          <div key={matrixIndex}>
            <h2>Result Matrix </h2>
            <table>
              <tbody>
                {matrix.map((row, rowIndex) => (
                  <tr key={rowIndex}>
                    {row.map((col, colIndex) => (
                      <td key={`${matrixIndex}-${rowIndex}-${colIndex}`}>
                        {col}
                      </td>
                    ))}
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        ))}
    </div>
  );
}

export default App;
