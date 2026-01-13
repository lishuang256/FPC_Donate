package main

import (
    "fmt"
    "os"
    "regexp"
    "math/big"
    "strings"
	"crypto/sha256"

	"github.com/ethereum/go-ethereum/crypto/secp256k1"
)


func main() {
    if len(os.Args) < 2 {
        fmt.Println("ERROR: No input provided")
        os.Exit(1)
    }
    
    input := os.Args[1]
    result := processResponse(input)
    fmt.Println(result)
}

func processResponse(response string) string {
    data, err := extractData(response)
    if err != nil {
        return fmt.Sprintf("ERROR: %v", err)
    }

    // fmt.Fprintf(os.Stderr, "Extracted data (hexadecimal):\n")
    // fmt.Fprintf(os.Stderr, "ACominX: 0x%x\n", data.ACominX)
    // fmt.Fprintf(os.Stderr, "ACominY: 0x%x\n", data.ACominY)
    // fmt.Fprintf(os.Stderr, "AComoutX: 0x%x\n", data.AComoutX)
    // fmt.Fprintf(os.Stderr, "AComoutY: 0x%x\n", data.AComoutY)
    // fmt.Fprintf(os.Stderr, "Tx1: 0x%x, Ty1: 0x%x\n", data.Tx1, data.Ty1)
    // fmt.Fprintf(os.Stderr, "Tx2: 0x%x, Ty2: 0x%x\n", data.Tx2, data.Ty2)
    // fmt.Fprintf(os.Stderr, "z: 0x%x\n", data.Z)
    // fmt.Fprintf(os.Stderr, "hzrx1: 0x%x, hzry1: 0x%x\n", data.Hzrx1, data.Hzry1)
    // fmt.Fprintf(os.Stderr, "hzrx2: 0x%x, hzry2: 0x%x\n", data.Hzrx2, data.Hzry2)
    
    validationResult := varifyNIZK(data)
    
    if validationResult {
        return "Audit passed!"
    }
    return "VALIDATION_FAILED"
}

type AuditData struct {
    ACominX  *big.Int
    ACominY  *big.Int
    AComoutX *big.Int
    AComoutY *big.Int
    Tx1      *big.Int
    Ty1      *big.Int
    Tx2      *big.Int
    Ty2      *big.Int
    Z        *big.Int
    Zr1      *big.Int
    Zr2      *big.Int
}

func extractData(response string) (*AuditData, error) {
    re := regexp.MustCompile(`ACominX:\s*([0-9a-fA-F]+)\s*ACominY:\s*([0-9a-fA-F]+)\s*AComoutX:\s*([0-9a-fA-F]+)\s*AComoutY:\s*([0-9a-fA-F]+)\s*\|\s*Tx1:\s*([0-9a-fA-F]+)\s*Ty1:\s*([0-9a-fA-F]+)\s*Tx2:\s*([0-9a-fA-F]+)\s*Ty2:\s*([0-9a-fA-F]+)\s*\|\s*z:\s*([0-9a-fA-F]+)\s*hzrx1:\s*([0-9a-fA-F]+)\s*hzry1:\s*([0-9a-fA-F]+)\s*hzrx2:\s*([0-9a-fA-F]+)\s*hzry2:\s*([0-9a-fA-F]+)`)
    
    matches := re.FindStringSubmatch(response)
    if matches == nil || len(matches) != 14 {
        return nil, fmt.Errorf("failed to parse response. Expected 13 hex numbers, got %d matches in: %s", len(matches)-1, response)
    }
    
    data := &AuditData{}
    
    var success bool
    var err error
    
    parseHex := func(hexStr string) (*big.Int, error) {
        hexStr = strings.TrimPrefix(strings.ToLower(hexStr), "0x")
        n := new(big.Int)
        n, success := n.SetString(hexStr, 16)
        if !success {
            return nil, fmt.Errorf("invalid hex number: %s", hexStr)
        }
        return n, nil
    }
    
    if data.ACominX, err = parseHex(matches[1]); err != nil {
        return nil, fmt.Errorf("ACominX: %v", err)
    }
    if data.ACominY, err = parseHex(matches[2]); err != nil {
        return nil, fmt.Errorf("ACominY: %v", err)
    }
    if data.AComoutX, err = parseHex(matches[3]); err != nil {
        return nil, fmt.Errorf("AComoutX: %v", err)
    }
    if data.AComoutY, err = parseHex(matches[4]); err != nil {
        return nil, fmt.Errorf("AComoutY: %v", err)
    }
    if data.Tx1, err = parseHex(matches[5]); err != nil {
        return nil, fmt.Errorf("Tx1: %v", err)
    }
    if data.Ty1, err = parseHex(matches[6]); err != nil {
        return nil, fmt.Errorf("Ty1: %v", err)
    }
    if data.Tx2, err = parseHex(matches[7]); err != nil {
        return nil, fmt.Errorf("Tx2: %v", err)
    }
    if data.Ty2, err = parseHex(matches[8]); err != nil {
        return nil, fmt.Errorf("Ty2: %v", err)
    }
    if data.Z, err = parseHex(matches[9]); err != nil {
        return nil, fmt.Errorf("z: %v", err)
    }
    if data.Zr1, err = parseHex(matches[10]); err != nil {
        return nil, fmt.Errorf("z: %v", err)
    }
    if data.Zr2, err = parseHex(matches[11]); err != nil {
        return nil, fmt.Errorf("z: %v", err)
    }
    
    return data, nil
}

func varifyNIZK(data *AuditData) bool {
    HX := new(big.Int)
    HX, success := HX.SetString("0xC6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5", 16)
    HY := new(big.Int)
    HY, success := HY.SetString("0x1AE168FEA63DC339A3C58419466EFAEEF7F632653266D0E1236431A950CFE52A", 16)

    // NIZK proof verify
	m := data.Tx1.String() + data.Ty1.String() + data.Tx2.String() + data.Ty2.String()
	h := sha256.New()
	h.Write(new(big.Int).SetString(m, 16))
	C := h.Sum(nil)

	Gzx, Gzy := secp256k1.S256().ScalarBaseMult(data.Z)
    Hzrx1, Hzry1 := secp256k1.S256().ScalarMult(HX, HY, data.Zr1)
    Hzrx2, Hzry2 := secp256k1.S256().ScalarMult(HX, HY, data.Zr2)
	ACominX_c, ACominY_c := secp256k1.S256().ScalarMult(ACominX, ACominY, C)
	AComoutX_c, AComoutY_c := secp256k1.S256().ScalarMult(AComoutX, AComoutY, C)

	leftx1, lefty1 := secp256k1.S256().Add(Gzx, Gzy, Hzrx1, Hzry1)
	leftx2, lefty2 := secp256k1.S256().Add(Gzx, Gzy, Hzrx2, Hzry2)
	rightx1, righty1 := secp256k1.S256().Add(data.Tx1, data.Ty1, ACominX_c, ACominY_c)
	rightx2, righty2 := secp256k1.S256().Add(data.Tx2, data.Ty2, AComoutX_c, AComoutY_c)

	flag1 := (leftx1.Cmp(rightx1) == 0) && (lefty1.Cmp(righty1) == 0)
	flag2 := (leftx2.Cmp(rightx2) == 0) && (lefty2.Cmp(righty2) == 0)
    return True
	return flag1 && flag2
}