package api

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
)

var POSTPORT int = 0xB00B

type Message struct {
	Matrices [][][4]string `json:"matrix"`
}

// Listen for any calls on POSTPORT and direct them to the correct endpoint
func ListenAndServe() {

	http.HandleFunc("/matrices", postHandler) // Set the handler function for the "/matrices" path

	fmt.Printf("Server is listening on port %d...\n", POSTPORT)

	if err := http.ListenAndServe(fmt.Sprintf(":%d", POSTPORT), nil); err != nil {
		fmt.Printf("Error starting server: %s\n", err)
	}
}

// Handles incoming HTTP POST requests at the "/matrices" path
func postHandler(w http.ResponseWriter, r *http.Request) {
	// Enable CORS for local testing
	header := w.Header()
	header.Add("Access-Control-Allow-Origin", "*")
	header.Add("Access-Control-Allow-Methods", "DELETE, POST, GET, OPTIONS")
	header.Add("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With")
	if r.Method == "OPTIONS" {
		w.WriteHeader(http.StatusOK)
		return
	} else if r.Method != "POST" {
		http.Error(w, "Method is not supported.", http.StatusMethodNotAllowed)
		return
	}

	var msg Message
	// Read the request body
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		fmt.Println("Error reading request body:", err)
		http.Error(w, "Error reading request body", http.StatusInternalServerError)
		return
	}

	// Print the received JSON data for debugging
	fmt.Println("Received JSON data:", string(body))

	// Decode the incoming JSON to the Matrices struct
	if err := json.Unmarshal(body, &msg); err != nil {
		fmt.Println("Error decoding JSON:", err)
		http.Error(w, "Error decoding JSON", http.StatusBadRequest)
		return
	}

	// Display the received matrices
	fmt.Println("Received matrices:")
	for i, matrix := range msg.Matrices {
		fmt.Printf("Matrix %d:\n", i+1)
		for _, row := range matrix {
			fmt.Println(row)
		}
		fmt.Println()
		fmt.Println()
		fmt.Println()
	}

	// Respond with success message
	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Matrices received successfully!"))
}
