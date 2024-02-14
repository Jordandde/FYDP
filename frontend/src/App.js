import React, { useState } from 'react';
import axios from 'axios';

function App() {

  let postPort = 0xB00B;
  const [matrices, setMatrices] = useState([
    [
      ['', '', '', ''],
      ['', '', '', ''],
      ['', '', '', ''],
      ['', '', '', '']
    ],
    [
      ['', '', '', ''],
      ['', '', '', ''],
      ['', '', '', ''],
      ['', '', '', '']
    ]
  ]);

  const handleChange = (matrixIndex, row, col, value) => {
    const newMatrices = matrices.map((matrix, i) => {
      if (i === matrixIndex) {
        return matrix.map((r, ri) => (
          r.map((c, ci) => (ri === row && ci === col ? value : c))
        ));
      }
      return matrix;
    });
    setMatrices(newMatrices);
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    try {
      const response = await axios.post('http://localhost:' + postPort + '/matrices', { matrices });
      console.log(response.data);
      alert('Matrices sent successfully!');
    } catch (error) {
      console.error('Error sending matrices:', error);
      alert('Failed to send matrices.');
    }
  };

  return (
    <div className="App">
      <form onSubmit={handleSubmit}>
        {matrices.map((matrix, matrixIndex) => (
          <div key={matrixIndex}>
            <h2>Enter a 4x4 Matrix ({matrixIndex + 1})</h2>
            {matrix.map((row, rowIndex) => (
              <div key={rowIndex}>
                {row.map((col, colIndex) => (
                  <input
                    key={`${matrixIndex}-${rowIndex}-${colIndex}`}
                    type="text"
                    value={col}
                    onChange={(e) => handleChange(matrixIndex, rowIndex, colIndex, e.target.value)}
                    style={{ width: '50px', marginRight: '10px' }}
                  />
                ))}
                <br />
              </div>
            ))}
          </div>
        ))}
        <button type="submit">Send Matrices</button>
      </form>
    </div>
  );
}

export default App;