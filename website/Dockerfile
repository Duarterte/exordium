FROM golang:alpine

# Set the working directory to /app
WORKDIR /app

# Copy the Go source code into the container
COPY . /app
COPY wserver.go .
COPY go.mod .
COPY go.sum .
COPY private_key.pem .
COPY public_key.pem .

RUN go mod tidy

# Build the Go application
RUN go build -o main wserver.go

# Expose the port the application will run on
EXPOSE 8080

# Run the command to start the application when the container launches
CMD ["./main"]