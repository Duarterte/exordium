package main

import (
	"crypto/rsa"
	"crypto/x509"
	"database/sql"
	"encoding/pem"
	"fmt"
	"log"
	"net/http"
	"os"
	"time"

	_ "github.com/go-sql-driver/mysql"
	"github.com/golang-jwt/jwt/v5"
	"github.com/google/uuid"
)

const dsn = "root:joda@tcp(192.168.0.5:3306)/exordium"

type Claims struct {
	Subject string `json:"sub"`
	jwt.RegisteredClaims
}

type User struct {
	ID       uuid.UUID
	Username string
	Email    string
	Password string
}

func getPrivateKey() *rsa.PrivateKey {
	privateKeyPEM, err := os.ReadFile("private_key.pem")
	if err != nil {
		fmt.Println("Error reading private key file:", err)
		os.Exit(1)
	}
	privateKeyBlock, _ := pem.Decode(privateKeyPEM)
	if privateKeyBlock == nil {
		fmt.Println("Error decoding private key")
		os.Exit(1)
	}
	privateKey, err := x509.ParsePKCS8PrivateKey(privateKeyBlock.Bytes)
	if err != nil {
		fmt.Println("Error parsing private key:", err)
		os.Exit(1)
	}
	return privateKey.(*rsa.PrivateKey)
}

func getPublicKey() *rsa.PublicKey {
	publicKeyPEM, err := os.ReadFile("public_key.pem")
	if err != nil {
		fmt.Println("Error reading public key file:", err)
		os.Exit(1)
	}
	publicKeyBlock, _ := pem.Decode(publicKeyPEM)
	if publicKeyBlock == nil {
		fmt.Println("Error decoding public key")
		os.Exit(1)
	}
	publicKey, err := x509.ParsePKIXPublicKey(publicKeyBlock.Bytes)
	if err != nil {
		fmt.Println("Error parsing public key:", err)
		os.Exit(1)
	}
	return publicKey.(*rsa.PublicKey)
}

func generateToken(userUUID uuid.UUID) *jwt.Token {
	claims := Claims{
		Subject: "example_user",
		RegisteredClaims: jwt.RegisteredClaims{
			ExpiresAt: jwt.NewNumericDate(time.Now().Add(time.Hour)),
			IssuedAt:  jwt.NewNumericDate(time.Now()),
			NotBefore: jwt.NewNumericDate(time.Now()),
			Issuer:    "Exordium",
			Subject:   userUUID.String(),
		},
	}
	token := jwt.NewWithClaims(jwt.SigningMethodRS256, claims)
	return token
}

func getTokenString(token *jwt.Token) string {
	tokenString, err := token.SignedString(getPrivateKey())
	if err != nil {
		fmt.Println("Error signing token:", err)
		os.Exit(1)
	}
	return tokenString
}

func verifyToken(tokenString string) (*Claims, bool) {
	publicKey := getPublicKey()
	claims := &Claims{}
	token, err := jwt.ParseWithClaims(tokenString, claims, func(t *jwt.Token) (interface{}, error) {
		if _, ok := t.Method.(*jwt.SigningMethodRSA); !ok {
			return nil, fmt.Errorf("unexpected signing method: %v", t.Header["alg"])
		}
		return publicKey, nil
	})

	if err != nil {
		fmt.Println("Error parsing token:", err)
		return nil, false
	}

	if token.Valid {
		return claims, true
	}
	return nil, false
}

func checkConn() {

	db, err := sql.Open("mysql", dsn)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	// Ping the database to verify the connection
	err = db.Ping()
	if err != nil {
		log.Fatal(err)
	}

	fmt.Println("Connected to MariaDB!")
}

func createUser(username, password, email string) <-chan bool {
	ch := make(chan bool)
	go func() {
		db, err := sql.Open("mysql", dsn)
		if err != nil {
			log.Fatal(err)
			ch <- false
			return
		}
		defer db.Close()

		// Insert the user into the database
		_, err = db.Exec("INSERT INTO users (username, password, email) VALUES (?, ?, ?)", username, password, email)
		if err != nil {
			log.Fatal(err)
			ch <- false
		} else {
			ch <- true
		}
	}()
	return ch
}

func getUserUUID(username string) uuid.UUID {
	db, err := sql.Open("mysql", dsn)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	var userUUID uuid.UUID
	err = db.QueryRow("SELECT id FROM users WHERE username = ?", username).Scan(&userUUID)
	if err != nil {
		log.Fatal(err)
	}
	return userUUID
}

func main() {
	http.HandleFunc("/", home)
	http.HandleFunc("/docs", docs)
	http.HandleFunc("/register", register)
	http.HandleFunc("/downloads", downloads)
	http.Handle("/user", Protected(http.HandlerFunc(userPage)))
	http.Handle("/success", Protected(http.HandlerFunc(success)))
	http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("static"))))
	fmt.Println("Server is listening on port 8080")
	checkConn()
	log.Fatal(http.ListenAndServe(":8080", nil))
}

// Protected Middleware
func Protected(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		// Get the Authorization cookie
		fmt.Println("Protected reached")
		cookie, err := r.Cookie("Authorization")
		if err != nil {
			if err == http.ErrNoCookie {
				// Cookie not found, redirect to login
				http.Redirect(w, r, "/login", http.StatusSeeOther)
				return
			}
			// Other error accessing cookie
			http.Error(w, "Error accessing cookie", http.StatusInternalServerError)
			return
		}

		tokenString := cookie.Value

		_, ok := verifyToken(tokenString)
		if !ok {

			http.Redirect(w, r, "/login", http.StatusSeeOther)
			return
		}

		ctx := r.Context()

		next.ServeHTTP(w, r.WithContext(ctx))
	})
}

func userPage(w http.ResponseWriter, r *http.Request) {
	fmt.Println("user page")
	http.ServeFile(w, r, "html/user.html")
}

func home(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "html/index.html")
}

func docs(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "html/docs.html")
}

func downloads(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "html/downloads.html")
}

func register(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" {
		http.ServeFile(w, r, "html/register.html")
	}
	if r.Method == "POST" {
		//get form data
		username := r.FormValue("username")
		password := r.FormValue("password")
		email := r.FormValue("email")
		fmt.Println(username, password, email)

		ch := createUser(username, password, email)
		result := <-ch
		if result {
			userUUID := getUserUUID(username)
			token := generateToken(userUUID)
			signedToken := getTokenString(token)
			cookie := &http.Cookie{
				Name:     "Authorization",
				Value:    signedToken,
				Path:     "/",
				Expires:  time.Now().Add(time.Hour),
				HttpOnly: true,
				Secure:   true,
			}
			http.SetCookie(w, cookie)
			http.Redirect(w, r, "/success", http.StatusSeeOther)
			fmt.Println("User created successfully")
		} else {
			fmt.Println("User creation failed")
			http.Redirect(w, r, "/", http.StatusSeeOther)
		}
	}
}

func success(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "html/success.html")
}
