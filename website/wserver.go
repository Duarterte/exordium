package main

import (
	"fmt"
	"log"
	"net/http"
)

func main() {
	// Serve HTML files from the html folder
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path == "/" {
			http.ServeFile(w, r, "html/index.html")
		} else {
			http.ServeFile(w, r, "html"+r.URL.Path)
		}
	})

	// Serve static files from the static folder
	http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("static"))))

	fmt.Println("Server is listening on port 8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
