package main

import (
	"fmt"
	"math"
	"sync"
)

// Função para verificar se um número é primo
func ehPrimo(n int) bool {
	if n <= 1 {
		return false
	}
	if n == 2 {
		return true
	}
	if n%2 == 0 {
		return false
	}
	for i := 3; i <= int(math.Sqrt(float64(n)))+1; i += 2 {
		if n%i == 0 {
			return false
		}
	}
	return true
}

// Goroutine para processar números e verificar se são primos
func worker(numeros <-chan int, resultados chan<- bool, wg *sync.WaitGroup) {
	defer wg.Done()
	for n := range numeros {
		resultados <- ehPrimo(n)
	}
}

func principal() {
	N := 100 // Valor N de 1 a N
	M := 5   // Número de goroutines
	numeros := make(chan int, N)
	resultados := make(chan bool, N)
	var wg sync.WaitGroup

	// Iniciar as goroutines
	for i := 0; i < M; i++ {
		wg.Add(1)
		go worker(numeros, resultados, &wg)
	}

	go func() {
		for i := 1; i <= N; i++ {
			numeros <- i
		}
		close(numeros)
	}()

	go func() {
		wg.Wait()
		close(resultados)
	}()

	// Contador de números primos encontrados
	totalPrimos := 0
	for primo := range resultados {
		if primo {
			totalPrimos++
		}
	}

	fmt.Printf("Total de números primos encontrados entre 1 e %d: %d\n", N, totalPrimos)
}
