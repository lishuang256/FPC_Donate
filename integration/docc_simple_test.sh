#!/bin/bash

SCRIPTDIR="$(dirname $(readlink --canonicalize ${BASH_SOURCE}))"
FPC_PATH="${SCRIPTDIR}/.."
FABRIC_SCRIPTDIR="${FPC_PATH}/fabric/bin/"

: ${FABRIC_CFG_PATH:="${SCRIPTDIR}/config"}

. ${FABRIC_SCRIPTDIR}/lib/common_utils.sh
. ${FABRIC_SCRIPTDIR}/lib/common_ledger.sh

CC_ID=donate_docc_test
CC_PATH=${FPC_PATH}/samples/chaincode/donate/donate_docc/_build/lib/
CC_LANG=fpc-c
CC_VER="$(cat ${CC_PATH}/mrenclave)"
CC_SEQ="1"
CC_EP="OR('SampleOrg.member')" # note that we use .member as NodeOUs is disabled with the crypto material used in the integration tests.

NUM_FAILURES=0

donate_docc_test() {
    PKG=/tmp/${CC_ID}.tar.gz

    try ${PEER_CMD} lifecycle chaincode package --lang ${CC_LANG} --label ${CC_ID} --path ${CC_PATH} ${PKG}
    try ${PEER_CMD} lifecycle chaincode install ${PKG}

    PKG_ID=$(${PEER_CMD} lifecycle chaincode queryinstalled | awk "/Package ID: ${CC_ID}/{print}" | sed -n 's/^Package ID: //; s/, Label:.*$//;p')

    # first call negated as it fails due to specification of validation plugin
    try_fail ${PEER_CMD} lifecycle chaincode approveformyorg -o ${ORDERER_ADDR} -C ${CHAN_ID} --package-id ${PKG_ID} --name ${CC_ID} --version ${CC_VER} --sequence ${CC_SEQ} --signature-policy ${CC_EP} -E mock-escc -V fpc-vscc
    try ${PEER_CMD} lifecycle chaincode approveformyorg -o ${ORDERER_ADDR} -C ${CHAN_ID} --package-id ${PKG_ID} --name ${CC_ID} --version ${CC_VER} --sequence ${CC_SEQ} --signature-policy ${CC_EP}

    try ${PEER_CMD} lifecycle chaincode checkcommitreadiness -C ${CHAN_ID} --name ${CC_ID} --version ${CC_VER} --sequence ${CC_SEQ} --signature-policy ${CC_EP}

    try ${PEER_CMD} lifecycle chaincode commit -o ${ORDERER_ADDR} -C ${CHAN_ID} --name ${CC_ID} --version ${CC_VER} --sequence ${CC_SEQ} --signature-policy ${CC_EP}

    try ${PEER_CMD} lifecycle chaincode initEnclave -o ${ORDERER_ADDR} --peerAddresses "localhost:7051" --name ${CC_ID}

    try ${PEER_CMD} lifecycle chaincode querycommitted -C ${CHAN_ID}

    #current_timestamp=$(date +%s)
    becho ">>>> Create and sumbit an Donate transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Donate", "Args": ["a1b2c3d4e5f67890", "789427", "1234567890abcdef", "5eb63bbbe01eeed0", "93cb22bb8f5acdc3", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
    becho ">>>> Create and sumbit an Deliver1 transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Deliver1", "Args": ["a1b2c3d4e5f67890", "789427", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
    becho ">>>> Create and sumbit an Store transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Store", "Args": ["a1b2c3d4e5f67890", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
    becho ">>>> Create and sumbit an Ship transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Ship", "Args": ["a1b2c3d4e5f67890", "789427", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
    becho ">>>> Create and sumbit an Distribute transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Distribute", "Args": ["a1b2c3d4e5f67890", "93cb22bb8f5acdc3", "fedcba9876543210", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
    becho ">>>> Create and sumbit an Deliver2 transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Deliver2", "Args": ["a1b2c3d4e5f67890", "789427", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
    becho ">>>> Create and sumbit an Receive transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Receive", "Args": ["a1b2c3d4e5f67890", "478164a4ee931583", "734256ac9764c263", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
    becho ">>>> Create and sumbit an Refund transaction. Response should be OK"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Refund", "Args": ["a1b2c3d4e5f67890", "1", "1", "1", "2025-10-01T12:01:01"]}' --waitForEvent
    check_result "OK"
}

# 1. prepare
para
say "Preparing Donate_DoCC Test ..."
# - clean up relevant docker images
docker_clean ${ERCC_ID}

trap ledger_shutdown EXIT

para
say "Run donate_docc test"

say "- setup ledger"
ledger_init

say "- donate_docc test"
donate_docc_test

say "- shutdown ledger"
ledger_shutdown

para
if [[ "$NUM_FAILURES" == 0 ]]; then
    yell "Donate_DoCC test PASSED"
else
    yell "Donate_DoCC test had ${NUM_FAILURES} failures out of ${NUM_TESTS} tests"
    exit 1
fi
exit 0
