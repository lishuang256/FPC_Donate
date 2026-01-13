#!/bin/bash

SCRIPTDIR="$(dirname $(readlink --canonicalize ${BASH_SOURCE}))"
FPC_PATH="${SCRIPTDIR}/.."
FABRIC_SCRIPTDIR="${FPC_PATH}/fabric/bin/"

: ${FABRIC_CFG_PATH:="${SCRIPTDIR}/config"}

. ${FABRIC_SCRIPTDIR}/lib/common_utils.sh
. ${FABRIC_SCRIPTDIR}/lib/common_ledger.sh

CC_ID=donate_dicc_test
CC_PATH=${FPC_PATH}/samples/chaincode/donate/donate_dicc/_build/lib/
CC_LANG=fpc-c
CC_VER="$(cat ${CC_PATH}/mrenclave)"
CC_SEQ="1"
CC_EP="OR('SampleOrg.member')" # note that we use .member as NodeOUs is disabled with the crypto material used in the integration tests.

NUM_FAILURES=0

donate_dosc_test() {
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

    # current_timestamp=$(date +%s)
    becho ">>>> Create and sumbit n Distribute transaction. Response should be OK"
    ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Distribute", "Args": ["101", "101facemask", "5eb63bbbe01eeed0", "93cb22bb8f5acdc3", "26374857", "36485947", "2025-10-01T12:01:01"]}' --waitForEvent
    ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Distribute", "Args": ["102", "102facemask", "93cb22bb8f5acdc3", "fedcba9876543210", "27465840", "17347304", "2025-10-01T12:02:01"]}' --waitForEvent
    ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Distribute", "Args": ["103", "103facemask", "478164a4ee931583", "734256ac9764c263", "73648774", "28378468", "2025-10-01T12:03:01"]}' --waitForEvent
    ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Distribute", "Args": ["104", "104facemask", "37459dec67a54388", "e433468900146930", "23824684", "27327723", "2025-10-01T12:04:01"]}' --waitForEvent
    ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Distribute", "Args": ["105", "105facemask", "9efc457890321455", "748569321ac54789", "15468898", "12635473", "2025-10-01T12:05:01"]}' --waitForEvent
    becho ">>>> Create and sumbit n Refund transaction. Response should be TRUE"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Refund", "Args": ["101facemask", "101101facemask5eb63bbbe01eeed093cb22bb8f5acdc326374857364859472025-10-01T12:01:01"]}' --waitForEvent
    check_result "TRUE"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Refund", "Args": ["102facemask", "102102facemask93cb22bb8f5acdc3fedcba987654321027465840173473042025-10-01T12:02:01"]}' --waitForEvent
    check_result "TRUE"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Refund", "Args": ["103facemask", "103103facemask478164a4ee931583734256ac9764c26373648774283784682025-10-01T12:03:01"]}' --waitForEvent
    check_result "TRUE"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Refund", "Args": ["104facemask", "104104facemask37459dec67a54388e43346890014693023824684273277232025-10-01T12:04:01"]}' --waitForEvent
    check_result "TRUE"
    try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Refund", "Args": ["105facemask", "105105facemask9efc457890321455748569321ac5478915468898126354732025-10-01T12:05:01"]}' --waitForEvent
    check_result "TRUE"
    becho ">>>> Create and sumbit an Audit transaction. Response should be OK"
    ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Audit", "Args": []}' --waitForEvent
    # specific_timestamp=$(date +%s)
    # time_difference=$((specific_timestamp - current_timestamp))
    # echo "time_cost(s): $time_difference"
}

# 1. prepare
para
say "Preparing Donate_DiCC Test ..."
# - clean up relevant docker images
docker_clean ${ERCC_ID}

trap ledger_shutdown EXIT

para
say "Run donate_dicc test"

say "- setup ledger"
ledger_init

say "- donate_dicc test"
donate_dosc_test

say "- shutdown ledger"
ledger_shutdown

para
if [[ "$NUM_FAILURES" == 0 ]]; then
    yell "Donate_DiCC test PASSED"
else
    yell "Donate_DiCC test had ${NUM_FAILURES} failures out of ${NUM_TESTS} tests"
    exit 1
fi
exit 0
