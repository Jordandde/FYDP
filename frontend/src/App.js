import React, { useState,useEffect } from "react";
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
import Drawer from "@mui/material/Drawer";

function App() {
  let postPort = 0xb00b;
  const [calcFinished, setCalcFinished] = useState(false);
  const matrices = useSelector((state) => state.matrices);
  const rows = useSelector((state) => state.rows);
  const cols = useSelector((state) => state.cols);
  const third = useSelector((state) => state.third);
  const [tempRows, setTempRows] = useState(rows);
  const [tempCols, setTempCols] = useState(cols);
  const [tempThird, setTempThird] = useState(third);
  const [result, setResult] = useState([
    Array.from({ length: rows }, () => Array.from({ length: third}, () => "0")),
  ]);
  let total = rows * third< 144;
  const [open, setOpen] = useState(false);
  const [spamMatrix, setSpamMatrix] = useState([ ]);
  const [spamInput, setSpamInput] = useState([]);
  const [spam, setSpam] = useState(false);

  useEffect(() => {
    if(spam) {
      handleSubmit(new Event("click"))
    }
  }, [matrices,spam]);

  const toggleDrawer = (newOpen) => () => {
    setOpen(newOpen);
  };

  const calibrationMatrix = [
    [['1','2','3','4'],
  ['1','2','3','4'],
  ['1','2','3','4'],
  ['1','2','3','4'],
  ],
  [['1','2','3','4'],
  ['1','2','3','4'],
  ['1','2','3','4'],
  ['1','2','3','4'],
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
            const index = ri * third + ci;
            return numbers[index];
          })
        );
      });
      setResult(newResult);
      if(spam) {
        let temp = spamMatrix;
        temp.push(newResult[0]);
        setSpamMatrix(temp);
        let temp2 = spamInput
        temp2.push(matrices[0])
        temp2.push(matrices[1])
        setSpamInput(temp2)
      }
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
  const handleThirdChange = (e) => {
    const value = parseInt(e.target.value);
    setTempThird(value);
  };
  const handleDimChange = (e) => {
    e.preventDefault();
    if (tempRows % 2 !== 0 || tempCols % 2 !== 0 || tempThird %2 !== 0) {
      alert("Invalid dimensions, must be even and greater than 0");
      return;
    }
    dispatch(updateDimensions({ tempRows, tempCols,tempThird }));
    setResult([
      Array.from({ length: rows}, () =>
        Array.from({ length: tempThird}, () => "0")
      ),
    ]);
    total = rows * third< 144;
  };

  const handleClear = (e) => {
    dispatch(clear({ rows, cols,third }));
  };

  const handleRandomize = async(e) => {
    dispatch(randomize({ rows, cols,third }));
  };
  const handleSpam = async (e) => {
    e.preventDefault();
    setSpam(true)
    console.log("spam activated")
    setSpamMatrix([])
    setSpamInput([])
    for(var i = 0; i < 10; i++) {
      await handleRandomize(e);
    }
    setSpam(false)
    setOpen(true)
  }

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
              <h3>X</h3>
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
              <h3>Y</h3>
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
              <h3>Z</h3>
            </Grid>
            <Grid item xs>
              <TextField
                type="number"
                value={tempThird}
                onChange={handleThirdChange}
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
            <Grid item xs>
              <Button
                color="inherit"
                style={{textTransform: "none"}}
                onClick={handleSpam}
                >
                  <h3>Spam</h3>
              </Button>
            </Grid>
          </Grid>
        </Toolbar>
      </AppBar>
      <Drawer
      anchor={"bottom"}
      open={open}
      onClose={toggleDrawer(false)}
    >
      <div style={{ padding: "1rem" }}>
        <button
        onClick={toggleDrawer(false)}
        >exit</button>
    {spamMatrix.map((matrix, matrixIndex) => (
            <div key={matrixIndex+2}>
            <h2>Input {2*matrixIndex + 1}</h2>
            {spamInput[2*matrixIndex].map((row, rowIndex) => (
                <Grid container spacing={1}>
                  {row.map((col, colIndex) => (
                    <Grid item xs>
                      {total ? (
                        <TextField
                          key={`${matrixIndex+ 2}-${rowIndex}-${colIndex}`}
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
            <h2>Input {2*matrixIndex+2}</h2>
              {spamInput[2*matrixIndex+1].map((row, rowIndex) => (
                <Grid container spacing={1}>
                  {row.map((col, colIndex) => (
                    <Grid item xs>
                      {total ? (
                        <TextField
                          key={`${matrixIndex+ 2}-${rowIndex}-${colIndex}`}
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
              <h2>Result Matrix {matrixIndex + 1}</h2>
              {matrix.map((row, rowIndex) => (
                <Grid container spacing={1}>
                  {row.map((col, colIndex) => (
                    <Grid item xs>
                      {total ? (
                        <TextField
                          key={`${matrixIndex+ 2}-${rowIndex}-${colIndex}`}
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
          ))}</div>
    </Drawer>
      <div style={{ padding: "1rem" }}>
          <div key={0}>
            <h2>
              Enter a {rows}x{cols} Matrix 0
            </h2>
            {matrices[0]?.map((row, rowIndex) => (
              <div key={rowIndex}>
                <Grid container spacing={1}>
                  {row.map((col, colIndex) => (
                    <Grid item xs>
                      {total ? (
                        <TextField
                          key={`${0}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          onChange={(e) =>
                            handleChange(
                              0,
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
                          key={`${0}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          onChange={(e) =>
                            handleChange(
                              0,
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
            <div key={1}>
            <h2>
              Enter a {cols}x{third} Matrix 1
            </h2>
            {matrices[1]?.map((row, rowIndex) => (
              <div key={rowIndex}>
                <Grid container spacing={1}>
                  {row.map((col, colIndex) => (
                    <Grid item xs>
                      {total ? (
                        <TextField
                          key={`${1}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          onChange={(e) =>
                            handleChange(
                              1,
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
                          key={`${1}-${rowIndex}-${colIndex}`}
                          type="number"
                          value={col}
                          onChange={(e) =>
                            handleChange(
                              1,
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
        {calcFinished &&
          result.map((matrix, matrixIndex) => (
            <div key={matrixIndex}>
              <h2>{rows}x{third} Result Matrix </h2>
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
