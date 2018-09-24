package main

import (
	"net/http"
)

func main() {
	http.HandleFunc("/test", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("SOME TEST PAGE!"))
	})

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		if r.Method == http.MethodPost {
			w.Write([]byte("<h1>its post bro!</h1>"))
			return
		}
		w.Write([]byte("Hello, world!"))
	})

	http.ListenAndServe(":1337", nil)
}
