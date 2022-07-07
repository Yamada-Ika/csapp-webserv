package main

import (
	"fmt"
	"os"
	"strconv"
	"strings"
)

// getNumbers convet to two integer
// '123&134' -> 123, 134
func getNumbers(s string) (n1, n2 int) {
	sepAt := strings.Index(s, "&")
	if sepAt < 0 {
		return
	}
	n1Str := s[:sepAt]
	n2Str := s[sepAt+1:]
	n1, _ = strconv.Atoi(n1Str)
	n2, _ = strconv.Atoi(n2Str)
	return
}

func main() {
	query := os.Getenv("QUERY_STRING")

	var n1 int
	var n2 int
	// Does QUERY_STRING exist ?
	if query != "" {
		n1, n2 = getNumbers(query)
	}

	// make response body
	body := fmt.Sprintf("<p>Welcome to add.com: The Internet addition portal.</p>\r\n")
	body += fmt.Sprintf("<p>The answer is: %d + %d = %d</p>\r\n", n1, n2, n1+n2)
	body += fmt.Sprintf("<p>Thanks for visiting!</p>\r\n")

	// make resonse
	resp := fmt.Sprintf("Connection: close\r\n")
	resp += fmt.Sprintf("Content-length: %d\r\n", len(body))
	resp += fmt.Sprintf("Content-type: text/html\r\n")
	resp += fmt.Sprintf("\r\n")
	resp += fmt.Sprintf("%s", body)

	fmt.Println(resp)
}
