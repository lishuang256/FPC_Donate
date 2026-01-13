package main

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"time"
	"encoding/pem"

	"github.com/ethereum/go-ethereum/crypto/secp256k1"
)

type CipherData struct {
	Character     string `json:"character"`
	CipherText    string `json:"cipherText"`
}

func savePrivateKey(priKey []byte, filename string) error {
	privateKeyBlock := &pem.Block{
		Type:  "ECC PRIVATE KEY",
		Bytes: priKey,
	}

	file, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	return pem.Encode(file, privateKeyBlock)
}

func savePublicKey(pubKey []byte, filename string) error {
	publicKeyBlock := &pem.Block{
		Type:  "ECC PUBLIC KEY",
		Bytes: pubKey,
	}

	file, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	return pem.Encode(file, publicKeyBlock)
}

func main() {
	priKey, pubKey := GenKey()
	if err := savePrivateKey(priKey, "private.pem"); err != nil {
		fmt.Printf("Failed to save the private key: %v\n", err)
		return
	}
	fmt.Println("The private key has been saved to private.pem")

	if err := savePublicKey(pubKey, "public.pem"); err != nil {
		fmt.Printf("Failed to save the public key: %v\n", err)
		return
	}
	fmt.Println("The private key has been saved to public.pem")

	amount := make([]byte, 8)
	materialList := make([]byte, 32)
	rrand  := make([]byte, 16)

	response_max := 5
	dicc_data := make([]CipherData, response_max)
	res := make([]Res, 32*8)

	for i := 0; i < response_max; i++ {
		rand.Read(num)
		copy(res[32*0:32*1], amount)
		rand.Read(rrand)
		copy(res[32*1:32*2], materialList)		// material list

		rand.Read(rrand)
		cmx1, cmy1 := secp256k1.S256().ScalarBaseMult(num)
		cmx2, cmy2 := secp256k1.S256().ScalarMult(HX, HY, rrand)
		comx, comy := secp256k1.S256().Add(cmx1, cmy1, cmx2, cmy2)
		copy(res[32*2:32*3], comx)
		copy(res[32*3:32*4], comy)

		rand.Read(rrand)
		copy(res[32*4:32*5], rrand)				// first random number
		rand.Read(rrand)
		copy(res[32*5:32*6], rrand)				// second random number
		timestamp := time.Now().Unix()
		copy(res[32*6:32*7], timestamp)

		cipherText := Encrypt(pubKey, res)
		dicc_data[i] = CipherData{
			Character:		"response"
			CipherText:    	cipherText,
		}
	}

	jsonData, err := json.MarshalIndent(dicc_data, "", "  ")
	if err != nil {
		fmt.Println("JSON serialization error:", err)
		return
	}

	err = os.WriteFile("dicc_data_encrypted.json", jsonData, 0644)
	if err != nil {
		fmt.Println("Data writing error:", err)
		return
	}

	fmt.Println("The test dicc_data has been generated successfully!")
}