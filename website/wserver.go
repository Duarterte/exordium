package main

import (
	"fmt"
	"log"
	"net/http"
)

func home(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "html/index.html")
}

func docs(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "html/docs.html")
}

func register(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "html/register.html")
}

func main() {
	http.HandleFunc("/", home)
	http.HandleFunc("/docs", docs)
	http.HandleFunc("/register", register)
	http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("static"))))

	fmt.Println("Server is listening on port 8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
