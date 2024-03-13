import React, { useState,useEffect } from "react";
import axios from "axios";
import Button from "@mui/material/Button";
import TextField from "@mui/material/TextField";
import Grid from "@mui/material/Grid";
import AppBar from "@mui/material/AppBar";
import Toolbar from "@mui/material/Toolbar";
import Typography from "@mui/material/Typography";

function ConvolutionPage() {
  let postPort = 0xb00b;
  const [convolutionMatrix, setConvolutionMatrix] = useState([[
    ['0', '0', '0'],
    ['0', '1', '0'],
    ['0', '0', '0'],
  ]]);
  const [imageSrc, setImageSrc] = useState(null);
  const [pixelSrc, setPixelSrc] = useState([[]]);
  const [resultSrc, setResultSrc] = useState(null);
  const [resultPixels, setResultPixels] = useState(null);
  useEffect(() => {
    setResultSrc(null); // Reset resultSrc whenever convolutionMatrix changes
  }, [convolutionMatrix]);

  const handleMatrixChange = (event, layerIndex, rowIndex, colIndex) => {
    setResultSrc(null);
    const newValue = event.target.value;
    const updatedMatrix = [...convolutionMatrix];
    updatedMatrix[layerIndex][rowIndex][colIndex] = newValue;
    setConvolutionMatrix(updatedMatrix);
  };
  const handleMatrixSwap = (e) => {
    e.preventDefault();
    setImageSrc(resultSrc)
    setPixelSrc(resultPixels)
    setResultSrc(null)
  }

  const handleFileChange = (event) => {
    event.preventDefault();
    const file = event.target.files[0];
    setResultSrc(null);
    // Create a FileReader to read the file
    const reader = new FileReader();
    reader.onload = (e) => {
      const image = new Image();
      image.onload = () => {
        // Create a canvas element
        const canvas = document.createElement("canvas");
        const ctx = canvas.getContext("2d");

        // Set canvas dimensions to match the image dimensions
        canvas.width = 100;
        canvas.height = 100;

        // Draw the image onto the canvas
        ctx.drawImage(image, 0, 0);

        // Get the pixel data from the canvas
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        const pixelData = Array.from(imageData.data);

        // Process the pixel data as needed (e.g., store it in state)
        // Here, we're just logging the pixel data for demonstration purposes
        setPixelSrc(pixelData);
        setImageSrc(e.target.result);
      };
      image.src = e.target.result;
    };
    event.target.value = ''
    reader.readAsDataURL(file);
  };

  const handleSubmit = async (event) => {
    event.preventDefault();
    let pixelData = Array.from(pixelSrc).map(value => value.toString());
    const pixelMatrix = [];
    // Convert RGB pixel values to grayscale
const grayscaleData = [];
for (let i = 0; i < pixelData.length; i += 4) {
  const red = pixelData[i];
  const green = pixelData[i + 1];
  const blue = pixelData[i + 2];
  // Calculate grayscale value using luminosity method
  const grayscaleValue = Math.round(0.21 * red + 0.72 * green + 0.07 * blue);
  grayscaleData.push(grayscaleValue.toString());
}

// Organize grayscaleData into rows and columns of 100x100
for (let i = 0; i < grayscaleData.length; i += 100) {
  const row = grayscaleData.slice(i, i + 100);
  pixelMatrix.push(row);
}
    try {
        const response = await axios.post(
            "http://localhost:" + postPort + "/matrices",
            {convmatrices: [convolutionMatrix, pixelMatrix]},
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
      // Create a canvas element
const canvas = document.createElement('canvas');
canvas.width = 100;
canvas.height = 100;

// Get the 2D rendering context of the canvas
const ctx = canvas.getContext('2d');

// Create an ImageData object from the grayscale pixel data
const imageData = ctx.createImageData(100, 100);
for (let i = 0; i < 100; i++) {
  for (let j = 0; j < 100; j++) {
    const grayscaleValue = numbers[i*100 + j];
    const index = (i * 100 + j)*4;
    imageData.data[index] = grayscaleValue; // Red channel
    imageData.data[index + 1] = grayscaleValue; // Green channel
    imageData.data[index + 2] = grayscaleValue; // Blue channel
    imageData.data[index + 3] = 255; // Alpha channel
  }
}
setResultPixels(imageData.data)
// Put the ImageData object onto the canvas
ctx.putImageData(imageData, 0, 0);

 // Convert the canvas to a BMP blob
 canvas.toBlob((blob) => {
  setResultSrc(URL.createObjectURL(blob));
}, "image/bmp");

      // Process the response here and convert it into a .bmp file
    } catch (error) {
      console.error("Error processing data:", error);
      // Handle error
    }
  };

  return (
    <div className="ConvolutionPage">
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
                AiNalog: Image Convolution
              </Typography>
            </Grid>
            <Grid item>
    <Button
  variant="contained"
  color="primary"
  onClick={() =>
    setConvolutionMatrix([
      [['0', '0', '0'],
       ['0', '1', '0'],
       ['0', '0', '0']]
    ])
  }
>
  Identity
</Button>
            <Button
  variant="contained"
  color="primary"
  onClick={() =>
    setConvolutionMatrix([
      [['0', '-1', '0'],
       ['-1', '5', '-1'],
       ['0', '-1', '0']]
    ])
  }
>
  Sharpen
</Button>
<Button
  variant="contained"
  color="primary"
  onClick={() =>
    setConvolutionMatrix([
      [['1', '2', '1'],
       ['2', '4', '2'],
       ['1', '2', '1']]
    ])
  }
>
  Blur
</Button>
<Button
  variant="contained"
  color="primary"
  onClick={() =>
    setConvolutionMatrix([
      [['-1', '-1', '-1'],
       ['-1', '8', '-1'],
       ['-1', '-1', '-1']]
    ])
  }
>
  Edge Detection
</Button>

      </Grid>
          </Grid>
        </Toolbar>
      </AppBar>

      <form onSubmit={handleSubmit}>
        <Grid container spacing={2}>
          <Grid item xs={12}>
            <h2>Enter 3x3 Kernel:</h2>
          </Grid>
          {convolutionMatrix.map((layer, layerIndex) => (
  <React.Fragment key={layerIndex}>
    {layer.map((row, rowIndex) => (
      <Grid item xs={12} key={rowIndex}>
        {row.map((col, colIndex) => (
          <TextField
            key={colIndex}
            type="number"
            value={col}
            onChange={(e) =>
              handleMatrixChange(e, layerIndex, rowIndex, colIndex)
            }
          />
        ))}
      </Grid>
    ))}
  </React.Fragment>
))}

          <Grid item xs={12}>
            <h2>Upload .bmp File:</h2>
          </Grid>
          <Grid item xs={12}>
            <input
              type="file"
              accept=".bmp"
              onChange={handleFileChange}
              style={{ display: "none" }}
              id="bmp-file-input"
            />
            <label htmlFor="bmp-file-input">
              <Button
                variant="contained"
                component="span"
                color="primary"
                style={{ textTransform: "none" }}
              >
                Upload BMP File
              </Button>
            </label>
          </Grid>
          {imageSrc && (

            <Grid item xs={12}>
            <Grid>
            <h2>Image Uploaded:</h2>
            </Grid>
              <img
                src={imageSrc}
                alt="Uploaded BMP"
                style={{ width: "300px", height: "auto", maxHeight: "300px" }}
              />
            </Grid>
          )}
        {resultSrc&& (
            <Grid item xs={12}>
          <Grid >
            <h2>Image Post-Convolution:</h2>
            </Grid>
              <img
                src={resultSrc}
                alt="Result BMP"
                style={{ width: "300px", height: "auto", maxHeight:"300px" }}
              />

            </Grid>

          )}
          {resultSrc && 
          <Grid item xs={12}>
            <Button variant="contained" color="primary" onClick={handleMatrixSwap}>
              Use Result Matrix
            </Button>
          </Grid>}
          {!resultSrc && <Grid item xs={12}>
            <Button variant="contained" color="primary" type="submit">
              Submit
            </Button>
          </Grid>
        }
        </Grid>
      </form>
    </div>
  );
}

export default ConvolutionPage;
