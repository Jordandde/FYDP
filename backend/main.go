package main

import (
	"fmt"

	"backend/api"
)

func main() {
	fmt.Println("Start up")
	api.ListenAndServe()
}
