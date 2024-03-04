import React, { useState } from "react";
import axios from "axios";
import { useSelector, useDispatch } from "react-redux";
import {
  updateDimensions,
  updateValue,
  clear,
  randomize,
} from "./matricesSlice";
import TextField from "@mui/material/TextField";
import AppBar from "@mui/material/AppBar";
import Toolbar from "@mui/material/Toolbar";
import Button from "@mui/material/Button";
import Typography from "@mui/material/Typography";
import Grid from "@mui/material/Grid";

function App() {
  let postPort = 0xb00b;
  const [calcFinished, setCalcFinished] = useState(false);
  const matrices = useSelector((state) => state.matrices);
  const rows = useSelector((state) => state.rows);
  const cols = useSelector((state) => state.cols);
  const [tempRows, setTempRows] = useState(rows);
  const [tempCols, setTempCols] = useState(cols);
  const [result, setResult] = useState([
    Array.from({ length: rows }, () => Array.from({ length: cols }, () => "0")),
  ]);
  let total = rows * cols < 144;

  const calibrationMatrix = [
  [
    ['1','1','1','1'],
    ['1','1','1','1'],
    ['1','1','1','1'],
    ['1','1','1','1'],
  ],
  [
    ['1','1','1','1'],
    ['1','1','1','1'],
    ['1','1','1','1'],
    ['1','1','1','1'],
  ]
  ]
  const dispatch = useDispatch();

  const handleChange = (matrixIndex, row, col, value) => {
    setCalcFinished(false);
    dispatch(updateValue({ matrixIndex, row, col, value }));
  };
  const handleCalibrate = async(e) => {
    e.preventDefault();
    try {
      const response = await axios.post(
        "http://localhost:" + postPort + "/matrices",
        { calibrationMatrix},
        {
          headers: {
            'Content-Type': 'text/plain'
          },
          maxContentLength: Infinity,
          maxBodyLength: Infinity,
        }
      );
      const responseData = response.data;
      const numbers = responseData.split(" ").map(Number);
      console.log(numbers)
      alert("Calibration complete")
    } catch (error) {
      console.error("Error sending calibration matrices:", error);
      alert("Failed to send calibration matrices.");
    }
  }
  const handleSubmit = async (e) => {
    e.preventDefault();
    try {
      const response = await axios.post(
        "http://localhost:" + postPort + "/matrices",
        { matrices },
        {
          headers: {
            'Content-Type': 'text/plain'
          },
          maxContentLength: Infinity,
          maxBodyLength: Infinity,
        }
      );
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
    total = rows * cols < 144;
  };

  const handleClear = (e) => {
    dispatch(clear({ rows, cols }));
  };

  const handleRandomize = (e) => {
    dispatch(randomize({ rows, cols }));
  };

  return (
    <div className="App">
      <AppBar position="static">
        <Toolbar>
          <Grid
            direction="row"
            alignItems="center"
            justifyContent="flex-end"
            container
            spacing={4}
          >
            <Grid item xs>
              <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
                AiNalog
              </Typography>
            </Grid>
            <Grid item xs>
              <h3>Rows</h3>
            </Grid>
            <Grid item xs>
              <TextField
                type="number"
                value={tempRows}
                onChange={handleRowChange}
                min="1"
                size="small"
              />
            </Grid>
            <Grid item xs>
              <h3>Columns</h3>
            </Grid>
            <Grid item xs>
              <TextField
                type="number"
                value={tempCols}
                onChange={handleColChange}
                min="1"
                size="small"
              />
            </Grid>
            <Grid item xs>
              <Button
                style={{ textTransform: "none" }}
                color="inherit"
                type="submit"
                onClick={handleDimChange}
              >
                <h3>Change</h3>
              </Button>
            </Grid>
            <Grid item xs>
              <Button
                color="inherit"
                style={{ textTransform: "none" }}
                onClick={handleSubmit}
              >
                <h3>Send</h3>
              </Button>
              
            </Grid>
            
            <Grid item xs>
              <Button
                color="inherit"
                style={{ textTransform: "none" }}
                onClick={handleClear}
              >
                <h3>Clear</h3>
              </Button>
            </Grid>
            <Grid item xs>
              <Button
                color="inherit"
                style={{ textTransform: "none" }}
                onClick={handleRandomize}
              >
                <h3>Random</h3>
              </Button>
            </Grid>
            <Grid item xs>
              <Button
                color="inherit"
                style={{textTransform: "none"}}
                onClick={handleCalibrate}
                >
                  <h3>Calibrate</h3>
              </Button>
            </Grid>
          </Grid>
        </Toolbar>
      </AppBar>
      <div style={{ padding: "1rem" }}>
        {matrices?.map((matrix, matrixIndex) => (
          <div key={matrixIndex}>
            <h2>
              Enter a {rows}x{cols} Matrix ({matrixIndex + 1})
            </h2>
            {matrix.map((row, rowIndex) => (
              <div key={rowIndex}>
                <Grid container spacing={1}>
                  {row.map((col, colIndex) => (
                    <Grid item xs>
                      {total ? (
                        <TextField
                          key={`${matrixIndex}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          onChange={(e) =>
                            handleChange(
                              matrixIndex,
                              rowIndex,
                              colIndex,
                              e.target.value
                            )
                          }
                          style={{
                            margin: "10px",
                          }}
                        />
                      ) : (
                        <input
                          key={`${matrixIndex}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          onChange={(e) =>
                            handleChange(
                              matrixIndex,
                              rowIndex,
                              colIndex,
                              e.target.value
                            )
                          }
                          style={{
                            width: "30px",
                          }}
                        />
                      )}
                    </Grid>
                  ))}
                  <br />
                </Grid>
              </div>
            ))}
          </div>
        ))}
        {calcFinished &&
          result.map((matrix, matrixIndex) => (
            <div key={matrixIndex}>
              <h2>Result Matrix </h2>
              {matrix.map((row, rowIndex) => (
                <Grid container spacing={1}>
                  {row.map((col, colIndex) => (
                    <Grid item xs>
                      {total ? (
                        <TextField
                          key={`${matrixIndex}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          disabled
                          style={{
                            margin: "10px",
                          }}
                        />
                      ) : (
                        <input
                          key={`${matrixIndex}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          disabled
                          style={{
                            width: "30px",
                          }}
                        />
                      )}
                    </Grid>
                  ))}
                </Grid>
              ))}
            </div>
          ))}
      </div>
    </div>
  );
}

export default App;
