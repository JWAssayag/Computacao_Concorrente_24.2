package main

import (
	"fmt"
)

func goroutine(canal chan string, resposta chan string) {
	msg := <-canal
	fmt.Println("Goroutine recebeu:", msg)

	resposta <- "Oi Main, bom dia, tudo bem?"

	msg = <-canal
	fmt.Println("Goroutine recebeu:", msg)

	resposta <- "Certo, entendido."
	fmt.Println("Goroutine: finalizando")
}

func main() {

	canal := make(chan string)
	resposta := make(chan string)

	go goroutine(canal, resposta)

	canal <- "Olá, Goroutine, bom dia!"
	msg := <-resposta
	fmt.Println("Main recebeu:", msg)
	canal <- "Tudo bem! Vou terminar tá?"
	msg = <-resposta
	fmt.Println("Main recebeu:", msg)
	fmt.Println("Main: finalizando")
}
