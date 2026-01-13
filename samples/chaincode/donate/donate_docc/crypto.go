package main

import (
	"crypto/sha256"
	"math/rand"
	"fmt"
	"math/big"
	"time"

	"github.com/ethereum/go-ethereum/crypto/secp256k1"
)

var (
	zero  = big.NewInt(0)
	one   = big.NewInt(1)
	two   = big.NewInt(2)
	three = big.NewInt(3)
	four  = big.NewInt(4)
	w     = 16 // at most 8 byte as a group
	l     = 32 // byteLen of random and priKey
	k     = 16 // offset
	bigk  = big.NewInt(16)
)

// PKCS7 standard
func Padding(msg []byte) []byte {
	var Len = 0
	if len(msg)%w == 0 {
		Len = w
	} else {
		Len = w - len(msg)%w
	}
	res := make([]byte, Len+len(msg))
	copy(res, msg)
	for i := 0; i < Len; i++ {
		res[i+len(msg)] = byte(Len)
	}
	return res
}

func ExpMod(a, e, p *big.Int) *big.Int { // x^y mod p
	res := big.NewInt(1)
	tmp := big.NewInt(0)
	x := new(big.Int).Set(a)
	y := new(big.Int).Set(e)
	for y.Cmp(zero) != 0 {
		if tmp.And(y, one).Cmp(one) == 0 { // ((y&1)==1)==0
			res.Mul(res, x).Mod(res, p)
			y.Sub(y, one)
		} else {
			x.Mul(x, x).Mod(x, p)
			y.Rsh(y, 1) // y>>1
		}
	}
	return res
}

// return [(x1,y1), (x2,y2)...]
func Encode(mm []byte) []*big.Int {
	msg := Padding(mm)
	res := make([]*big.Int, 2*len(msg)/w)
	big7 := big.NewInt(7)
	P := secp256k1.S256().P
	Psub1 := new(big.Int).Sub(P, one)              // P-1
	half_Psub1 := new(big.Int).Div(Psub1, two)     // (P-1)/2
	Padd1 := new(big.Int).Add(P, one)              // P+1
	quarter_Padd1 := new(big.Int).Div(Padd1, four) // (P+1)/4
	for i := 0; i < len(msg); i += w {
		m := new(big.Int).SetBytes(msg[i : i+w])
		x := new(big.Int).Mul(m, bigk) //x.Mod(x, P)// no use in theory
		for j := 0; j < k; j++ {
			xxx := ExpMod(x, three, P)        // x^3
			yy := new(big.Int).Add(xxx, big7) // y^2 = x^3 + 7 (mod p)
			Legendre := ExpMod(yy, half_Psub1, P)
			if Legendre.Cmp(one) == 0 {
				y := ExpMod(yy, quarter_Padd1, P) // P == 3 mod(4)
				res[2*i/w] = x
				res[2*i/w+1] = y
				break
			}
			x.Add(x, one) // Xj = k*X+j
		}
	}
	return res
}

func GetRand(length uint) []byte {
	res := make([]byte, length)
	source := rand.NewSource(time.Now().UnixNano())
	r := rand.New(source)
	r.Read(res)
	return res
}

func GenKey() (priByte []byte, pubByte []byte) {
	// fmt.Println("\t\tInitialize the key pair")
	// pubKey = h ^ priKey
	priKey := GetRand(2 * uint(l)) // priKey = [r1, r2]
	r1 := priKey[:l]
	r2 := priKey[l:]
	Hx, Hy := secp256k1.S256().ScalarBaseMult(r1)
	pubKeyX, pubKeyY := secp256k1.S256().ScalarMult(Hx, Hy, r2)
	return priKey, secp256k1.S256().Marshal(pubKeyX, pubKeyY) // []byte,big-endian
}

func Encrypt(pubKey []byte, msgg []byte) []byte {
	// fmt.Println("\t\tEncrypt")
	points := Encode(msgg)
	cipherText := make([]byte, 130*len(points)/2)
	pubKeyX, pubKeyY := secp256k1.S256().Unmarshal(pubKey)

	for i := 0; i < len(points); i += 2 {
		r := make([]byte, l)
		rand.Read(r)
		C1x, C1y := secp256k1.S256().ScalarBaseMult(r) // C1 = rG
		Mx := points[i]
		My := points[i+1]
		rQx, rQy := secp256k1.S256().ScalarMult(pubKeyX, pubKeyY, r)
		C2x, C2y := secp256k1.S256().Add(Mx, My, rQx, rQy) // C2 = M + rQ
		C1 := secp256k1.S256().Marshal(C1x, C1y)
		C2 := secp256k1.S256().Marshal(C2x, C2y)
		j := 130 * (i / 2)
		copy(cipherText[j:j+65], C1)
		copy(cipherText[j+65:j+130], C2)
	}
	return cipherText
}

func Decrypt(priKey []byte, cipher []byte) []byte {
	// fmt.Println("\t\tDecrypt")
	priKey1 := priKey[:l]
	priKey2 := priKey[l:]
	plainText := make([]byte, w*len(cipher)/130)
	P := secp256k1.S256().P
	ll := 0
	for i := 0; i < len(cipher); i += 130 {
		C1x, C1y := secp256k1.S256().Unmarshal(cipher[i : i+65])
		C2x, C2y := secp256k1.S256().Unmarshal(cipher[i+65 : i+130])
		tmpx, tmpy := secp256k1.S256().ScalarMult(C1x, C1y, priKey1)
		kC1x, kC1y := secp256k1.S256().ScalarMult(tmpx, tmpy, priKey2)
		Mx, _ := secp256k1.S256().Add(C2x, C2y, kC1x, kC1y.Sub(P, kC1y)) // X = C2 - dC1
		Mbig := new(big.Int).Quo(Mx, bigk)                               // X = kM + i mod p
		tmp := Mbig.Bytes()                                              // w byte by default
		if len(tmp) < w {
			ll = len(tmp)
		} else {
			ll = w
		}
		t := w * (i / 130)
		copy(plainText[t:w+t], tmp[0:ll])
	}

	ll = len(plainText)
	padLen := plainText[ll-1]
	res := plainText[:(ll - int(padLen))]
	return res
}