package api

import (
	"encoding/json"
	"fmt"
	"net/http"
)

type Message struct {
	Matrix [][]string `json:"matrix"`
}

// Exported function to start listening on port 5050 and handle incoming JSON messages
func ListenAndServe() {
	http.HandleFunc("/matrix", postHandler) // Set the handler function for the root path
	fmt.Println("Server is listening on port 5050...")
	if err := http.ListenAndServe(":5050", nil); err != nil {
		fmt.Printf("Error starting server: %s\n", err)
	}
}

// Handles incoming HTTP POST requests at the root path
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
	// Decode the incoming JSON to the Message struct
	if err := json.NewDecoder(r.Body).Decode(&msg); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	// Display the message text
	fmt.Println("Received message: ")
	fmt.Println(msg.Matrix)
}
