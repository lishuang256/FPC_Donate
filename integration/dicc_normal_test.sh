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

JSON_FILE="decrypted_data.json"

if [ ! -f "$JSON_FILE" ]; then
    echo "Error: The $JSON_FILE file cannot be found"
    exit 1
fi

COUNT=$(jq '. | length' "$JSON_FILE")

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
    becho ">>>> Create and sumbit n Distribute transactions and n Refund transactions. Response should be OK or TRUE"
    for i in $(seq 0 $(($COUNT-1))); do
        AMOUNT=$(jq -r ".[$i].amount" "$JSON_FILE")
        MATERIAL_LIST=$(jq -r ".[$i].materialList" "$JSON_FILE")
        COMMENT_X=$(jq -r ".[$i].commentX" "$JSON_FILE")
        COMMENT_Y=$(jq -r ".[$i].commentY" "$JSON_FILE")
        RANDOM_NUM=$(jq -r ".[$i].randomNumber" "$JSON_FILE")
        RANDOM_NUM_NEW=$(jq -r ".[$i].randomNumNew" "$JSON_FILE")
        TIMESTAMP=$(jq -r ".[$i].timestamp" "$JSON_FILE")

        ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Distribute", "Args": ["'"${AMOUNT}"'", "'"${MATERIAL_LIST}"'", "'"${COMMENT_X}"'", "'"${COMMENT_Y}"'", "'"${RANDOM_NUM}"'", "'"${RANDOM_NUM_NEW}"'", "'"${TIMESTAMP}"'"]}' --waitForEvent
        check_result "TRUE"

        try_out_r ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Refund", "Args": ["'"${MATERIAL_LIST}"'", "'"${AMOUNT}${MATERIAL_LIST}${COMMENT_X}${COMMENT_Y}${RANDOM_NUM}${RANDOM_NUM_NEW}${TIMESTAMP}"'"]}' --waitForEvent
        check_result "TRUE"

    done

    becho ">>>> Create and sumbit an Audit transaction. Response should be OK"
    ${PEER_CMD} chaincode invoke -o ${ORDERER_ADDR} -C ${CHAN_ID} -n ${CC_ID} -c '{"Function":"Audit", "Args": []}' --waitForEvent

    go run ${FPC_PATH}/samples/chaincode/donate/donate_dicc/audit.go
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
