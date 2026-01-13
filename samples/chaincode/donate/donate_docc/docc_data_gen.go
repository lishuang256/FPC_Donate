package main

import (
	"crypto/rand"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"time"
	"math/big"

	"github.com/ethereum/go-ethereum/crypto/secp256k1"
)

type Data struct {
	ResponseHash		string `json:"responseHash"`
	ResponseEnc			string `json:"responseEnc"`
	CommentX        	string `json:"commentX"`
	CommentY    		string `json:"commentY"`
	CommentXNew        	string `json:"commentXNew"`
	CommentYNew    		string `json:"commentYNew"`
	Deposite 			string `json:"deposite"`
	FeedbackHash		string `json:"feedbackHash"`
	FeedbackEnc			string `json:"feedbackEnc"`
	Timestamp       	string `json:"timestamp"`
}

func genRandom(length int) string {
	bytes := make([]byte, length)
	rand.Read(bytes)
	return hex.EncodeToString(bytes)
}

func Hash(input string) string {
	hash := sha256.New()
	hash.Write([]byte(input))
	
	hashBytes := hash.Sum(nil)
	return hex.EncodeToString(hashBytes)
}

func main() {
	amount_DR := genRandom(8)			// donate amount
	materialList := genRandom(32)
	randomNumber := genRandom(16)
	randomNumberNew := genRandom(16)
	deposite := genRandom(8)
	timestamp := time.Now().Format(time.RFC3339)

	priKey, pubKey := GenKey()
	response := amount_DR + materialList + randomNumber + randomNumberNew + timestamp
	responseEnc := Encrypt(pubKey, []byte(response))
	feedback := materialList + responseEnc + timestamp
	feedbackEnc := Encrypt(pubKey, []byte(feedback))
	docc_data := Data{
		ResponseHash:       Hash(response),
		ResponseEnc:		responseEnc,
		CommentX:           "",
		CommentY:       	"",
		CommentXNew:        "",
		CommentYNew:       	"",
		Deposite:           deposite,
		FeedbackHash:       Hash(feedback),
		FeedbackEnc:		feedbackEnc,
		Timestamp:       	timestamp,
	}

	HX, _ := new(big.Int).SetString("C6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5", 16)
	HY, _ := new(big.Int).SetString("1AE168FEA63DC339A3C58419466EFAEEF7F632653266D0E1236431A950CFE52A", 16)

	cmx1, cmy1 := secp256k1.S256().ScalarBaseMult([]byte(amount_DR))
	cmx2, cmy2 := secp256k1.S256().ScalarMult(HX, HY, []byte(randomNumber))
	cmx, cmy := secp256k1.S256().Add(cmx1, cmy1, cmx2, cmy2)
	docc_data.CommentX = cmx.String()
	docc_data.CommentY = cmy.String()

	cmx2, cmy2 = secp256k1.S256().ScalarMult(HX, HY, []byte(randomNumberNew))
	cmx, cmy = secp256k1.S256().Add(cmx1, cmy1, cmx2, cmy2)
	docc_data.CommentXNew = cmx.String()
	docc_data.CommentYNew = cmy.String()

	data_set := []Data{docc_data}
	jsonData, err := json.MarshalIndent(data_set, "", "  ")
	if err != nil {
		fmt.Println("JSON serialization error:", err)
		return
	}

	err = os.WriteFile("docc_data.json", jsonData, 0644)
	if err != nil {
		fmt.Println("Data writing error:", err)
		return
	}

	fmt.Println("The test docc_data has been generated successfully!")
}