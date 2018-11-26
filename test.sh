

[ -f generate ] && rm generate
make
if [ -f generate ]; then
	cat myconfig.xml | ./generate -run 1>/dev/null
	curl https://raw.githubusercontent.com/hyperledger/fabric-samples/release-1.3/first-network/configtx.yaml | ./generate -hype  1>/dev/null
fi

