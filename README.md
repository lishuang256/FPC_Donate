[FPC Framework Source Code](https://github.com/hyperledger/fabric-private-chaincode)

## Setup Development Environment

[Using preconfigured Docker container development environment](docs/setup-option1.md)

To build all required FPC components and run the integration tests run the following:
```bash
make docker
make
 ```

## Build

In [DoCC](samples/chaincode/donate/donate_docc) or [DiCC](samples/chaincode/donate/donate_dicc) folder, to build the chaincode, execute:
```bash
make
```

Following is a part of expected output.  Please note `[100%] Built target enclave` message in the output.  This suggests that build was successful.

## Test

In [integration](integration) folder, execute the simple test script for DoCC:
```bash
bash ./docc_simple_test.sh
```

Response from each transaction is:
```
OK
```

Execute the simple test script for DoCC:
```bash
bash ./dicc_simple_test.sh
```

Response from the Distribute transaction is:
```
ComqX: xxx ComqY: xxx
```

Response from the Refund transaction is:
```
TRUE
```

Response from the Audit transaction is:
```
ACominX: xxx ACominY: xxx AComoutX: xxx AComoutY: xxx | Tx1: xxx Ty1: xxx Tx2: xxx Ty2: xxx | z: xxx zr1: xxx zr2: xxx
```
