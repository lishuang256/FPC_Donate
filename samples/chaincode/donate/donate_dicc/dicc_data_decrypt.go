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

type PlainData struct {
	Amount			string   `json:"amount"`
	MaterialList 	string   `json:"materialList"`
	CommentX   		string   `json:"commentX"`
	CommentY   		string   `json:"commentY"`
	RandomNum  		string   `json:"randomNumber"`
	RandomNumNew    string   `json:"randomNumNew"`
	Timestamp  		string   `json:"timestamp"`
}

func LoadPrivateKey(filename string) ([]byte, error) {
	data, err := os.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	block, _ := pem.Decode(data)
	if block == nil {
		return nil, fmt.Errorf("Cannot decode PEM block")
	}

	return block.Bytes, nil
}

func parseDecryptedData(decrypted []byte) (amount, materialList, commentX, commentY, randomNum, randomNumNew, timestamp string) {
	if len(decrypted) < 32*8 {
		fmt.Printf("Warning: Insufficient decryption data length, expected %d bytes, actual %d bytes\n", 32*6, len(decrypted))
		temp := make([]byte, 32*8)
		copy(temp, decrypted)
		decrypted = temp
	}

	amount = hex.EncodeToString(decrypted[32*0:32*1])
	materialList = hex.EncodeToString(decrypted[32*1:32*2])
	commentX = hex.EncodeToString(decrypted[32*2:32*3])
	commentY = hex.EncodeToString(decrypted[32*3:32*4])
	randomNum = hex.EncodeToString(decrypted[32*4:32*5])
	randomNumNew = hex.EncodeToString(decrypted[32*5:32*6])
	timestamp = hex.EncodeToString(decrypted[32*6:32*7])

	return amount, materialList, commentX, commentY, randomNum, randomNumNew, timestamp
}

func main() {
	ppriKey, err := loadPrivateKey("private.pem")
	if err != nil {
		fmt.Printf("Failed to load the private key: %v\n", err)
		return
	}
	fmt.Println("The private key has been loaded successfully")

	jsonData, err := os.ReadFile("dicc_data_encrypted.json")
	if err != nil {
		fmt.Printf("Failed to read dicc_data_encrypted.json: %v\n", err)
		return
	}

	var encryptedData []Data
	err = json.Unmarshal(jsonData, &encryptedData)
	if err != nil {
		fmt.Printf("Failed to parse JSON: %v\n", err)
		return
	}

	fmt.Printf("Find %d encrypted data\n", len(encryptedData))

	var plainDataList []PlainData
	for i, data := range encryptedData {
		fmt.Printf("The data in item %d is being decrypted...\n", i+1)

		cipherBytes, err := hex.DecodeString(data.CipherText)
		if err != nil {
			fmt.Printf("The ciphertext decoding of the %d data item failed: %v\n", i+1, err)
			continue
		}

		decryptedBytes := Decrypt(priKey, cipherBytes)
		amount, materialList, commentX, commentY, randomNum, randomNumNew, timestamp := parseDecryptedData(decryptedBytes)

		plainData := PlainData{
			OrderID:    	amount,
			MaterialList:  	materialList,
			CommentX:   	commentX
			CommentY:  		commentY
			RandomNum:  	randomNum,
			RandomNumNew:  	randomNumNew,
			Timestamp:  	timestamp,
		}

		plainDataList = append(plainDataList, plainData)
	}

	outputJSON, err := json.MarshalIndent(plainDataList, "", "  ")
	if err != nil {
		fmt.Printf("JSON serialization error: %v\n", err)
		return
	}

	err = os.WriteFile("decrypted_data.json", outputJSON, 0644)
	if err != nil {
		fmt.Printf("Data writing error: %v\n", err)
		return
	}

	fmt.Printf("Decryption completed! A total of %d pieces of data were processed, and the results have been saved in decrypted_data.json\n", len(plainDataList))
}