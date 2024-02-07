import React, { useState } from 'react';
import axios from 'axios';


function App() {

  let postPort = 0xB00B;
  const [matrix, setMatrix] = useState([
    ['', '', '',''],
    ['', '', '',''],
    ['', '', '',''],
    ['', '', '','']
  ]);

  const handleChange = (row, col, value) => {
    const newMatrix = matrix.map((r, ri) => (
      r.map((c, ci) => (ri === row && ci === col ? value : c))
    ));
    setMatrix(newMatrix);
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    try {
      const response = await axios.post('http://localhost:' + postPort + '/matrix', { matrix: matrix });
      console.log(response.data);
      alert('Matrix sent successfully!');
    } catch (error) {
      console.error('Error sending matrix:', error);
      alert('Failed to send matrix.');
    }
  };

  return (
    <div className="App">
      <form onSubmit={handleSubmit}>
        <h2>Enter a 4x4 Matrix</h2>
        {matrix.map((row, rowIndex) => (
          <div key={rowIndex}>
            {row.map((col, colIndex) => (
              <input
                key={`${rowIndex}-${colIndex}`}
                type="text"
                value={col}
                onChange={(e) => handleChange(rowIndex, colIndex, e.target.value)}
                style={{ width: '50px', marginRight: '10px' }}
              />
            ))}
            <br />
          </div>
        ))}
        <button type="submit">Send Matrix</button>
      </form>
    </div>
  );
}

export default App;